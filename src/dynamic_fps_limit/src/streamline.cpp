#include <windows.h>

#include <atomic>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <MinHook.h>

#include "streamline.h"
#include "streamline_api.h"

namespace {

template <typename T>
void load_export(HMODULE module, const char* name, T& destination) {
    const auto address = GetProcAddress(module, name);
    static_assert(sizeof(destination) == sizeof(address));
    memcpy(&destination, &address, sizeof(destination));
}

struct ReflexContext {
    ReflexContext() {
        last_options.mode = streamline::ReflexMode::low_latency;
    }

    CRITICAL_SECTION set_options_lock{};
    bool set_options_lock_initialized{};
    std::atomic<uint32_t> frame_limit_us{};
    streamline::ReflexOptions last_options{};
    streamline::ReflexSetOptions original_set_options{};
};

ReflexContext g_reflex{};

uint32_t fps_to_frame_limit_us(uint32_t fps) {
    if (fps == 0) {
        return 0;
    }
    const uint64_t rounded = (1000000ULL + fps / 2ULL) / fps;
    return rounded == 0 ? 1 : static_cast<uint32_t>(rounded);
}

streamline::Result hooked_reflex_set_options(
    streamline::ReflexOptions& options) {
    EnterCriticalSection(&g_reflex.set_options_lock);

    const uint32_t original_frame_limit_us = options.frame_limit_us;
    g_reflex.last_options = options;
    // The cache contains only the v1 prefix. Do not retain a game-owned chain
    // or advertise the game's potentially larger structure version.
    g_reflex.last_options.next = nullptr;
    g_reflex.last_options.struct_version = streamline::STRUCT_VERSION_1;

    options.frame_limit_us =
        g_reflex.frame_limit_us.load(std::memory_order_relaxed);
    const streamline::Result result =
        g_reflex.original_set_options(options);
    options.frame_limit_us = original_frame_limit_us;

    LeaveCriticalSection(&g_reflex.set_options_lock);
    return result;
}

void format_hook_error(char* message, size_t message_size,
                       const char* operation, MH_STATUS status) {
    snprintf(message, message_size,
             "dynamic_fps_limit: %s failed (MinHook status %d)", operation,
             static_cast<int>(status));
}

bool find_feature_function(uint32_t feature, const char* function_name,
                           void*& hook_target, char* message,
                           size_t message_size) {
    const HMODULE interposer = GetModuleHandleW(L"sl.interposer.dll");
    if (interposer == nullptr) {
        snprintf(message, message_size,
                 "dynamic_fps_limit: sl.interposer.dll is not loaded");
        return false;
    }

    streamline::GetFeatureFunction get_feature_function{};
    load_export(interposer, "slGetFeatureFunction", get_feature_function);
    if (get_feature_function == nullptr) {
        snprintf(message, message_size,
                 "dynamic_fps_limit: slGetFeatureFunction is unavailable");
        return false;
    }

    hook_target = nullptr;
    const streamline::Result result = get_feature_function(
        feature, function_name, hook_target);
    if (result != streamline::Result::ok || hook_target == nullptr) {
        snprintf(message, message_size,
                 "dynamic_fps_limit: %s is not initialized (%d)", function_name,
                 static_cast<int>(result));
        return false;
    }

    return true;
}

bool create_hook(void* hook_target, void* detour, void** original,
                 char* message, size_t message_size) {
    const MH_STATUS status = MH_CreateHook(hook_target, detour, original);
    if (status != MH_OK) {
        format_hook_error(message, message_size, "MH_CreateHook", status);
        return false;
    }
    return true;
}

void remove_reflex_hook(void* hook_target) {
    if (hook_target != nullptr && MH_RemoveHook(hook_target) == MH_OK) {
        g_reflex.original_set_options = nullptr;
    }
}

streamline::DLSSGGetState initialize_dlssg(char* message,
                                           size_t message_size) {
    static streamline::DLSSGGetState get_state{};
    if (get_state != nullptr) {
        return get_state;
    }

    void* address{};
    if (!find_feature_function(streamline::FEATURE_DLSS_G,
                               "slDLSSGGetState", address, message,
                               message_size)) {
        return nullptr;
    }

    static_assert(sizeof(get_state) == sizeof(address));
    memcpy(&get_state, &address, sizeof(get_state));
    return get_state;
}

} // namespace

namespace streamline {

FrameGenerationState get_frame_generation_state(char* message,
                                                size_t message_size) {
    if (message_size > 0) {
        message[0] = '\0';
    }

    const DLSSGGetState get_state = initialize_dlssg(message, message_size);
    if (get_state == nullptr) {
        return FrameGenerationState::error;
    }

    ViewportHandle viewport{0};
    DLSSGState state{};
    if (get_state(viewport, state, nullptr) != Result::ok) {
        snprintf(message, message_size,
                 "dynamic_fps_limit: Failed to query NVIDIA DLSS-G state");
        return FrameGenerationState::error;
    }

    if (state.status != DLSSGStatus::ok) {
        snprintf(message, message_size,
                 "dynamic_fps_limit: NVIDIA DLSS-G is in an invalid runtime "
                 "state");
        return FrameGenerationState::error;
    }

    return state.num_frames_actually_presented > 1
               ? FrameGenerationState::active
               : FrameGenerationState::inactive;
}

bool install_reflex_hook(uint32_t initial_fps, char* message,
                         size_t message_size) {
    if (message_size > 0) {
        message[0] = '\0';
    }

    void* hook_target{};
    if (!find_feature_function(FEATURE_REFLEX, "slReflexSetOptions",
                               hook_target, message, message_size)) {
        return false;
    }
    MH_STATUS status = MH_Initialize();
    if (status != MH_OK && status != MH_ERROR_ALREADY_INITIALIZED) {
        format_hook_error(message, message_size, "MH_Initialize", status);
        return false;
    }

    g_reflex.frame_limit_us.store(fps_to_frame_limit_us(initial_fps),
                                 std::memory_order_relaxed);
    if (!g_reflex.set_options_lock_initialized) {
        InitializeCriticalSection(&g_reflex.set_options_lock);
        g_reflex.set_options_lock_initialized = true;
    }

    if (!create_hook(
            hook_target,
            reinterpret_cast<void*>(&hooked_reflex_set_options),
            reinterpret_cast<void**>(&g_reflex.original_set_options), message,
            message_size)) {
        return false;
    }
    status = MH_EnableHook(hook_target);
    if (status != MH_OK) {
        format_hook_error(message, message_size, "MH_EnableHook", status);
        remove_reflex_hook(hook_target);
        return false;
    }
    return true;
}

bool set_reflex_fps_limit(uint32_t fps, char* message,
                          size_t message_size) {
    if (message_size > 0) {
        message[0] = '\0';
    }
    g_reflex.frame_limit_us.store(fps_to_frame_limit_us(fps),
                                 std::memory_order_relaxed);
    const Result result = hooked_reflex_set_options(g_reflex.last_options);

    if (result != Result::ok) {
        snprintf(message, message_size,
                 "dynamic_fps_limit: slReflexSetOptions failed (%d)",
                 static_cast<int>(result));
        return false;
    }

    snprintf(message, message_size,
             "dynamic_fps_limit: Reflex limit set to %u FPS",
             static_cast<unsigned int>(fps));
    return true;
}

} // namespace streamline
