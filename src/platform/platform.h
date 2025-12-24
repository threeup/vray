#pragma once

#include "interface/window_interface.h"
#include "interface/input_interface.h"
#include "interface/renderer_interface.h"
#include <memory>

/**
 * @brief A context that holds all platform-specific systems.
 *
 * This struct owns the window, input, and renderer implementations, providing
 * a single point of access for the rest of the application.
 */
struct Platform {
    std::unique_ptr<WindowInterface> window;
    std::unique_ptr<InputInterface> input;
    std::unique_ptr<RendererInterface> renderer;

    // Factory function to create a platform context with a specific backend.
    static Platform CreateRaylibPlatform();

private:
    // Private constructor to enforce creation via factory.
    Platform(std::unique_ptr<WindowInterface> w,
             std::unique_ptr<InputInterface> i,
             std::unique_ptr<RendererInterface> r);
};