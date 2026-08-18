#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include <MinHook.h>

#include "mod_loader_api.h"
#include "reflex_api.h"

namespace {

constexpr uint32_t DEFAULT_FPS = 60;
constexpr DWORD RETRY_INTERVAL_MS = 1000;

struct Context {
    HMODULE self{};
    mod_log_fn log{};
    uint32_t frame_limit_us{};
    bool log_enabled{};
    streamline::ReflexSetOptions original_set_options{};
};

Context g_context{};

void log(const char* message) {
    if (g_context.log != nullptr) {
        g_context.log(message);
    }
}

bool own_config_path(wchar_t path[MAX_PATH]) {
    const DWORD length = GetModuleFileNameW(g_context.self, path, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) {
        return false;
    }

    wchar_t* slash = nullptr;
    for (wchar_t* cursor = path; *cursor != L'\0'; ++cursor) {
        if (*cursor == L'\\' || *cursor == L'/') {
            slash = cursor;
        }
    }
    if (slash == nullptr || (slash - path) + 12 >= MAX_PATH) {
        return false;
    }
    slash[1] = L'\0';
    lstrcatW(path, L"config.ini");
    return true;
}

uint32_t read_config_fps(const wchar_t* path) {
    wchar_t fallback[32]{};
    wchar_t value[32]{};
    wsprintfW(fallback, L"%u", static_cast<unsigned int>(DEFAULT_FPS));
    GetPrivateProfileStringW(L"reflex_fps_limit", L"fps", fallback, value,
                             static_cast<DWORD>(sizeof(value) /
                                                sizeof(value[0])),
                             path);

    wchar_t* end = nullptr;
    errno = 0;
    const unsigned long long parsed = wcstoull(value, &end, 10);
    if (errno == ERANGE || end == value || *end != L'\0' || value[0] == L'-' ||
        parsed > UINT32_MAX) {
        log("reflex_fps_limit: invalid fps in config.ini; using 60 FPS");
        return DEFAULT_FPS;
    }
    return static_cast<uint32_t>(parsed);
}

bool read_config_log(const wchar_t* path) {
    return GetPrivateProfileIntW(L"reflex_fps_limit", L"log", 0, path) != 0;
}

uint32_t fps_to_frame_limit_us(uint32_t fps) {
    if (fps == 0) {
        return 0;
    }
    const uint64_t rounded = (1000000ULL + fps / 2ULL) / fps;
    return rounded == 0 ? 1 : static_cast<uint32_t>(rounded);
}

template <typename T>
void load_export(HMODULE module, const char* name, T& destination) {
    const FARPROC address = GetProcAddress(module, name);
    static_assert(sizeof(destination) == sizeof(address));
    memcpy(&destination, &address, sizeof(destination));
}

streamline::Result hooked_set_options(
    const streamline::ReflexOptions& options) {
    streamline::ReflexOptions overridden = options;
    overridden.frame_limit_us = g_context.frame_limit_us;
    const streamline::Result result =
        g_context.original_set_options(overridden);

    if (g_context.log_enabled) {
        char message[256]{};
        snprintf(message, sizeof(message),
                 "reflex_fps_limit: slReflexSetOptions frame limit %u -> %u us "
                 "returned %d",
                 static_cast<unsigned int>(options.frame_limit_us),
                 static_cast<unsigned int>(overridden.frame_limit_us),
                 static_cast<int>(result));
        log(message);
    }
    return result;
}

void log_hook_error(const char* operation, MH_STATUS status) {
    char message[256]{};
    snprintf(message, sizeof(message),
             "reflex_fps_limit: %s failed (MinHook status %d)", operation,
             static_cast<int>(status));
    log(message);
}

void* find_hook_target() {
    const HMODULE interposer = GetModuleHandleW(L"sl.interposer.dll");
    if (interposer == nullptr) {
        log("reflex_fps_limit: sl.interposer.dll is not loaded");
        return nullptr;
    }

    streamline::GetFeatureFunction get_feature_function{};
    load_export(interposer, "slGetFeatureFunction", get_feature_function);
    if (get_feature_function == nullptr) {
        log("reflex_fps_limit: slGetFeatureFunction is unavailable");
        return nullptr;
    }

    void* hook_target = nullptr;
    const streamline::Result result = get_feature_function(
        streamline::FEATURE_REFLEX, "slReflexSetOptions", hook_target);
    if (result != streamline::Result::ok || hook_target == nullptr) {
        char message[256]{};
        snprintf(message, sizeof(message),
                 "reflex_fps_limit: Reflex is not initialized (%d)",
                 static_cast<int>(result));
        log(message);
        return nullptr;
    }
    return hook_target;
}

DWORD WINAPI worker(void*) {
    wchar_t config_path[MAX_PATH]{};
    uint32_t fps = DEFAULT_FPS;
    if (own_config_path(config_path)) {
        fps = read_config_fps(config_path);
        g_context.log_enabled = read_config_log(config_path);
    } else {
        log("reflex_fps_limit: failed to locate config.ini; using 60 FPS");
    }
    g_context.frame_limit_us = fps_to_frame_limit_us(fps);

    char message[256]{};
    snprintf(message, sizeof(message),
             "reflex_fps_limit: configured %u FPS (%u us)",
             static_cast<unsigned int>(fps),
             static_cast<unsigned int>(g_context.frame_limit_us));
    log(message);

    MH_STATUS status = MH_Initialize();
    if (status != MH_OK && status != MH_ERROR_ALREADY_INITIALIZED) {
        log_hook_error("MH_Initialize", status);
        return 0;
    }

    void* hook_target = nullptr;
    while ((hook_target = find_hook_target()) == nullptr) {
        Sleep(RETRY_INTERVAL_MS);
    }

    status = MH_CreateHook(
        hook_target, reinterpret_cast<void*>(&hooked_set_options),
        reinterpret_cast<void**>(&g_context.original_set_options));
    if (status != MH_OK) {
        log_hook_error("MH_CreateHook", status);
        return 0;
    }

    status = MH_EnableHook(hook_target);
    if (status != MH_OK) {
        log_hook_error("MH_EnableHook", status);
        MH_RemoveHook(hook_target);
        return 0;
    }
    log("reflex_fps_limit: hooked slReflexSetOptions");
    return 0;
}

} // namespace

extern "C" __declspec(dllexport) void MOD_LOADER_CALL
on_mod_load(mod_log_fn logger) {
    g_context.log = logger;
    const HANDLE thread = CreateThread(nullptr, 0, worker, nullptr, 0, nullptr);
    if (thread != nullptr) {
        CloseHandle(thread);
    } else {
        log("reflex_fps_limit: failed to create worker thread");
    }
}

extern "C" BOOL WINAPI DllMain(HMODULE module, DWORD reason, void*) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_context.self = module;
        DisableThreadLibraryCalls(module);
    }
    return TRUE;
}
