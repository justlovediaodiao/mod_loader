#include <windows.h>

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "mod_loader_api.h"
#include "streamline.h"

namespace rtss {
bool set_fps_limit(uint32_t fps, char* message, size_t message_size);
}

namespace {

constexpr uint32_t DEFAULT_FRAME_GENERATION_OFF_FPS = 60;
constexpr uint32_t DEFAULT_FRAME_GENERATION_ON_FPS = 120;
constexpr DWORD CHECK_INTERVAL_MS = 300;

HMODULE g_self{};
mod_log_fn g_log{};

bool own_config_path(wchar_t path[MAX_PATH]) {
    const DWORD length = GetModuleFileNameW(g_self, path, MAX_PATH);
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

uint32_t read_config_fps(const wchar_t* path, const wchar_t* key,
                         uint32_t fallback) {
    wchar_t fallback_text[32]{};
    wchar_t value[32]{};
    wsprintfW(fallback_text, L"%u", static_cast<unsigned int>(fallback));
    GetPrivateProfileStringW(L"dynamic_fps", key, fallback_text, value,
                             static_cast<DWORD>(sizeof(value) /
                                                sizeof(value[0])),
                             path);

    wchar_t* end = nullptr;
    errno = 0;
    const unsigned long long parsed = wcstoull(value, &end, 10);
    if (errno == ERANGE || end == value || *end != L'\0' || value[0] == L'-' ||
        parsed > UINT32_MAX) {
        return fallback;
    }
    return static_cast<uint32_t>(parsed);
}

void log(const char* message) {
    if (g_log != nullptr) {
        g_log(message);
    }
}

DWORD WINAPI worker(void*) {
    log("dynamic_fps: Initialized");
    wchar_t config_path[MAX_PATH]{};
    if (!own_config_path(config_path)) {
        log("dynamic_fps: Failed to locate mods\\config.ini; using defaults");
    }
    const uint32_t frame_generation_off_fps =
        config_path[0] != L'\0'
            ? read_config_fps(config_path, L"frame_generation_off_fps",
                              DEFAULT_FRAME_GENERATION_OFF_FPS)
            : DEFAULT_FRAME_GENERATION_OFF_FPS;
    const uint32_t frame_generation_on_fps =
        config_path[0] != L'\0'
            ? read_config_fps(config_path, L"frame_generation_on_fps",
                              DEFAULT_FRAME_GENERATION_ON_FPS)
            : DEFAULT_FRAME_GENERATION_ON_FPS;
    char message[256]{};
    snprintf(message, sizeof(message),
             "dynamic_fps: Frame generation OFF: %u FPS; ON: %u FPS",
             static_cast<unsigned int>(frame_generation_off_fps),
             static_cast<unsigned int>(frame_generation_on_fps));
    log(message);

    uint32_t current_fps = 0;
    bool has_current_fps = false;
    bool rtss_warning_logged = false;
    bool streamline_warning_logged = false;
    bool frame_generation_active = false;

    for (;;) {
        const streamline::FrameGenerationState frame_generation_state =
            streamline::get_frame_generation_state(message, sizeof(message));
        if (frame_generation_state ==
            streamline::FrameGenerationState::error) {
            if (!streamline_warning_logged) {
                streamline_warning_logged = true;
                log(message);
            }
        } else {
            streamline_warning_logged = false;
            frame_generation_active =
                frame_generation_state ==
                streamline::FrameGenerationState::active;
        }
        const uint32_t target_fps = frame_generation_active
                                        ? frame_generation_on_fps
                                        : frame_generation_off_fps;
        if (!has_current_fps || target_fps != current_fps) {
            if (rtss::set_fps_limit(target_fps, message, sizeof(message))) {
                current_fps = target_fps;
                has_current_fps = true;
                rtss_warning_logged = false;
                log(message);
            } else if (!rtss_warning_logged) {
                rtss_warning_logged = true;
                log(message);
            }
        }
        Sleep(CHECK_INTERVAL_MS);
    }
}

} // namespace

extern "C" __declspec(dllexport) void MOD_LOADER_CALL
on_mod_load(mod_log_fn logger) {
    g_log = logger;
    const HANDLE thread = CreateThread(nullptr, 0, worker, nullptr, 0, nullptr);
    if (thread != nullptr) {
        CloseHandle(thread);
    } else {
        log("dynamic_fps: Failed to create worker thread");
    }
}

extern "C" BOOL WINAPI DllMain(HMODULE module, DWORD reason, void*) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_self = module;
        DisableThreadLibraryCalls(module);
    }
    return TRUE;
}
