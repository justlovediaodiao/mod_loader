#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cwchar>
#include <exception>

#include "mod_loader_api.h"
#include <cstdint>
#include <string>
#include <vector>

static HMODULE g_self;
static mod_log_fn g_log;
static LONG g_started;

static void log(const std::string& message) {
    if (g_log) g_log(("steam_achievement: " + message).c_str());
}

static std::vector<std::string> parse_ids(const std::string& value) {
    std::vector<std::string> ids;
    size_t start = 0;
    while (start <= value.size()) {
        const size_t end = value.find(',', start);
        std::string id = value.substr(start, end - start);
        const size_t first = id.find_first_not_of(" \t\r\n");
        if (first != std::string::npos) {
            id = id.substr(first, id.find_last_not_of(" \t\r\n") - first + 1);
            bool duplicate = false;
            for (const auto& previous : ids) duplicate |= previous == id;
            if (!duplicate) ids.push_back(id);
        }
        if (end == std::string::npos) break;
        start = end + 1;
    }
    return ids;
}

// Steam's flat C exports use the default C calling convention (Windows x64).
struct Api {
    void* stats = nullptr;
    uint32_t (*count)(void*) = nullptr;
    const char* (*name)(void*, uint32_t) = nullptr;
    const char* (*attribute)(void*, const char*, const char*) = nullptr;
    bool (*get)(void*, const char*, bool*) = nullptr;
    bool (*set)(void*, const char*) = nullptr;
    bool (*store)(void*) = nullptr;
};

static void run(const Api& api, bool list, const std::vector<std::string>& ids) {
    if (list) {
        const auto count = api.count(api.stats);
        log("achievement count=" + std::to_string(count));
        for (uint32_t i = 0; i < count; ++i) {
            const char* raw_id = api.name(api.stats, i);
            if (!raw_id || !*raw_id) {
                log("failed to read achievement at index=" + std::to_string(i));
                continue;
            }
            const std::string id(raw_id);
            const char* value = api.attribute(api.stats, id.c_str(), "name");
            const std::string title = value ? value : "";
            value = api.attribute(api.stats, id.c_str(), "desc");
            log("id=" + id + " | title=" + title + " | description=" + (value ? value : ""));
        }
    }

    bool changed = false;
    for (const auto& id : ids) {
        bool unlocked = false;
        if (!api.get(api.stats, id.c_str(), &unlocked)) {
            log("cannot read achievement id=" + id + " (unknown ID or stats unavailable)");
        } else if (unlocked) {
            log("already unlocked: " + id);
        } else if (api.set(api.stats, id.c_str())) {
            changed = true;
            log("SetAchievement accepted: " + id);
        } else {
            log("SetAchievement failed: " + id);
        }
    }
    if (changed) {
        log(api.store(api.stats)
            ? "StoreStats accepted; server persistence is asynchronous and not confirmed by this mod"
            : "StoreStats failed; achievement changes have not been submitted by this mod");
    }
}

static bool read_config(bool& list, std::vector<std::string>& ids) {
    wchar_t path[32768];
    const DWORD length = GetModuleFileNameW(g_self, path, 32768);
    if (!length || length >= 32768) return false;
    wchar_t* slash = wcsrchr(path, L'\\');
    if (!slash) return false;
    const std::wstring config = std::wstring(path, slash + 1) + L"config.ini";
    wchar_t setting[32];
    GetPrivateProfileStringW(L"steam_achievement", L"list_achievement", L"0",
                             setting, 32, config.c_str());
    if (_wcsicmp(setting, L"true") == 0 || wcscmp(setting, L"1") == 0) list = true;
    else if (_wcsicmp(setting, L"false") != 0 && wcscmp(setting, L"0") != 0) {
        log("invalid list_achievement; use 0/1 or false/true");
        return false;
    }
    wchar_t value[32768];
    const DWORD size = GetPrivateProfileStringW(L"steam_achievement", L"achievement_id",
                                               L"", value, 32768, config.c_str());
    if (size >= 32767) {
        log("achievement_id is too long; refusing a truncated list");
        return false;
    }
    if (!size) return true;
    const int bytes = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value,
                                         size, nullptr, 0, nullptr, nullptr);
    if (!bytes) return false;
    std::string utf8(bytes, '\0');
    if (!WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value, size,
                            utf8.data(), bytes, nullptr, nullptr)) return false;
    ids = parse_ids(utf8);
    return true;
}

template <typename T>
static bool resolve(HMODULE module, T& function, const char* name) {
    function = reinterpret_cast<T>(GetProcAddress(module, name));
    if (!function) log(std::string("missing Steam export: ") + name);
    return function != nullptr;
}

static void work() {
    bool list = false;
    std::vector<std::string> ids;
    if (!read_config(list, ids)) {
        log("failed to read config.ini next to the mod DLL");
        return;
    }
    if (!list && ids.empty()) {
        log("disabled; set list_achievement or achievement_id in [steam_achievement]");
        return;
    }

    log("waiting for the game's Steam API and achievement data");
    const ULONGLONG deadline = GetTickCount64() + 60000;
    HMODULE module = nullptr;
    while (GetTickCount64() < deadline) {
        // Pin the loaded module so function pointers remain valid for the worker.
        if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN, L"steam_api64.dll", &module)) break;
        Sleep(1000);
    }
    if (!module) {
        log("timed out waiting for steam_api64.dll");
        return;
    }

    Api api;
    bool valid = true;
    valid &= resolve(module, api.count, "SteamAPI_ISteamUserStats_GetNumAchievements");
    valid &= resolve(module, api.name, "SteamAPI_ISteamUserStats_GetAchievementName");
    valid &= resolve(module, api.get, "SteamAPI_ISteamUserStats_GetAchievement");
    if (list) valid &= resolve(module, api.attribute, "SteamAPI_ISteamUserStats_GetAchievementDisplayAttribute");
    if (!ids.empty()) {
        valid &= resolve(module, api.set, "SteamAPI_ISteamUserStats_SetAchievement");
        valid &= resolve(module, api.store, "SteamAPI_ISteamUserStats_StoreStats");
    }
    if (!valid) return;

    using Accessor = void* (*)();
    Accessor accessor = nullptr;
    for (const char* name : {"SteamAPI_SteamUserStats_v013", "SteamAPI_SteamUserStats_v012",
                             "SteamAPI_SteamUserStats_v011"}) {
        accessor = reinterpret_cast<Accessor>(GetProcAddress(module, name));
        if (accessor) {
            log(std::string("using ") + name);
            break;
        }
    }
    if (!accessor) {
        log("no supported SteamUserStats accessor (v011/v012/v013); this Steam DLL is unsupported");
        return;
    }
    auto request = reinterpret_cast<bool (*)(void*)>(
        GetProcAddress(module, "SteamAPI_ISteamUserStats_RequestCurrentStats"));
    // DLL loading must not consume the time allowed for asynchronous stats loading.
    const ULONGLONG stats_deadline = GetTickCount64() + 60000;
    ULONGLONG next_request = 0;
    while (GetTickCount64() < stats_deadline) {
        api.stats = accessor();
        if (api.stats) {
            // Older SDKs load stats asynchronously. Let the game pump its own
            // callbacks; a successful GetAchievement is the readiness probe.
            const uint32_t count = api.count(api.stats);
            if (count) {
                const char* first = api.name(api.stats, 0);
                bool unlocked = false;
                if (first && *first && api.get(api.stats, first, &unlocked)) {
                    run(api, list, ids);
                    return;
                }
            }
            // Acceptance is not completion: a transient asynchronous failure
            // must not prevent another request for the rest of this launch.
            const ULONGLONG now = GetTickCount64();
            if (request && now >= next_request) {
                request(api.stats);
                next_request = now + 10000;
            }
        }
        Sleep(1000);
    }
    if (api.stats && api.count(api.stats) == 0)
        log("no achievements reported (count=0); the game may have none or Steam data may be unavailable");
    else
        log("timed out waiting for initialized Steam achievement data");
    if (!ids.empty()) log("configured achievements were not unlocked");
}

static DWORD WINAPI worker(void*) {
    try { work(); }
    catch (const std::exception&) {
        if (g_log) g_log("steam_achievement: worker failed with a C++ exception");
    }
    return 0;
}

extern "C" __declspec(dllexport)
void MOD_LOADER_CALL on_mod_load(mod_log_fn logger) {
    if (InterlockedCompareExchange(&g_started, 1, 0) != 0) return;
    g_log = logger;
    HANDLE thread = CreateThread(nullptr, 0, worker, nullptr, 0, nullptr);
    if (thread) CloseHandle(thread);
    else if (g_log) g_log("steam_achievement: failed to create worker thread");
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_self = instance;
        DisableThreadLibraryCalls(instance);
    }
    return TRUE;
}
