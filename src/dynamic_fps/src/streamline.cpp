#include <windows.h>

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "streamline.h"
#include "streamline_api.h"

namespace {

template <typename T>
void load_export(HMODULE module, const char* name, T& destination) {
    const auto address = GetProcAddress(module, name);
    static_assert(sizeof(destination) == sizeof(address));
    memcpy(&destination, &address, sizeof(destination));
}

streamline::DLSSGGetState initialize_dlssg(char* message,
                                           size_t message_size) {
    static streamline::DLSSGGetState get_state{};
    if (get_state != nullptr) {
        return get_state;
    }

    const HMODULE module = GetModuleHandleW(L"sl.interposer.dll");
    streamline::GetFeatureFunction get_feature_function{};
    if (module != nullptr) {
        load_export(module, "slGetFeatureFunction", get_feature_function);
    }

    void* address{};
    const bool initialized =
        get_feature_function != nullptr &&
        get_feature_function(streamline::FEATURE_DLSS_G,
                             "slDLSSGGetState", address) ==
            streamline::Result::ok &&
        address != nullptr;
    if (!initialized) {
        snprintf(message, message_size,
                 "dynamic_fps: NVIDIA DLSS-G is not initialized");
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
                 "dynamic_fps: Failed to query NVIDIA DLSS-G state");
        return FrameGenerationState::error;
    }

    if (state.status != DLSSGStatus::ok) {
        snprintf(message, message_size,
                 "dynamic_fps: NVIDIA DLSS-G is in an invalid runtime state");
        return FrameGenerationState::error;
    }

    return state.num_frames_actually_presented > 1
               ? FrameGenerationState::active
               : FrameGenerationState::inactive;
}

} // namespace streamline
