#pragma once

#include <stddef.h>

namespace streamline {

enum class FrameGenerationState {
    error,
    inactive,
    active,
};

FrameGenerationState get_frame_generation_state(char* message,
                                                 size_t message_size);

} // namespace streamline
