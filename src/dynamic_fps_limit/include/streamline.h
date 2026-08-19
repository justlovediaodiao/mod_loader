#pragma once

#include <stddef.h>
#include <stdint.h>

namespace streamline {

enum class FrameGenerationState {
    error,
    inactive,
    active,
};

FrameGenerationState get_frame_generation_state(char* message,
                                                 size_t message_size);

// Installs the Reflex hook. Intercepted calls use initial_fps until polling
// supplies the first runtime state.
bool install_reflex_hook(uint32_t initial_fps, char* message,
                         size_t message_size);

bool set_reflex_fps_limit(uint32_t fps, char* message,
                          size_t message_size);

} // namespace streamline
