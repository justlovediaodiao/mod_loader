#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <limits.h>
#include <errno.h>
#include <wchar.h>
#include "mod_loader_api.h"

struct Il2CppDomain;
struct Il2CppAssembly;
struct Il2CppImage;
struct Il2CppClass;
struct MethodInfo;
struct Il2CppException;
struct MonoDomain;
struct MonoThread;
struct MonoAssembly;
struct MonoImage;
struct MonoClass;
struct MonoMethod;
struct MonoObject;

static HMODULE g_self;
static mod_log_fn g_log;

static void log(const wchar_t* message) {
    if (g_log) g_log(message);
}

static bool own_config_path(wchar_t path[MAX_PATH]) {
    DWORD length = GetModuleFileNameW(g_self, path, MAX_PATH);
    if (!length || length >= MAX_PATH) return false;

    wchar_t* slash = nullptr;
    for (wchar_t* p = path; *p; ++p)
        if (*p == L'\\' || *p == L'/') slash = p;
    if (!slash || (slash - path) + 12 >= MAX_PATH) return false;
    slash[1] = L'\0';
    lstrcatW(path, L"config.ini");
    return true;
}

static int read_config_int(const wchar_t* path, const wchar_t* key,
                           int fallback) {
    wchar_t fallback_text[32];
    wchar_t value[32];
    wsprintfW(fallback_text, L"%d", fallback);
    GetPrivateProfileStringW(L"unity_fps_limit", key, fallback_text, value,
                             static_cast<DWORD>(sizeof(value) / sizeof(value[0])),
                             path);
    wchar_t* end = nullptr;
    errno = 0;
    long parsed = wcstol(value, &end, 10);
    if (errno == ERANGE || end == value || *end != L'\0' ||
        parsed < INT_MIN || parsed > INT_MAX)
        return fallback;
    return static_cast<int>(parsed);
}

static bool invoke_il2cpp_setters(HMODULE module, int fps, int vsync) {
    using DomainGet = Il2CppDomain* (*)();
    using ThreadAttach = void* (*)(Il2CppDomain*);
    using AssemblyOpen = const Il2CppAssembly* (*)(Il2CppDomain*, const char*);
    using AssemblyImage = const Il2CppImage* (*)(const Il2CppAssembly*);
    using ClassFromName = Il2CppClass* (*)(const Il2CppImage*, const char*, const char*);
    using MethodFromName = const MethodInfo* (*)(Il2CppClass*, const char*, int);
    using RuntimeInvoke = void* (*)(const MethodInfo*, void*, void**, Il2CppException**);

#define IL2CPP_API(type, name) auto name = reinterpret_cast<type>(GetProcAddress(module, "il2cpp_" #name))
    IL2CPP_API(DomainGet, domain_get);
    IL2CPP_API(ThreadAttach, thread_attach);
    IL2CPP_API(AssemblyOpen, domain_assembly_open);
    IL2CPP_API(AssemblyImage, assembly_get_image);
    IL2CPP_API(ClassFromName, class_from_name);
    IL2CPP_API(MethodFromName, class_get_method_from_name);
    IL2CPP_API(RuntimeInvoke, runtime_invoke);
#undef IL2CPP_API
    if (!domain_get || !thread_attach || !domain_assembly_open ||
        !assembly_get_image || !class_from_name ||
        !class_get_method_from_name || !runtime_invoke) return false;

    Il2CppDomain* domain = domain_get();
    if (!domain) return false;
    thread_attach(domain);
    const Il2CppAssembly* assembly = domain_assembly_open(domain, "UnityEngine.CoreModule.dll");
    if (!assembly) assembly = domain_assembly_open(domain, "UnityEngine.CoreModule");
    if (!assembly) return false;
    const Il2CppImage* image = assembly_get_image(assembly);
    if (!image) return false;

    Il2CppClass* quality = class_from_name(image, "UnityEngine", "QualitySettings");
    Il2CppClass* application = class_from_name(image, "UnityEngine", "Application");
    const MethodInfo* set_vsync = quality
        ? class_get_method_from_name(quality, "set_vSyncCount", 1) : nullptr;
    const MethodInfo* set_fps = application
        ? class_get_method_from_name(application, "set_targetFrameRate", 1) : nullptr;
    if (!set_vsync || !set_fps) return false;

    Il2CppException* exception = nullptr;
    void* vsync_args[] = { &vsync };
    runtime_invoke(set_vsync, nullptr, vsync_args, &exception);
    if (exception) return false;
    void* fps_args[] = { &fps };
    runtime_invoke(set_fps, nullptr, fps_args, &exception);
    return exception == nullptr;
}

struct MonoApi {
    MonoDomain* (*get_root_domain)();
    MonoThread* (*thread_attach)(MonoDomain*);
    void (*assembly_foreach)(void (*)(MonoAssembly*, void*), void*);
    MonoImage* (*assembly_get_image)(MonoAssembly*);
    const char* (*image_get_name)(MonoImage*);
    MonoClass* (*class_from_name)(MonoImage*, const char*, const char*);
    MonoMethod* (*class_get_method_from_name)(MonoClass*, const char*, int);
    MonoObject* (*runtime_invoke)(MonoMethod*, void*, void**, MonoObject**);
};

struct ImageSearch { MonoApi* api; MonoImage* image; };

static void find_unity_image(MonoAssembly* assembly, void* user) {
    auto* search = static_cast<ImageSearch*>(user);
    if (search->image) return;
    MonoImage* image = search->api->assembly_get_image(assembly);
    const char* name = image ? search->api->image_get_name(image) : nullptr;
    if (name && (lstrcmpiA(name, "UnityEngine.CoreModule.dll") == 0 ||
                 lstrcmpiA(name, "UnityEngine.dll") == 0)) search->image = image;
}

static bool invoke_mono_setters(HMODULE module, int fps, int vsync) {
    MonoApi api{};
#define MONO_API(field, export_name) api.field = reinterpret_cast<decltype(api.field)>(GetProcAddress(module, export_name))
    MONO_API(get_root_domain, "mono_get_root_domain");
    MONO_API(thread_attach, "mono_thread_attach");
    MONO_API(assembly_foreach, "mono_assembly_foreach");
    MONO_API(assembly_get_image, "mono_assembly_get_image");
    MONO_API(image_get_name, "mono_image_get_name");
    MONO_API(class_from_name, "mono_class_from_name");
    MONO_API(class_get_method_from_name, "mono_class_get_method_from_name");
    MONO_API(runtime_invoke, "mono_runtime_invoke");
#undef MONO_API
    if (!api.get_root_domain || !api.thread_attach || !api.assembly_foreach ||
        !api.assembly_get_image || !api.image_get_name || !api.class_from_name ||
        !api.class_get_method_from_name || !api.runtime_invoke) return false;

    MonoDomain* domain = api.get_root_domain();
    if (!domain) return false;
    api.thread_attach(domain);
    ImageSearch search{ &api, nullptr };
    api.assembly_foreach(find_unity_image, &search);
    if (!search.image) return false;

    MonoClass* quality = api.class_from_name(search.image, "UnityEngine", "QualitySettings");
    MonoClass* application = api.class_from_name(search.image, "UnityEngine", "Application");
    MonoMethod* set_vsync = quality
        ? api.class_get_method_from_name(quality, "set_vSyncCount", 1) : nullptr;
    MonoMethod* set_fps = application
        ? api.class_get_method_from_name(application, "set_targetFrameRate", 1) : nullptr;
    if (!set_vsync || !set_fps) return false;

    MonoObject* exception = nullptr;
    void* vsync_args[] = { &vsync };
    api.runtime_invoke(set_vsync, nullptr, vsync_args, &exception);
    if (exception) return false;
    void* fps_args[] = { &fps };
    api.runtime_invoke(set_fps, nullptr, fps_args, &exception);
    return exception == nullptr;
}

static DWORD WINAPI fps_worker(void*) {
    wchar_t ini[MAX_PATH];
    if (!own_config_path(ini)) {
        log(L"unity_fps_limit: failed to locate mods\\config.ini");
        return 1;
    }
    // Signed parsing is intentional: Unity uses targetFrameRate = -1 to select
    // the platform default frame rate behavior.
    int fps = read_config_int(ini, L"fps", -1);
    int vsync = read_config_int(ini, L"vsync", 0);
    log(L"unity_fps_limit: waiting for Unity scripting backend");

    for (;;) {
        HMODULE il2cpp = GetModuleHandleW(L"GameAssembly.dll");
        if (il2cpp && invoke_il2cpp_setters(il2cpp, fps, vsync)) {
            log(L"unity_fps_limit: applied through IL2CPP");
            return 0;
        }
        HMODULE mono = GetModuleHandleW(L"mono-2.0-bdwgc.dll");
        if (!mono) mono = GetModuleHandleW(L"mono.dll");
        if (mono && invoke_mono_setters(mono, fps, vsync)) {
            log(L"unity_fps_limit: applied through Mono");
            return 0;
        }
        Sleep(1000);
    }
}

extern "C" __declspec(dllexport) void MOD_LOADER_CALL on_mod_load(mod_log_fn logger) {
    g_log = logger;
    HANDLE thread = CreateThread(nullptr, 0, fps_worker, nullptr, 0, nullptr);
    if (thread) CloseHandle(thread);
    else log(L"unity_fps_limit: failed to create worker thread");
}

extern "C" BOOL WINAPI DllMain(HINSTANCE self, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_self = self;
        DisableThreadLibraryCalls(self);
    }
    return TRUE;
}
