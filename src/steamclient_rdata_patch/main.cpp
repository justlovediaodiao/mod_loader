#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stddef.h>
#include <string.h>
#include <wchar.h>

#include <MinHook.h>

#include "mod_loader_api.h"

using LoadLibraryExWFn = HMODULE(WINAPI*)(LPCWSTR, HANDLE, DWORD);

static mod_log_fn g_log;
static LoadLibraryExWFn g_original_load_library_ex_w;
static volatile LONG g_patch_logged;
static volatile LONG g_patch_error_logged;

static void log_message(const wchar_t* message) {
    if (g_log) g_log(message);
}

static void log_hook_error(const wchar_t* operation, MH_STATUS status) {
    wchar_t message[256];
    wsprintfW(message, L"steamclient_rdata_patch: %s failed (MinHook status %d)",
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

static bool make_rdata_writable(HMODULE module, DWORD* error) {
    if (error) *error = ERROR_BAD_EXE_FORMAT;
    if (!module) return false;

    auto* base = reinterpret_cast<unsigned char*>(module);
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0)
        return false;

    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(
        base + static_cast<size_t>(dos->e_lfanew));
    if (nt->Signature != IMAGE_NT_SIGNATURE ||
        nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        return false;

    const IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(nt);
    for (WORD index = 0; index < nt->FileHeader.NumberOfSections;
         ++index, ++section) {
        constexpr char rdata_name[] = ".rdata";
        if (memcmp(section->Name, rdata_name, sizeof(rdata_name) - 1) != 0 ||
            section->Name[sizeof(rdata_name) - 1] != 0)
            continue;

        SIZE_T size = section->Misc.VirtualSize;
        if (!size) size = section->SizeOfRawData;
        if (!size) return false;

        DWORD old_protection = 0;
        if (!VirtualProtect(base + section->VirtualAddress, size,
                            PAGE_READWRITE, &old_protection)) {
            if (error) *error = GetLastError();
            return false;
        }
        if (error) *error = ERROR_SUCCESS;
        return true;
    }

    return false;
}

static void patch_and_log(HMODULE module) {
    DWORD error = ERROR_SUCCESS;
    if (make_rdata_writable(module, &error)) {
        if (InterlockedCompareExchange(&g_patch_logged, 1, 0) == 0)
            log_message(L"steamclient_rdata_patch: made steamclient64.dll .rdata writable in this process");
        return;
    }

    if (InterlockedCompareExchange(&g_patch_error_logged, 1, 0) == 0) {
        wchar_t message[256];
        wsprintfW(message,
                  L"steamclient_rdata_patch: failed to make .rdata writable (Win32 error %lu)",
                  error);
        log_message(message);
    }
}

static HMODULE WINAPI hooked_load_library_ex_w(
    LPCWSTR file_name, HANDLE file, DWORD flags) {
    HMODULE module = g_original_load_library_ex_w(file_name, file, flags);
    const wchar_t* filename = filename_part(file_name);
    if (module && filename &&
        lstrcmpiW(filename, L"steamclient64.dll") == 0)
        patch_and_log(module);
    return module;
}

static bool install_hook() {
    MH_STATUS status = MH_Initialize();
    if (status != MH_OK && status != MH_ERROR_ALREADY_INITIALIZED) {
        log_hook_error(L"MH_Initialize", status);
        return false;
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
        return false;
    }

    status = MH_EnableHook(target);
    if (status != MH_OK) {
        log_hook_error(L"MH_EnableHook", status);
        MH_RemoveHook(target);
        return false;
    }

    log_message(L"steamclient_rdata_patch: installed LoadLibraryExW post-load patch");
    return true;
}

extern "C" __declspec(dllexport)
void MOD_LOADER_CALL on_mod_load(mod_log_fn logger) {
    g_log = logger;
    HMODULE existing = GetModuleHandleW(L"steamclient64.dll");
    if (existing) patch_and_log(existing);
    install_hook();
}
