#include "render.h"
#include "raylib.h"
#include "raymath.h"
#include <vector>
#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"
#include "platform/window_interface.h"
#include "platform/renderer_interface.h"
#include "mesh.h"

// Module-local lighting state
static Light g_lights[MAX_LIGHTS] = {};
static int g_activeLight = 0;
static int g_lightCount = 0;

namespace {
    enum class InstanceKind { Tree, Pyramid };
    struct Instance {
        Vector3 pos;
        InstanceKind kind;
    };

    std::vector<Instance> g_instances;

    void BuildInstances()
    {
        g_instances.clear();

        const int cols = 5;
        const int rows = 4;
        const float step = 2.5f; // >= 2 units separation
        const float halfW = (cols - 1) * 0.5f * step;
        const float halfH = (rows - 1) * 0.5f * step;

        int count = 0;
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                if (count >= 20) break;
                float x = -halfW + c * step;
                float z = -halfH + r * step;
                InstanceKind kind = (count < 10) ? InstanceKind::Tree : InstanceKind::Pyramid;
                g_instances.push_back({ Vector3{ x, 0.0f, z }, kind });
                ++count;
            }
        }
    }
}

// Post-processing shaders
static Shader g_bloomShader = {0};
static Shader g_pastelShader = {0};
void Render_Init(AppContext& ctx) {
    // Initialize render targets for post-processing
    ctx.targets.width = ctx.window->GetWidth();
    ctx.targets.height = ctx.window->GetHeight();
    ctx.targets.scale = 1.0f;
    
    int rtWidth = (int)(ctx.targets.width * ctx.targets.scale);
    int rtHeight = (int)(ctx.targets.height * ctx.targets.scale);
    
    ctx.targets.scene = LoadRenderTexture(rtWidth, rtHeight);
    ctx.targets.post = LoadRenderTexture(rtWidth, rtHeight);
    
    // Load lighting shader
    ctx.shaders.lighting = LoadShader("assets/lighting.vs", "assets/lighting.fs");
    ctx.shaders.viewPosLoc = GetShaderLocation(ctx.shaders.lighting, "viewPos");
    
    // Load flat shader for crisp faceted look (using both vertex and fragment shaders)
    ctx.shaders.flat = LoadShader("assets/xflat.vs", "assets/xflat.fs");
    ctx.shaders.flatLightPosLoc = GetShaderLocation(ctx.shaders.flat, "lightPos");
    ctx.shaders.flatViewPosLoc = GetShaderLocation(ctx.shaders.flat, "viewPos");
    
    // Load post-processing shaders
    g_bloomShader = LoadShader(0, "assets/bloom.fs");
    g_pastelShader = LoadShader(0, "assets/pastel.fs");
    
    // Load a low-poly sphere mesh with a touch of noise for faceted look
    //Mesh sphereMesh = MeshGenerator::createLowPolySphereMesh(2.0f, 16, 16, 0.25f);
    Mesh sphereMesh = MeshGenerator::createCustomOctahedron(1.5f, 1);
    MeshUtils::unshareMeshVertices(&sphereMesh);  // Unshare vertices for flat/faceted shading
    UploadMesh(&sphereMesh, false);
    ctx.models.sphere = LoadModelFromMesh(sphereMesh);
    ctx.models.sphere.materials[0].shader = ctx.shaders.flat;

    Mesh sideSphereMesh = MeshGenerator::createCustomIcosphere(1.5f, 1);
    MeshUtils::unshareMeshVertices(&sideSphereMesh);  // Unshare vertices for flat/faceted shading
    UploadMesh(&sideSphereMesh, false);
    ctx.models.sideSphere = LoadModelFromMesh(sideSphereMesh);
    ctx.models.sideSphere.materials[0].shader = ctx.shaders.flat;
    
    // Create hexagonal pyramid mesh (cone with 6 sides)
    Mesh pyramidMesh = GenMeshCone(1.5f, 3.0f, 6);
    MeshUtils::unshareMeshVertices(&pyramidMesh);  // Unshare vertices for flat/faceted shading
    ctx.models.pyramid = LoadModelFromMesh(pyramidMesh);
    ctx.models.pyramid.materials[0].shader = ctx.shaders.flat;
    
    
    Mesh treeMesh = MeshGenerator::createSquareTree(1.5f, 1, 1);
    MeshUtils::unshareMeshVertices(&treeMesh);  // Unshare vertices for flat/faceted shading
    UploadMesh(&treeMesh, false);
    ctx.models.tree = LoadModelFromMesh(treeMesh);
    ctx.models.tree.materials[0].shader = ctx.shaders.flat;

    BuildInstances();

    // Initialize lights - adjusted position and color for stylized icon appearance
    g_lights[0] = CreateLight(LIGHT_POINT, Vector3{3, 5, 2}, Vector3Zero(), Color{255, 240, 200, 255}, ctx.shaders.flat);
    g_lightCount = 1;
    g_activeLight = 0;
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
    if (g_bloomShader.id != 0) {
        UnloadShader(g_bloomShader);
    }
    if (g_pastelShader.id != 0) {
        UnloadShader(g_pastelShader);
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

void render_scene(const AppContext& ctx, RendererInterface* renderer) {
    // Begin 3D mode with camera
    renderer->Begin3D(ctx.camera);
    
    // Update flat shader with light and camera positions
    if (ctx.shaders.flatLightPosLoc != -1) {
        Vector3 lightPos = g_lights[g_activeLight].position;
        SetShaderValue(ctx.shaders.flat, ctx.shaders.flatLightPosLoc, &lightPos, SHADER_UNIFORM_VEC3);
    }
    if (ctx.shaders.flatViewPosLoc != -1) {
        SetShaderValue(ctx.shaders.flat, ctx.shaders.flatViewPosLoc, &ctx.camera.position, SHADER_UNIFORM_VEC3);
    }
    
    // Draw grid/floor at y=0
    DrawGrid(10, 1.0f);
    
    // Draw auto-distributed instances
    for (const auto& inst : g_instances) {
        if (inst.kind == InstanceKind::Tree) {
            DrawModel(ctx.models.tree, inst.pos, 1.0f, RED);
        } else {
            DrawModel(ctx.models.pyramid, inst.pos, 1.0f, GREEN);
        }
    }
    
    // Draw light indicator
    DrawSphere(g_lights[g_activeLight].position, 0.2f, g_lights[g_activeLight].color);
    
    // End 3D mode
    renderer->End3D();
}

void Render_DrawFrame(AppContext& ctx) {
    // Render 3D scene to sceneTarget
    BeginTextureMode(ctx.targets.scene);
    ClearBackground(RAYWHITE);
    render_scene(ctx, ctx.renderer.get());
    EndTextureMode();
    
    // Apply bloom post-processing if enabled
    if (ctx.ui.bloomEnabled) {
        BeginTextureMode(ctx.targets.post);
        ClearBackground(BLACK);
        BeginShaderMode(g_bloomShader);
        
        // Set bloom intensity uniform if available
        int bloomIntensityLoc = GetShaderLocation(g_bloomShader, "intensity");
        if (bloomIntensityLoc != -1) {
            SetShaderValue(g_bloomShader, bloomIntensityLoc, &ctx.ui.bloomIntensity, SHADER_UNIFORM_FLOAT);
        }
        
        DrawTexturePro(
            ctx.targets.scene.texture,
            Rectangle{0, 0, (float)ctx.targets.scene.texture.width, -(float)ctx.targets.scene.texture.height},
            Rectangle{0, 0, (float)ctx.targets.width, (float)ctx.targets.height},
            Vector2{0, 0},
            0.0f,
            WHITE
        );
        EndShaderMode();
        EndTextureMode();
    } else {
        // If bloom is disabled, just copy the scene to post target
        BeginTextureMode(ctx.targets.post);
        ClearBackground(BLACK);
        DrawTexturePro(
            ctx.targets.scene.texture,
            Rectangle{0, 0, (float)ctx.targets.scene.texture.width, -(float)ctx.targets.scene.texture.height},
            Rectangle{0, 0, (float)ctx.targets.width, (float)ctx.targets.height},
            Vector2{0, 0},
            0.0f,
            WHITE
        );
        EndTextureMode();
    }
    
    // Apply pastel post-processing on top of bloom if enabled
    int winW = ctx.window->GetWidth();
    int winH = ctx.window->GetHeight();
    
    ctx.window->BeginFrame();
    ClearBackground(BLACK);
    
    if (ctx.ui.pastelEnabled) {
        BeginShaderMode(g_pastelShader);
        
        // Set pastel intensity uniform if available
        int pastelIntensityLoc = GetShaderLocation(g_pastelShader, "intensity");
        if (pastelIntensityLoc != -1) {
            SetShaderValue(g_pastelShader, pastelIntensityLoc, &ctx.ui.pastelIntensity, SHADER_UNIFORM_FLOAT);
        }
        
        DrawTexturePro(
            ctx.targets.post.texture,
            Rectangle{0, 0, (float)ctx.targets.post.texture.width, -(float)ctx.targets.post.texture.height},
            Rectangle{0, 0, (float)winW, (float)winH},
            Vector2{0, 0},
            0.0f,
            WHITE
        );
        EndShaderMode();
    } else {
        // If pastel is disabled, just draw the post target directly
        DrawTexturePro(
            ctx.targets.post.texture,
            Rectangle{0, 0, (float)ctx.targets.post.texture.width, -(float)ctx.targets.post.texture.height},
            Rectangle{0, 0, (float)winW, (float)winH},
            Vector2{0, 0},
            0.0f,
            WHITE
        );
    }
}
