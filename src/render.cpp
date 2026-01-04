#include "render.h"
#include "raylib.h"
#include "raymath.h"
#include <vector>
#include "platform/window_interface.h"
#include "platform/renderer_interface.h"
#include "mesh.h"
#include "world/world.h"

// Initialize render targets and shaders using window size from AppContext
void Render_Init(AppContext& ctx) {
    int windowWidth = ctx.window->GetWidth();
    int windowHeight = ctx.window->GetHeight();
    // 1. Initialize Render Targets
    ctx.targets.width = windowWidth;
    ctx.targets.height = windowHeight;
    // ctx.targets.scale already set by UI toggle (default 1.0f)

    int rtWidth = (int)(ctx.targets.width * ctx.targets.scale);
    int rtHeight = (int)(ctx.targets.height * ctx.targets.scale);

    ctx.targets.scene = LoadRenderTexture(rtWidth, rtHeight);
    ctx.targets.post = LoadRenderTexture(rtWidth, rtHeight);

    // 2. Load Shaders
    // Lighting
    ctx.shaders.lighting = LoadShader("assets/lighting.vs", "assets/lighting.fs");
    
    // Flat / Faceted
    ctx.shaders.flat = LoadShader("assets/xflat.vs", "assets/xflat.fs");
    ctx.shaders.flatLightPosLoc = GetShaderLocation(ctx.shaders.flat, "lightPos");
    ctx.shaders.flatViewPosLoc = GetShaderLocation(ctx.shaders.flat, "viewPos");

    // Post-Processing
    ctx.shaders.bloom = LoadShader(0, "assets/bloom.fs");
    ctx.shaders.pastel = LoadShader(0, "assets/pastel.fs");
}


void Render_Cleanup(AppContext& ctx) {
    // Unload render textures
    if (ctx.targets.scene.id != 0) {
        UnloadRenderTexture(ctx.targets.scene);
    }
    if (ctx.targets.post.id != 0) {
        UnloadRenderTexture(ctx.targets.post);
    }
    
    // Unload shaders
    if (ctx.shaders.lighting.id != 0) {
        UnloadShader(ctx.shaders.lighting);
    }
    if (ctx.shaders.flat.id != 0) {
        UnloadShader(ctx.shaders.flat);
    }
    if (ctx.shaders.bloom.id != 0) {
        UnloadShader(ctx.shaders.bloom);
    }
    if (ctx.shaders.pastel.id != 0) {
        UnloadShader(ctx.shaders.pastel);
    }
    // Unload sphere
    if (ctx.models.sphere.meshCount > 0) {
        UnloadModel(ctx.models.sphere);
    }
    
    // Unload pyramid
    if (ctx.models.pyramid.meshCount > 0) {
        UnloadModel(ctx.models.pyramid);
    }

    // Unload tree
    if (ctx.models.tree.meshCount > 0) {
        UnloadModel(ctx.models.tree);
    }
}

void Render_HandleResize(AppContext& ctx, int width, int height) {
    ctx.targets.width = width;
    ctx.targets.height = height;
    
    int rtWidth = (int)(width * ctx.targets.scale);
    int rtHeight = (int)(height * ctx.targets.scale);
    
    // Recreate render textures at new size
    if (ctx.targets.scene.id != 0) {
        UnloadRenderTexture(ctx.targets.scene);
    }
    if (ctx.targets.post.id != 0) {
        UnloadRenderTexture(ctx.targets.post);
    }
    
    ctx.targets.scene = LoadRenderTexture(rtWidth, rtHeight);
    ctx.targets.post = LoadRenderTexture(rtWidth, rtHeight);
}

// Sets up shader uniforms like Light and Camera position
static void ApplyGlobalUniforms(AppContext& ctx, const World& world) {
    float camPos[3] = { ctx.camera.position.x, ctx.camera.position.y, ctx.camera.position.z };
    SetShaderValue(ctx.shaders.flat, ctx.shaders.flatViewPosLoc, camPos, SHADER_UNIFORM_VEC3);

    if (world.lightCount > 0) {
        const Light& mainLight = world.lights[world.activeLight];
        UpdateLightValues(ctx.shaders.flat, mainLight);
    }
}

// Draws a texture to a target (or screen) using a specific shader
static void ApplyEffect(Shader shader, RenderTexture2D source, RenderTexture2D* destination, int width, int height) {
    if (destination) BeginTextureMode(*destination);
    
    BeginShaderMode(shader);
        DrawTexturePro(source.texture,
            Rectangle{0, 0, (float)source.texture.width, -(float)source.texture.height},
            Rectangle{0, 0, (float)width, (float)height},
            Vector2{0, 0}, 0.0f, WHITE);
    EndShaderMode();

    if (destination) EndTextureMode();
}

// Simple copy pass without a shader (default pipeline)
static void ApplyCopy(RenderTexture2D source, RenderTexture2D* destination, int width, int height) {
    if (destination) BeginTextureMode(*destination);

    DrawTexturePro(source.texture,
        Rectangle{0, 0, (float)source.texture.width, -(float)source.texture.height},
        Rectangle{0, 0, (float)width, (float)height},
        Vector2{0, 0}, 0.0f, WHITE);

    if (destination) EndTextureMode();
}

static void Render_DrawScene(AppContext& app, const World& world) {
    RenderTargets& targets = app.targets;
    RenderShaders& shaders = app.shaders;
    BeginTextureMode(targets.scene);
        ClearBackground(RAYWHITE);
        BeginShaderMode(shaders.flat);
        BeginMode3D(app.camera);
            // 1. Draw entities (Models use the flat shader assigned in World_Init)
            if (app.ui.showEntities) {
                for (const auto& entity : world.entities) {
                    DrawModel(entity.model, entity.position, entity.scale.x, entity.color);
                }
            }

            // 2. Draw the environment
            if (app.ui.showEnvironment) {
                World_DrawGround(world);
            }

            // 3. Draw Light Indicator (The Toggle)
            if (app.ui.showLightIndicator && world.lightCount > 0) {
                Vector3 lightPos = world.lights[world.activeLight].position;
                Color lightCol = world.lights[world.activeLight].color;
                
                // Draw sphere without the flat shader so it glows in the bloom pass.
                EndShaderMode();
                DrawSphere(lightPos, 0.25f, lightCol);
                BeginShaderMode(shaders.flat);
            }
        EndMode3D();
        EndShaderMode();
    EndTextureMode();
}

void Render_DrawFrame(AppContext& ctx, World& world) {
    // --- STEP 1: PREPARE SHADERS ---
    ApplyGlobalUniforms(ctx, world);

    // --- STEP 2: SCENE PASS (3D Geometry) ---
    Render_DrawScene(ctx, world);

    // --- STEP 3: POST-PROCESS PASS (2D Effects) ---
    
    // Pass A: Bloom (Scene -> Post)
    if (ctx.ui.bloomEnabled) {
        SetShaderValue(ctx.shaders.bloom, GetShaderLocation(ctx.shaders.bloom, "intensity"), &ctx.ui.bloomIntensity, SHADER_UNIFORM_FLOAT);
        ApplyEffect(ctx.shaders.bloom, ctx.targets.scene, &ctx.targets.post, ctx.targets.width, ctx.targets.height);
    } else {
        // Simple copy if effect is off
        ApplyCopy(ctx.targets.scene, &ctx.targets.post, ctx.targets.width, ctx.targets.height);
    }

    // --- STEP 4: FINAL OUTPUT (Post -> Screen) ---
    // Caller must have begun the frame (BeginFrame/BeginDrawing)
    ClearBackground(BLACK);
    
    if (ctx.ui.pastelEnabled) {
        SetShaderValue(ctx.shaders.pastel, GetShaderLocation(ctx.shaders.pastel, "intensity"), &ctx.ui.pastelIntensity, SHADER_UNIFORM_FLOAT);
        ApplyEffect(ctx.shaders.pastel, ctx.targets.post, nullptr, ctx.window->GetWidth(), ctx.window->GetHeight());
    } else {
        ApplyCopy(ctx.targets.post, nullptr, ctx.window->GetWidth(), ctx.window->GetHeight());
    }

    // Draw UI/HUD here (UI_Draw will overlay afterward)
    DrawFPS(10, 10);
}