#pragma once

#include "raylib.h"
#include "app.h"
#include "mesh.h"

struct AppContext; // Forward declaration

// Load shaders and set initial uniforms (light and fxaa resolution)
void load_shaders(RenderShaders& shaders, int width, int height);
// Reload/resize-dependent shader values (e.g., FXAA resolution)
void update_shader_resolution(RenderShaders& shaders, int width, int height);

// Render 3D scene into the current render target (flat shader)
void render_scene(const AppContext& ctx, RendererInterface* renderer);

// Main rendering context functions
void Render_Init(AppContext& ctx);
void Render_Cleanup(AppContext& ctx);
void Render_DrawFrame(AppContext& ctx);
void Render_HandleResize(AppContext& ctx, int width, int height);
