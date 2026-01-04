#pragma once

#include "app.h"      // AppContext, RenderTargets, RenderShaders, models, ui

struct World;

// Initialize GPU resources (targets/shaders) using window size from ctx.window
void Render_Init(AppContext& ctx);

// Recreate render targets for a resize or scale change
void Render_HandleResize(AppContext& ctx, int width, int height);

// Draw one full frame (3D scene + post + HUD)
void Render_DrawFrame(AppContext& ctx, World& world);

// Free GPU resources
void Render_Cleanup(AppContext& ctx);