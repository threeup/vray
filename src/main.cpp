#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "platform/platform.h"
#include "raylib.h" // Still needed for types like Color, Vector3, etc.
#include "app.h"
#include "game.h"
#include "card.h"
#include "camControl.h"
#include "render.h"
#include "ui.h"
#include <cmath>
#include <string>

int main() {
    // Create and initialize the platform context
    Platform platform = Platform::CreateRaylibPlatform();
    platform.window->Init(800, 600, "vray ver1");

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    // Create main application context
    AppContext ctx {
        .window = platform.window,
        .input = platform.input,
        .renderer = platform.renderer
    };

    // Initialize 3D camera
    initializeCamera(ctx.camera);
    ctx.camera.fovy = 45.0f;
    ctx.camera.projection = CAMERA_PERSPECTIVE;

    // Create Game State
    Game game;
    init_game(game);
    ctx.game = &game;

    // Initialize rendering systems
    Render_Init(ctx);

    while (!platform.window->ShouldClose()) {
        float dt = GetFrameTime();

        // --- Update ---
        updateCamera(ctx.camera);
        update_game(game, dt);

        handle_input(game, platform);

        // Update render scale if toggles changed
        int winW = platform.window->GetWidth();
        int winH = platform.window->GetHeight();
        float targetScale = ctx.ui.supersample2x ? 2.0f : 1.0f;
        if (fabsf(targetScale - ctx.targets.scale) > 0.01f || winW != ctx.targets.width || winH != ctx.targets.height) {
            ctx.targets.scale = targetScale;
            Render_HandleResize(ctx, winW, winH);
        }

        // --- Draw ---
        Render_DrawFrame(ctx);

        UiActions uiActions = UI_Draw(ctx);

        handle_ui_actions(game, uiActions);

        platform.window->EndFrame();
    }

    // Cleanup
    Render_Cleanup(ctx);

    return 0;
}
