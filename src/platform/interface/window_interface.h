#pragma once

/**
 * @brief An interface for a platform window.
 *
 * This provides an abstraction over the windowing system, allowing different
 * backends like raylib, SDL, or native APIs to be used.
 */
class WindowInterface {
public:
    virtual ~WindowInterface() = default;

    virtual void Init(int width, int height, const char* title) = 0;
    [[nodiscard]] virtual bool ShouldClose() = 0;
    virtual void Close() = 0;
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    [[nodiscard]] virtual int GetWidth() const = 0;
    [[nodiscard]] virtual int GetHeight() const = 0;
    [[nodiscard]] virtual void* GetHandle() = 0;
};