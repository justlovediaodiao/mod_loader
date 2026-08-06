#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stddef.h>
#include <wchar.h>

#include <MinHook.h>

#include "mod_loader_api.h"

using LoadLibraryExWFn = HMODULE(WINAPI*)(LPCWSTR, HANDLE, DWORD);

static mod_log_fn g_log;
static LoadLibraryExWFn g_original_load_library_ex_w;
static wchar_t g_local_steamclient[MAX_PATH];
static volatile LONG g_redirect_logged;

static void log_message(const wchar_t* message) {
    if (g_log) g_log(message);
}

static void log_hook_error(const wchar_t* operation, MH_STATUS status) {
    wchar_t message[256];
    wsprintfW(message, L"steamclient_redirect: %s failed (MinHook status %d)",
              operation, static_cast<int>(status));
    log_message(message);
}

static bool build_local_steamclient_path() {
    DWORD length = GetModuleFileNameW(nullptr, g_local_steamclient, MAX_PATH);
    if (!length || length >= MAX_PATH) return false;

    wchar_t* slash = nullptr;
    for (wchar_t* p = g_local_steamclient; *p; ++p)
        if (*p == L'\\' || *p == L'/') slash = p;
    if (!slash) return false;

    constexpr wchar_t filename[] = L"steamclient64.dll";
    const size_t directory_length =
        static_cast<size_t>(slash - g_local_steamclient + 1);
    if (directory_length + _countof(filename) > MAX_PATH) return false;

    slash[1] = L'\0';
    lstrcatW(g_local_steamclient, filename);
    DWORD attributes = GetFileAttributesW(g_local_steamclient);
    return attributes != INVALID_FILE_ATTRIBUTES &&
           !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

static const wchar_t* filename_part(const wchar_t* path) {
    if (!path) return nullptr;
    const wchar_t* filename = path;
    for (const wchar_t* p = path; *p; ++p)
        if (*p == L'\\' || *p == L'/') filename = p + 1;
    return filename;
}

static HMODULE WINAPI redirected_load_library_ex_w(
    LPCWSTR file_name, HANDLE file, DWORD flags) {
    const wchar_t* filename = filename_part(file_name);
    if (filename && lstrcmpiW(filename, L"steamclient64.dll") == 0 &&
        lstrcmpiW(file_name, g_local_steamclient) != 0) {
        if (InterlockedCompareExchange(&g_redirect_logged, 1, 0) == 0)
            log_message(L"steamclient_redirect: redirecting steamclient64.dll to the local copy");
        return g_original_load_library_ex_w(g_local_steamclient, file, flags);
    }
    return g_original_load_library_ex_w(file_name, file, flags);
}

static bool install_redirect() {
    MH_STATUS status = MH_Initialize();
    if (status != MH_OK && status != MH_ERROR_ALREADY_INITIALIZED) {
        log_hook_error(L"MH_Initialize", status);
        return false;
    }

    void* target = nullptr;
    status = MH_CreateHookApiEx(
        L"kernel32.dll",
        "LoadLibraryExW",
        reinterpret_cast<void*>(&redirected_load_library_ex_w),
        reinterpret_cast<void**>(&g_original_load_library_ex_w),
        &target
    );
    if (status != MH_OK) {
        log_hook_error(L"MH_CreateHookApiEx", status);
        return false;
    }

    status = MH_EnableHook(target);
    if (status != MH_OK) {
        log_hook_error(L"MH_EnableHook", status);
        MH_RemoveHook(target);
        return false;
    }

    log_message(L"steamclient_redirect: installed LoadLibraryExW detour");
    return true;
}

extern "C" __declspec(dllexport)
void MOD_LOADER_CALL on_mod_load(mod_log_fn logger) {
    g_log = logger;
    if (!build_local_steamclient_path()) {
        log_message(L"steamclient_redirect: local steamclient64.dll was not found beside the game executable");
        return;
    }

    if (GetModuleHandleW(L"steamclient64.dll"))
        log_message(L"steamclient_redirect: warning: steamclient64.dll was already loaded before hook installation");

    install_redirect();
}
