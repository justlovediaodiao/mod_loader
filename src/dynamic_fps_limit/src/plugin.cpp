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

enum class Limiter {
    reflex,
    rtss,
};

struct Config {
    uint32_t frame_generation_off_fps{DEFAULT_FRAME_GENERATION_OFF_FPS};
    uint32_t frame_generation_on_fps{DEFAULT_FRAME_GENERATION_ON_FPS};
    Limiter limiter{Limiter::reflex};
};

HMODULE g_self{};
mod_log_fn g_log{};
Config g_config{};

void log(const char* message);

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
    GetPrivateProfileStringW(L"dynamic_fps_limit", key, fallback_text, value,
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

Limiter read_config_limiter(const wchar_t* path) {
    wchar_t value[32]{};
    GetPrivateProfileStringW(L"dynamic_fps_limit", L"limiter", L"reflex", value,
                             static_cast<DWORD>(sizeof(value) /
                                                sizeof(value[0])),
                             path);
    if (_wcsicmp(value, L"rtss") == 0) {
        return Limiter::rtss;
    }
    if (_wcsicmp(value, L"reflex") != 0) {
        log("dynamic_fps_limit: Invalid limiter in config.ini; using reflex");
    }
    return Limiter::reflex;
}

Config read_config(const wchar_t* path) {
    Config config{};
    if (path == nullptr) {
        return config;
    }
    config.frame_generation_off_fps = read_config_fps(
        path, L"frame_generation_off_fps",
        DEFAULT_FRAME_GENERATION_OFF_FPS);
    config.frame_generation_on_fps = read_config_fps(
        path, L"frame_generation_on_fps", DEFAULT_FRAME_GENERATION_ON_FPS);
    config.limiter = read_config_limiter(path);
    return config;
}

void log(const char* message) {
    if (g_log != nullptr) {
        g_log(message);
    }
}

void on_frame_generation_change(bool active) {
    const uint32_t target_fps = active
                                    ? g_config.frame_generation_on_fps
                                    : g_config.frame_generation_off_fps;
    char message[256]{};
    bool applied{};
    if (g_config.limiter == Limiter::reflex) {
        applied = streamline::set_reflex_fps_limit(
            target_fps, message, sizeof(message));
    } else {
        applied = rtss::set_fps_limit(target_fps, message, sizeof(message));
    }

    if (!applied) {
        log(message);
    } else {
        snprintf(message, sizeof(message),
                 "dynamic_fps_limit: frame generation %s; set %s limit to %u FPS",
                 active ? "ON" : "OFF",
                 g_config.limiter == Limiter::reflex ? "Reflex" : "RTSS",
                 static_cast<unsigned int>(target_fps));
        log(message);
    }
}

void initialize() {
    wchar_t config_path[MAX_PATH]{};
    if (!own_config_path(config_path)) {
        log("dynamic_fps_limit: Failed to locate mods\\config.ini; using defaults");
    }
    g_config =
        read_config(config_path[0] != L'\0' ? config_path : nullptr);

    char message[256]{};
    if (!streamline::install_hooks(
            g_config.limiter == Limiter::reflex,
            g_config.frame_generation_off_fps, on_frame_generation_change,
            message, sizeof(message))) {
        log(message);
        return;
    }
}

} // namespace

extern "C" __declspec(dllexport) void MOD_LOADER_CALL
on_mod_load(mod_log_fn logger) {
    g_log = logger;
    initialize();
}

extern "C" BOOL WINAPI DllMain(HMODULE module, DWORD reason, void*) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_self = module;
        DisableThreadLibraryCalls(module);
    }
    return TRUE;
}
