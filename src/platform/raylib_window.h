#pragma once

#include "interface/window_interface.h"

class RaylibWindow : public WindowInterface {
public:
    RaylibWindow();
    ~RaylibWindow() override;

    void Init(int width, int height, const char* title) override;
    [[nodiscard]] bool ShouldClose() override;
    void Close() override;
    void BeginFrame() override;
    void EndFrame() override;

    [[nodiscard]] int GetWidth() const override;
    [[nodiscard]] int GetHeight() const override;

    [[nodiscard]] void* GetHandle() override;
};