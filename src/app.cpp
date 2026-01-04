#include "app.h"

static void unload_targets(RenderTargets& rt) {
    if (rt.scene.id != 0) {
        UnloadRenderTexture(rt.scene);
        rt.scene = {0};
    }
    if (rt.post.id != 0) {
        UnloadRenderTexture(rt.post);
        rt.post = {0};
    }
}

void init_app(AppContext& ctx, int width, int height) {
    ctx.targets.width = width;
    ctx.targets.height = height;
    ctx.targets.scale = 1.0f;
    ctx.ui.supersample2x = false;
    ctx.ui.fxaa = true;
    ctx.ui.showEntities = true;
    ctx.ui.showEnvironment = true;
    ctx.ui.renderControlsCollapsed = true;
}

void rebuild_render_targets(AppContext& ctx, float scale, int width, int height) {
    ctx.targets.scale = scale;
    ctx.targets.width = width;
    ctx.targets.height = height;

    int rtWidth = static_cast<int>(width * scale);
    int rtHeight = static_cast<int>(height * scale);

    unload_targets(ctx.targets);
    ctx.targets.scene = LoadRenderTexture(rtWidth, rtHeight);
    ctx.targets.post = LoadRenderTexture(rtWidth, rtHeight);
}

void shutdown_app(AppContext& ctx) {
    unload_targets(ctx.targets);
}

