#include <windows.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

namespace {

using LoadProfile = void(WINAPI*)(LPCSTR);
using SaveProfile = void(WINAPI*)(LPCSTR);
using SetProfileProperty = BOOL(WINAPI*)(LPCSTR, LPBYTE, DWORD);
using UpdateProfiles = void(WINAPI*)();

struct RtssState {
    LoadProfile load_profile{};
    SaveProfile save_profile{};
    SetProfileProperty set_property{};
    UpdateProfiles update_profiles{};
    char profile[MAX_PATH]{};
    bool incompatible{};
};

RtssState g_state;

template <typename T>
void load_export(HMODULE module, const char* name, T& destination) {
    const auto address = GetProcAddress(module, name);
    static_assert(sizeof(destination) == sizeof(address));
    memcpy(&destination, &address, sizeof(destination));
}

bool initialize() {
    if (g_state.load_profile != nullptr) {
        return true;
    }
    if (g_state.incompatible) {
        return false;
    }

    const HMODULE module = GetModuleHandleW(L"RTSSHooks64.dll");
    if (module == nullptr) {
        return false;
    }

    RtssState api;
    load_export(module, "LoadProfile", api.load_profile);
    load_export(module, "SaveProfile", api.save_profile);
    load_export(module, "SetProfileProperty", api.set_property);
    load_export(module, "UpdateProfiles", api.update_profiles);

    if (api.load_profile == nullptr || api.save_profile == nullptr ||
        api.set_property == nullptr || api.update_profiles == nullptr) {
        g_state.incompatible = true;
        return false;
    }

    g_state = api;
    return true;
}

bool initialize_profile() {
    if (g_state.profile[0] != '\0') {
        return true;
    }

    wchar_t executable[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) {
        return false;
    }

    const wchar_t* filename = wcsrchr(executable, L'\\');
    filename = filename == nullptr ? executable : filename + 1;
    return WideCharToMultiByte(CP_ACP, 0, filename, -1, g_state.profile,
                               MAX_PATH, nullptr, nullptr) > 0;
}

} // namespace

namespace rtss {

bool set_fps_limit(uint32_t fps, char* message, size_t message_size) {
    if (!initialize()) {
        snprintf(message, message_size,
                 "dynamic_fps_limit: RTSS is unavailable or incompatible");
        return false;
    }
    if (!initialize_profile()) {
        snprintf(message, message_size,
                 "dynamic_fps_limit: Failed to determine the game profile");
        return false;
    }

    DWORD denominator = 1;
    DWORD value = fps;
    g_state.load_profile(g_state.profile);

    if (!g_state.set_property("FramerateLimitDenominator",
                              reinterpret_cast<LPBYTE>(&denominator),
                              sizeof(denominator)) ||
        !g_state.set_property("FramerateLimit", reinterpret_cast<LPBYTE>(&value),
                              sizeof(value))) {
        snprintf(message, message_size,
                 "dynamic_fps_limit: Failed to update RTSS profile %s",
                 g_state.profile);
        return false;
    }

    g_state.save_profile(g_state.profile);
    g_state.update_profiles();

    snprintf(message, message_size,
             "dynamic_fps_limit: RTSS profile %s set to %u FPS",
             g_state.profile, static_cast<unsigned int>(fps));
    return true;
}

} // namespace rtss
