#include <gtest/gtest.h>
#include "raylib.h"
#include "platform/platform.h"
#include "platform/interface/window_interface.h"
#include <cmath>

namespace {
struct WindowGuard {
    WindowGuard(int w, int h, const char* title, unsigned int flags = 0) {
        if (flags != 0) SetConfigFlags(flags);
        InitWindow(w, h, title);
    }
    ~WindowGuard() {
        if (IsWindowReady()) CloseWindow();
    }
};
}

TEST(Smoke, RenderLoopStability) {
    WindowGuard win(640, 360, "render_stability", FLAG_WINDOW_HIDDEN | FLAG_MSAA_4X_HINT);
    SetTargetFPS(60);
    for (int i = 0; i < 10; ++i) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("frame", 10, 10, 10, BLACK);
        EndDrawing();
    }
    SUCCEED();
}

TEST(Smoke, InputPolling) {
    WindowGuard win(320, 200, "input_poll", FLAG_WINDOW_HIDDEN);
    // No real input injection; just ensure calls are safe.
    bool anyDown = IsKeyDown(KEY_A) || IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    (void)anyDown;
    SUCCEED();
}

TEST(Smoke, WindowResize) {
    WindowGuard win(300, 200, "resize", FLAG_WINDOW_HIDDEN);
    SetWindowSize(640, 480);
    // Allow a couple of frames for internal resize handling
    for (int i = 0; i < 2; ++i) {
        BeginDrawing();
        ClearBackground(BLACK);
        EndDrawing();
    }
    EXPECT_EQ(GetScreenWidth(), 640);
    EXPECT_EQ(GetScreenHeight(), 480);
}

TEST(Smoke, AssetLoadFailure) {
    WindowGuard win(320, 240, "asset_fail", FLAG_WINDOW_HIDDEN);
    SetTraceLogLevel(LOG_NONE); // suppress error spam in test output
    Texture2D tex = LoadTexture("assets/__does_not_exist__.png");
    bool ready = tex.id != 0;
    if (ready) {
        UnloadTexture(tex);
    }
    EXPECT_FALSE(ready);
}

TEST(Smoke, PlatformInitAndFrame) {
    Platform platform = Platform::CreateRaylibPlatform();
    platform.window->Init(200, 150, "platform_smoke");
    platform.window->BeginFrame();
    ClearBackground(RAYWHITE);
    platform.window->EndFrame();
    platform.window->Close();
    SUCCEED();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
