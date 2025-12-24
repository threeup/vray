#pragma once

/**
 * @brief An interface for platform input.
 *
 * This provides an abstraction over the input system, allowing different
 * backends (like raylib or native APIs) to be used to query keyboard,
 * mouse, and gamepad state.
 */
class InputInterface {
public:
    virtual ~InputInterface() = default;

    [[nodiscard]] virtual bool IsKeyPressed(int key) const = 0;
    [[nodiscard]] virtual bool IsKeyDown(int key) const = 0;
    // Add other methods like GetMousePosition, IsMouseButtonDown, etc. as needed
};