#include "platform.h"
#include "raylib_window.h"
#include "raylib_input.h"
#include "raylib_renderer.h"

Platform::Platform(std::unique_ptr<WindowInterface> w,
                   std::unique_ptr<InputInterface> i,
                   std::unique_ptr<RendererInterface> r)
    : window(std::move(w)), input(std::move(i)), renderer(std::move(r)) {}

Platform Platform::CreateRaylibPlatform() {
    auto window = std::make_unique<RaylibWindow>();
    auto input = std::make_unique<RaylibInput>();
    auto renderer = std::make_unique<RaylibRenderer>();

    return Platform(std::move(window), std::move(input), std::move(renderer));
}