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
#include "world/world.h"
#include <cmath>
#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <exception>

namespace {
    std::string TimestampUtc()
    {
        using clock = std::chrono::system_clock;
        const auto now = clock::now();
        const auto tt = clock::to_time_t(now);
        std::tm utc{};
#ifdef _WIN32
        gmtime_s(&utc, &tt);
#else
        gmtime_r(&tt, &utc);
#endif
        std::ostringstream oss;
        oss << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
        return oss.str();
    }

    void LogCrash(const std::string& message)
    {
        std::ofstream file("app_crashes.log", std::ios::app);
        if (!file.is_open()) return;
        file << "[" << TimestampUtc() << "] " << message << "\n";
    }

    void ShowFatalMessage(const std::string& message)
    {
        TraceLog(LOG_ERROR, "Fatal error: %s", message.c_str());
    }
}

int main() {
    std::string fatalMessage;
    bool fatal = false;

    try {
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

        // Build world using render shaders
        World world{};
        World_Init(world, ctx);

        try {
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
                platform.window->BeginFrame();
                Render_DrawFrame(ctx, world);

                UiActions uiActions = UI_Draw(ctx);

                handle_ui_actions(game, uiActions);

                platform.window->EndFrame();
            }
        } catch (const std::exception& ex) {
            fatal = true;
            fatalMessage = std::string("Unhandled exception in main loop: ") + ex.what();
        } catch (...) {
            fatal = true;
            fatalMessage = "Unknown error in main loop";
        }

        // Cleanup
        Render_Cleanup(ctx);
    } catch (const std::exception& ex) {
        fatal = true;
        fatalMessage = std::string("Startup failure: ") + ex.what();
    } catch (...) {
        fatal = true;
        fatalMessage = "Unknown startup failure";
    }

    if (fatal) {
        LogCrash(fatalMessage);
        ShowFatalMessage(fatalMessage);
        return 1;
    }

    return 0;
}
