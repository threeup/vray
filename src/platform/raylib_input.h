#pragma once

#include "interface/input_interface.h"
#include "raylib.h"

class RaylibInput : public InputInterface {
public:
    RaylibInput() = default;
    ~RaylibInput() override = default;

    [[nodiscard]] bool IsKeyPressed(int key) const override;
    [[nodiscard]] bool IsKeyDown(int key) const override;
};