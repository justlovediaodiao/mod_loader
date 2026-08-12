// Generic mod loader with the system-DLL proxy copied/adapted from
// UnityDoorstop src/windows/proxy. This file contains no Unity behavior.
// See UPSTREAM.md for the exact source mapping and update procedure.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include "mod_loader_api.h"

static HMODULE g_real_dll;
static wchar_t g_own_dir[MAX_PATH];
extern "C" void load_functions(HMODULE dll);

static CRITICAL_SECTION g_log_lock;

static void MOD_LOADER_CALL mod_log(const wchar_t* text) {
    if (!text) return;
    EnterCriticalSection(&g_log_lock);
    wchar_t path[MAX_PATH];
    wsprintfW(path, L"%s\\mod.log", g_own_dir);
    HANDLE file = CreateFileW(path, FILE_APPEND_DATA, FILE_SHARE_READ, nullptr,
                              OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) { LeaveCriticalSection(&g_log_lock); return; }
    SYSTEMTIME now{};
    GetLocalTime(&now);
    wchar_t line[768];
    int count = wsprintfW(line, L"[%02u:%02u:%02u] %s\r\n", now.wHour,
                          now.wMinute, now.wSecond, text);
    DWORD written;
    WriteFile(file, line, (DWORD)(count * sizeof(wchar_t)), &written, nullptr);
    CloseHandle(file);
    LeaveCriticalSection(&g_log_lock);
}

static void get_own_directory(HMODULE self) {
    GetModuleFileNameW(self, g_own_dir, MAX_PATH);
    wchar_t* slash = g_own_dir;
    for (wchar_t* p = g_own_dir; *p; ++p) if (*p == L'\\' || *p == L'/') slash = p;
    *slash = L'\0';
}

// Adapted directly from UnityDoorstop src/windows/proxy/proxy.h. It first
// tries <proxy-name>_alt.dll beside the proxy, then System32/<proxy-name>.dll.
static HMODULE load_proxy(HMODULE self) {
    wchar_t module_path[MAX_PATH], module_name[MAX_PATH], path[MAX_PATH];
    GetModuleFileNameW(self, module_path, MAX_PATH);
    wchar_t* file = module_path;
    for (wchar_t* p = module_path; *p; ++p) if (*p == L'\\' || *p == L'/') file = p + 1;
    lstrcpynW(module_name, file, MAX_PATH);
    wchar_t* dot = nullptr;
    for (wchar_t* p = module_name; *p; ++p) if (*p == L'.') dot = p;
    if (dot) *dot = L'\0';

    wsprintfW(path, L"%s\\%s_alt.dll", g_own_dir, module_name);
    HMODULE handle = LoadLibraryW(path);
    if (handle) return handle;

    UINT n = GetSystemDirectoryW(path, MAX_PATH);
    if (!n || n >= MAX_PATH - 32) return nullptr;
    lstrcatW(path, L"\\");
    lstrcatW(path, module_name);
    lstrcatW(path, L".dll");
    return LoadLibraryW(path);
}

struct mapped_file {
    HANDLE file = INVALID_HANDLE_VALUE;
    HANDLE mapping = nullptr;
    const uint8_t* data = nullptr;

    ~mapped_file() {
        if (data) UnmapViewOfFile(data);
        if (mapping) CloseHandle(mapping);
        if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
    }
};

static bool map_read_only(const wchar_t* path, mapped_file& mapped) {
    mapped.file = CreateFileW(path, GENERIC_READ,
                              FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                              nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (mapped.file == INVALID_HANDLE_VALUE) return false;
    mapped.mapping = CreateFileMappingW(mapped.file, nullptr,
                                        PAGE_READONLY | SEC_IMAGE_NO_EXECUTE,
                                        0, 0, nullptr);
    if (!mapped.mapping) return false;
    mapped.data = static_cast<const uint8_t*>(
        MapViewOfFile(mapped.mapping, FILE_MAP_READ, 0, 0, 0));
    return mapped.data != nullptr;
}

// Inspect the PE export table before loading the DLL. Loading an arbitrary DLL
// just to call GetProcAddress would already run its process-attach initializer.
static bool has_on_mod_load_export(const wchar_t* path) {
    mapped_file mapped;
    if (!map_read_only(path, mapped)) return false;
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(mapped.data);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew < 0) return false;
    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(
        mapped.data + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE ||
        nt->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 ||
        nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC ||
        nt->OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_EXPORT) return false;

    const DWORD image_size = nt->OptionalHeader.SizeOfImage;
    const auto contains = [image_size](DWORD rva, size_t size) {
        return rva <= image_size && size <= image_size - rva;
    };
    const auto& export_entry =
        nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (!export_entry.VirtualAddress ||
        !contains(export_entry.VirtualAddress, sizeof(IMAGE_EXPORT_DIRECTORY))) return false;
    const auto* exports = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(
        mapped.data + export_entry.VirtualAddress);
    if (!exports->AddressOfNames ||
        !contains(exports->AddressOfNames,
                  static_cast<size_t>(exports->NumberOfNames) * sizeof(DWORD))) return false;

    const auto* names = reinterpret_cast<const DWORD*>(
        mapped.data + exports->AddressOfNames);
    static const char expected_name[] = "on_mod_load";
    for (DWORD i = 0; i < exports->NumberOfNames; ++i) {
        if (!contains(names[i], sizeof(expected_name))) continue;
        const char* name = reinterpret_cast<const char*>(mapped.data + names[i]);
        size_t j = 0;
        while (j < sizeof(expected_name) && name[j] == expected_name[j]) ++j;
        if (j == sizeof(expected_name)) return true;
    }
    return false;
}

static DWORD WINAPI load_mods(void*) {
    wchar_t pattern[MAX_PATH];
    wsprintfW(pattern, L"%s\\mods\\*.dll", g_own_dir);
    WIN32_FIND_DATAW entry{};
    HANDLE find = FindFirstFileW(pattern, &entry);
    if (find == INVALID_HANDLE_VALUE) {
        mod_log(L"No mods directory or no DLL mods found");
        return 0;
    }
    do {
        if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        wchar_t full[MAX_PATH], message[MAX_PATH + 64];
        wsprintfW(full, L"%s\\mods\\%s", g_own_dir, entry.cFileName);
        if (!has_on_mod_load_export(full)) {
            wsprintfW(message, L"Skipped DLL without on_mod_load: %s", entry.cFileName);
            mod_log(message);
            continue;
        }
        HMODULE mod = LoadLibraryW(full);
        if (!mod) {
            wsprintfW(message, L"Failed to load mod: %s", entry.cFileName);
            mod_log(message);
            continue;
        }
        auto on_load = reinterpret_cast<on_mod_load_fn>(GetProcAddress(mod, "on_mod_load"));
        if (!on_load) {
            wsprintfW(message, L"Failed to resolve on_mod_load: %s", entry.cFileName);
            mod_log(message);
            FreeLibrary(mod);
            continue;
        }
        wsprintfW(message, L"Loading mod: %s", entry.cFileName);
        mod_log(message);
        on_load(mod_log);
        wsprintfW(message, L"Loaded mod: %s", entry.cFileName);
        mod_log(message);
    } while (FindNextFileW(find, &entry));
    FindClose(find);
    return 0;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE self, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(self);
        get_own_directory(self);
        InitializeCriticalSection(&g_log_lock);
        g_real_dll = load_proxy(self);
        if (!g_real_dll) return FALSE;
        load_functions(g_real_dll);
        HANDLE thread = CreateThread(nullptr, 0, load_mods, nullptr, 0, nullptr);
        if (thread) CloseHandle(thread);
    } else if (reason == DLL_PROCESS_DETACH) {
        DeleteCriticalSection(&g_log_lock);
    }
    return TRUE;
}
