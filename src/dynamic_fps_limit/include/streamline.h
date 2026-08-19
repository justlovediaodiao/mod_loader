#pragma once

#include <stddef.h>
#include <stdint.h>

namespace streamline {

using FrameGenerationCallback = void (*)(bool active);
// Installs the hooks and applies the initial Reflex fallback before returning.
bool install_hooks(bool use_reflex, uint32_t initial_reflex_fps,
                   FrameGenerationCallback callback, char* message,
                   size_t message_size);

// Must be called from the game thread.
bool set_reflex_fps_limit(uint32_t fps, char* message,
                          size_t message_size);

} // namespace streamline
