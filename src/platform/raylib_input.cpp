#include "raylib_input.h"

bool RaylibInput::IsKeyPressed(int key) const {
    return ::IsKeyPressed(key);
}

bool RaylibInput::IsKeyDown(int key) const {
    return ::IsKeyDown(key);
}