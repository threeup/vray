#include "raylib_window.h"
#include "raylib.h"

RaylibWindow::RaylibWindow() {
    // Constructor
}

RaylibWindow::~RaylibWindow() {
    if (IsWindowReady()) {
        CloseWindow();
    }
}

void RaylibWindow::Init(int width, int height, const char* title) {
    InitWindow(width, height, title);
}

bool RaylibWindow::ShouldClose() {
    return WindowShouldClose();
}

void RaylibWindow::Close() {
    CloseWindow();
}

void RaylibWindow::BeginFrame() {
    BeginDrawing();
}

void RaylibWindow::EndFrame() {
    EndDrawing();
}

int RaylibWindow::GetWidth() const { return GetScreenWidth(); }
int RaylibWindow::GetHeight() const { return GetScreenHeight(); }

void* RaylibWindow::GetHandle() { return GetWindowHandle(); }