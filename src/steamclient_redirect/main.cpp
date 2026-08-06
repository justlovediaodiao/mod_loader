#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <wchar.h>

#include "mod_loader_api.h"

using LoadLibraryExWFn = HMODULE(WINAPI*)(LPCWSTR, HANDLE, DWORD);

static mod_log_fn g_log;
static LoadLibraryExWFn g_original_load_library_ex_w;
static wchar_t g_local_steamclient[MAX_PATH];
static volatile LONG g_redirect_logged;

static void log_message(const wchar_t* message) {
    if (g_log) g_log(message);
}

static bool build_local_steamclient_path() {
    DWORD length = GetModuleFileNameW(nullptr, g_local_steamclient, MAX_PATH);
    if (!length || length >= MAX_PATH) return false;

    wchar_t* slash = nullptr;
    for (wchar_t* p = g_local_steamclient; *p; ++p)
        if (*p == L'\\' || *p == L'/') slash = p;
    if (!slash) return false;

    constexpr wchar_t filename[] = L"steamclient64.dll";
    const size_t directory_length = static_cast<size_t>(slash - g_local_steamclient + 1);
    constexpr size_t filename_length = sizeof(filename) / sizeof(filename[0]);
    if (directory_length + filename_length > MAX_PATH) return false;

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

static bool image_range_valid(
    uintptr_t image_base, size_t image_size, uintptr_t address, size_t size) {
    if (address < image_base || size > image_size) return false;
    const uintptr_t offset = address - image_base;
    return offset <= image_size - size;
}

static bool patch_steam_api_import(HMODULE steam_api) {
    const uintptr_t base = reinterpret_cast<uintptr_t>(steam_api);
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0) return false;

    const uintptr_t nt_address = base + static_cast<uintptr_t>(dos->e_lfanew);
    if (nt_address < base) return false;
    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(nt_address);
    if (nt->Signature != IMAGE_NT_SIGNATURE ||
        nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) return false;

    const size_t image_size = nt->OptionalHeader.SizeOfImage;
    if (!image_range_valid(base, image_size, nt_address, sizeof(*nt))) return false;

    const auto& import_directory =
        nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (!import_directory.VirtualAddress ||
        import_directory.Size < sizeof(IMAGE_IMPORT_DESCRIPTOR)) return false;

    const uintptr_t imports_address = base + import_directory.VirtualAddress;
    if (imports_address < base ||
        !image_range_valid(base, image_size, imports_address,
                           import_directory.Size)) return false;

    auto* descriptor = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(imports_address);
    const size_t descriptor_count =
        import_directory.Size / sizeof(IMAGE_IMPORT_DESCRIPTOR);

    for (size_t descriptor_index = 0;
         descriptor_index < descriptor_count && descriptor->Name;
         ++descriptor_index, ++descriptor) {
        if (!descriptor->OriginalFirstThunk || !descriptor->FirstThunk) continue;

        const uintptr_t lookup_address = base + descriptor->OriginalFirstThunk;
        const uintptr_t iat_address = base + descriptor->FirstThunk;
        if (lookup_address < base || iat_address < base) continue;

        auto* lookup = reinterpret_cast<IMAGE_THUNK_DATA64*>(lookup_address);
        auto* iat = reinterpret_cast<IMAGE_THUNK_DATA64*>(iat_address);

        for (size_t thunk_index = 0;; ++thunk_index, ++lookup, ++iat) {
            const uintptr_t lookup_item = reinterpret_cast<uintptr_t>(lookup);
            const uintptr_t iat_item = reinterpret_cast<uintptr_t>(iat);
            if (!image_range_valid(base, image_size, lookup_item, sizeof(*lookup)) ||
                !image_range_valid(base, image_size, iat_item, sizeof(*iat))) break;
            if (!lookup->u1.AddressOfData) break;
            if (IMAGE_SNAP_BY_ORDINAL64(lookup->u1.Ordinal)) continue;

            const uintptr_t name_address = base + lookup->u1.AddressOfData;
            if (name_address < base ||
                !image_range_valid(base, image_size, name_address,
                                   sizeof(IMAGE_IMPORT_BY_NAME))) continue;
            const auto* import =
                reinterpret_cast<const IMAGE_IMPORT_BY_NAME*>(name_address);
            if (lstrcmpA(reinterpret_cast<const char*>(import->Name),
                         "LoadLibraryExW") != 0) continue;

            auto* slot = reinterpret_cast<void**>(&iat->u1.Function);
            auto original = reinterpret_cast<LoadLibraryExWFn>(*slot);
            if (!original) return false;

            DWORD old_protection;
            if (!VirtualProtect(slot, sizeof(*slot), PAGE_READWRITE,
                                &old_protection)) return false;
            g_original_load_library_ex_w = original;
            InterlockedExchangePointer(
                slot, reinterpret_cast<void*>(&redirected_load_library_ex_w));
            DWORD ignored;
            VirtualProtect(slot, sizeof(*slot), old_protection, &ignored);
            FlushInstructionCache(GetCurrentProcess(), slot, sizeof(*slot));
            return true;
        }
    }
    return false;
}

static void install_for_module(HMODULE steam_api) {
    if (GetModuleHandleW(L"steamclient64.dll"))
        log_message(L"steamclient_redirect: warning: steamclient64.dll was already loaded");
    if (patch_steam_api_import(steam_api))
        log_message(L"steamclient_redirect: installed LoadLibraryExW IAT redirect");
    else
        log_message(L"steamclient_redirect: LoadLibraryExW import was not found");
}

static DWORD WINAPI install_worker(void*) {
    // steam_api64.dll is normally a static game dependency and is already
    // present. Polling also covers games that load it shortly after startup.
    for (unsigned attempt = 0; attempt < 3000; ++attempt) {
        HMODULE steam_api = GetModuleHandleW(L"steam_api64.dll");
        if (steam_api) {
            install_for_module(steam_api);
            return 0;
        }
        Sleep(10);
    }
    log_message(L"steamclient_redirect: timed out waiting for steam_api64.dll");
    return 1;
}

extern "C" __declspec(dllexport)
void MOD_LOADER_CALL on_mod_load(mod_log_fn logger) {
    g_log = logger;
    if (!build_local_steamclient_path()) {
        log_message(L"steamclient_redirect: local steamclient64.dll was not found beside the game executable");
        return;
    }

    HMODULE steam_api = GetModuleHandleW(L"steam_api64.dll");
    if (steam_api) {
        install_for_module(steam_api);
        return;
    }

    HANDLE thread = CreateThread(nullptr, 0, install_worker, nullptr, 0, nullptr);
    if (thread) CloseHandle(thread);
    else log_message(L"steamclient_redirect: failed to create install worker");
}
