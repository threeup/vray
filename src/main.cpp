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
#include "boss.h"
#include <cmath>
#include <algorithm>
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

    // Map grid coordinates from Game entities onto the 3D world coordinates for actors.
    Vector3 GridToWorldPos(const World& world, const Vector2& gridPos) {
        auto idx = [](int x, int y) { return y * World::kTilesWide + x; };
        int gx = static_cast<int>(std::round(gridPos.x));
        int gy = static_cast<int>(std::round(gridPos.y));
        gx = std::clamp(gx, 0, World::kTilesWide - 1);
        gy = std::clamp(gy, 0, World::kTilesHigh - 1);

        auto tileHeight = [&](TileType t) {
            switch (t) {
            case TileType::Water: return -0.05f;
            case TileType::Mountain: return 0.6f;
            case TileType::Skyscraper: return 0.12f;
            default: return 0.0f;
            }
        };

        float baseY = tileHeight(world.tiles[idx(gx, gy)]) + 0.60f; // lift actor above slab
        float wx = (gx - World::kTilesWide * 0.5f + 0.5f) * World::kTileSize;
        float wz = (gy - World::kTilesHigh * 0.5f + 0.5f) * World::kTileSize;
        return {wx, baseY, wz};
    }

    void SyncWorldActorsFromGame(const Game& game, World& world) {
        // Collect actors in render world
        std::vector<WorldEntity*> worldPlayers;
        std::vector<WorldEntity*> worldEnemies;
        for (auto& we : world.entities) {
            if (!we.isActor) continue;
            if (we.isEnemy) {
                worldEnemies.push_back(&we);
            } else {
                worldPlayers.push_back(&we);
            }
        }

        // Collect actors in game state
        std::vector<const Entity*> gamePlayers;
        std::vector<const Entity*> gameEnemies;
        for (const auto& e : game.entities) {
            if (e.type == PLAYER) {
                gamePlayers.push_back(&e);
            } else if (e.type == ENEMY) {
                gameEnemies.push_back(&e);
            }
        }

        auto applyPositions = [&](const std::vector<const Entity*>& src, std::vector<WorldEntity*>& dst) {
            size_t n = std::min(src.size(), dst.size());
            for (size_t i = 0; i < n; ++i) {
                Vector3 wpos = GridToWorldPos(world, src[i]->position);
                dst[i]->targetPos = wpos;
                dst[i]->startPos = wpos;
                dst[i]->position = wpos;
            }
        };

        applyPositions(gamePlayers, worldPlayers);
        applyPositions(gameEnemies, worldEnemies);
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

        Boss boss;
        boss.begin(game);
        ctx.boss = &boss;

        // Initialize rendering systems
        Render_Init(ctx);

        // Build world using render shaders
        World world{};
        World_Init(world, ctx);
        
        float totalElapsedTime = 0.0f;

        try {
            while (!platform.window->ShouldClose()) {
                float dt = GetFrameTime();
                totalElapsedTime += dt;

                // --- Update ---
                updateCamera(ctx.camera);
                update_game(game, dt);
                World_Update(world, totalElapsedTime);

                boss.update(game, dt);

                handle_input(game, platform);
                SyncWorldActorsFromGame(game, world);

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
                boss.processUi(game, uiActions);

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
