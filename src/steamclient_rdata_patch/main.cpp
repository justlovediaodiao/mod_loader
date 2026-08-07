#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <intrin.h>
#include <stddef.h>
#include <string.h>
#include <wchar.h>

#include <MinHook.h>

#include "mod_loader_api.h"

using LoadLibraryExWFn = HMODULE(WINAPI*)(LPCWSTR, HANDLE, DWORD);
using LdrLockLoaderLockFn = LONG(NTAPI*)(ULONG, ULONG*, ULONG_PTR*);
using LdrUnlockLoaderLockFn = LONG(NTAPI*)(ULONG, ULONG_PTR);

// Only the stable, leading fields used below are described here. These layouts
// are specific to the x64 target enforced by CMake.
struct LoaderUnicodeString {
    USHORT Length;
    USHORT MaximumLength;
    wchar_t* Buffer;
};

struct LoaderDataTableEntry {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    void* DllBase;
    void* EntryPoint;
    ULONG SizeOfImage;
    ULONG Padding;
    LoaderUnicodeString FullDllName;
    LoaderUnicodeString BaseDllName;
};

struct PebLoaderData {
    ULONG Length;
    BOOLEAN Initialized;
    BYTE Padding1[3];
    void* SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
};

struct PartialPeb {
    BYTE Reserved[0x18];
    PebLoaderData* Ldr;
};

static_assert(offsetof(LoaderDataTableEntry, DllBase) == 0x30);
static_assert(offsetof(LoaderDataTableEntry, FullDllName) == 0x48);
static_assert(offsetof(LoaderDataTableEntry, BaseDllName) == 0x58);
static_assert(offsetof(PartialPeb, Ldr) == 0x18);

static mod_log_fn g_log;
static LoadLibraryExWFn g_original_load_library_ex_w;
static LdrLockLoaderLockFn g_ldr_lock_loader_lock;
static LdrUnlockLoaderLockFn g_ldr_unlock_loader_lock;
static wchar_t g_original_steamclient[MAX_PATH];
static wchar_t g_patched_steamclient[MAX_PATH];
static HMODULE g_loaded_steamclient;
static CRITICAL_SECTION g_redirect_lock;

static void log_message(const wchar_t* message) {
    if (g_log) g_log(message);
}

static void log_hook_error(const wchar_t* operation, MH_STATUS status) {
    wchar_t message[256];
    wsprintfW(message,
              L"steamclient_rdata_patch: %s failed (MinHook status %d)",
              operation, static_cast<int>(status));
    log_message(message);
}

static const wchar_t* filename_part(const wchar_t* path) {
    if (!path) return nullptr;
    const wchar_t* filename = path;
    for (const wchar_t* p = path; *p; ++p)
        if (*p == L'\\' || *p == L'/') filename = p + 1;
    return filename;
}

static bool get_patched_steamclient_path() {
    DWORD size = sizeof(g_original_steamclient);
    LSTATUS status = RegGetValueW(
        HKEY_CURRENT_USER,
        L"Software\\Valve\\Steam\\ActiveProcess",
        L"SteamClientDll64",
        RRF_RT_REG_SZ,
        nullptr,
        g_original_steamclient,
        &size
    );
    if (status != ERROR_SUCCESS || !g_original_steamclient[0])
        return false;

    lstrcpynW(g_patched_steamclient, g_original_steamclient, MAX_PATH);

    wchar_t* slash = nullptr;
    for (wchar_t* p = g_patched_steamclient; *p; ++p)
        if (*p == L'\\' || *p == L'/') slash = p;
    if (!slash) return false;

    constexpr wchar_t patched_name[] = L"steamclient64_patched.dll";
    const size_t directory_length =
        static_cast<size_t>(slash - g_patched_steamclient + 1);
    constexpr size_t patched_name_length =
        sizeof(patched_name) / sizeof(patched_name[0]);
    if (directory_length + patched_name_length > MAX_PATH)
        return false;

    slash[1] = L'\0';
    lstrcatW(g_patched_steamclient, patched_name);

    DWORD attributes = GetFileAttributesW(g_patched_steamclient);
    return attributes != INVALID_FILE_ATTRIBUTES &&
           !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

static void overwrite_loader_string(
    LoaderUnicodeString* destination, const wchar_t* source) {
    const size_t source_chars = wcslen(source);
    const size_t source_bytes = source_chars * sizeof(wchar_t);
    memcpy(destination->Buffer, source, source_bytes + sizeof(wchar_t));
    destination->Length = static_cast<USHORT>(source_bytes);
}

static bool restore_original_loader_names(HMODULE module) {
    ULONG disposition = 0;
    ULONG_PTR cookie = 0;
    if (g_ldr_lock_loader_lock(0, &disposition, &cookie) < 0)
        return false;

    bool restored = false;
    PartialPeb* peb = reinterpret_cast<PartialPeb*>(__readgsqword(0x60));
    LIST_ENTRY* head = &peb->Ldr->InLoadOrderModuleList;
    for (LIST_ENTRY* link = head->Flink; link != head; link = link->Flink) {
        LoaderDataTableEntry* entry = CONTAINING_RECORD(
            link, LoaderDataTableEntry, InLoadOrderLinks);
        if (entry->DllBase != module) continue;

        // Both replacement strings are shorter than the patched names whose
        // loader buffers they reuse.
        overwrite_loader_string(&entry->FullDllName, g_original_steamclient);
        overwrite_loader_string(&entry->BaseDllName, L"steamclient64.dll");
        restored = true;
        break;
    }
    g_ldr_unlock_loader_lock(0, cookie);
    return restored;
}

static HMODULE WINAPI hooked_load_library_ex_w(
    LPCWSTR file_name, HANDLE file, DWORD flags) {
    const wchar_t* filename = filename_part(file_name);
    if (filename && lstrcmpiW(filename, L"steamclient64.dll") == 0) {
        EnterCriticalSection(&g_redirect_lock);

        HMODULE existing_module = nullptr;
        if (g_loaded_steamclient &&
            GetModuleHandleExW(
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                reinterpret_cast<LPCWSTR>(g_loaded_steamclient),
                &existing_module)) {
            LeaveCriticalSection(&g_redirect_lock);
            return existing_module;
        }
        g_loaded_steamclient = nullptr;

        HMODULE module = g_original_load_library_ex_w(
            g_patched_steamclient, file, flags);
        bool names_restored = false;
        if (module) {
            g_loaded_steamclient = module;
            names_restored = restore_original_loader_names(module);
        }
        LeaveCriticalSection(&g_redirect_lock);

        log_message(L"steamclient_rdata_patch: loading Steam-directory patched steamclient64.dll");
        if (module && names_restored)
            log_message(L"steamclient_rdata_patch: restored loader entry path and name to steamclient64.dll");
        else if (module)
            log_message(L"steamclient_rdata_patch: failed to restore loader entry path and name");
        return module;
    }
    return g_original_load_library_ex_w(file_name, file, flags);
}

static void install_hook() {
    MH_STATUS status = MH_Initialize();
    if (status != MH_OK && status != MH_ERROR_ALREADY_INITIALIZED) {
        log_hook_error(L"MH_Initialize", status);
        return;
    }

    void* target = nullptr;
    status = MH_CreateHookApiEx(
        L"kernel32.dll",
        "LoadLibraryExW",
        reinterpret_cast<void*>(&hooked_load_library_ex_w),
        reinterpret_cast<void**>(&g_original_load_library_ex_w),
        &target
    );
    if (status != MH_OK) {
        log_hook_error(L"MH_CreateHookApiEx", status);
        return;
    }

    status = MH_EnableHook(target);
    if (status != MH_OK) {
        log_hook_error(L"MH_EnableHook", status);
        MH_RemoveHook(target);
        return;
    }

    log_message(L"steamclient_rdata_patch: installed Steam-directory LoadLibraryExW redirect");
}

extern "C" __declspec(dllexport)
void MOD_LOADER_CALL on_mod_load(mod_log_fn logger) {
    g_log = logger;
    if (!get_patched_steamclient_path()) {
        log_message(L"steamclient_rdata_patch: steamclient64_patched.dll was not found in the Steam directory");
        return;
    }

    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!ntdll) {
        log_message(L"steamclient_rdata_patch: failed to find ntdll.dll");
        return;
    }
    g_ldr_lock_loader_lock = reinterpret_cast<LdrLockLoaderLockFn>(
        GetProcAddress(ntdll, "LdrLockLoaderLock"));
    g_ldr_unlock_loader_lock = reinterpret_cast<LdrUnlockLoaderLockFn>(
        GetProcAddress(ntdll, "LdrUnlockLoaderLock"));
    if (!g_ldr_lock_loader_lock || !g_ldr_unlock_loader_lock) {
        log_message(L"steamclient_rdata_patch: failed to resolve loader lock functions");
        return;
    }
    InitializeCriticalSection(&g_redirect_lock);
    install_hook();
}
