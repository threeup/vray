#pragma once

#include "interface/render_backend.h"
#include "raylib.h"

/**
 * @brief Raylib implementation of RenderBackend.
 * 
 * This is the concrete implementation that delegates to raylib functions.
 * Used for normal gameplay rendering.
 */
class RaylibRenderBackend : public RenderBackend {
public:
    RaylibRenderBackend() = default;
    virtual ~RaylibRenderBackend() = default;

    // 3D mode management
    void BeginMode3D(const Camera3D& camera) override {
        ::BeginMode3D(camera);
    }

    void EndMode3D() override {
        ::EndMode3D();
    }

    // Drawing primitives
    void DrawModel(Model model, Vector3 position, float scale, Color tint) override {
        ::DrawModel(model, position, scale, tint);
    }

    void DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint) override {
        ::DrawModelEx(model, position, rotationAxis, rotationAngle, scale, tint);
    }

    void DrawCube(Vector3 position, float width, float height, float depth, Color color) override {
        ::DrawCube(position, width, height, depth, color);
    }

    void DrawCubeWires(Vector3 position, float width, float height, float depth, Color color) override {
        ::DrawCubeWires(position, width, height, depth, color);
    }

    void DrawLine3D(Vector3 startPos, Vector3 endPos, Color color) override {
        ::DrawLine3D(startPos, endPos, color);
    }

    void DrawSphere(Vector3 centerPos, float radius, Color color) override {
        ::DrawSphere(centerPos, radius, color);
    }

    void DrawGrid(int slices, float spacing) override {
        ::DrawGrid(slices, spacing);
    }

    // Shader management
    void BeginShaderMode(Shader shader) override {
        ::BeginShaderMode(shader);
    }

    void EndShaderMode() override {
        ::EndShaderMode();
    }

    // Texture rendering
    void BeginTextureMode(RenderTexture2D target) override {
        ::BeginTextureMode(target);
    }

    void EndTextureMode() override {
        ::EndTextureMode();
    }

    void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint) override {
        ::DrawTexturePro(texture, source, dest, origin, rotation, tint);
    }

    void ClearBackground(Color color) override {
        ::ClearBackground(color);
    }
};
