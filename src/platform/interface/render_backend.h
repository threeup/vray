#pragma once

#include "raylib.h"

/**
 * @brief Abstract interface for rendering backend.
 * 
 * This interface allows the renderer to be swapped out (e.g., Raylib vs Mock).
 * Enables headless testing and future render API abstraction.
 */
class RenderBackend {
public:
    virtual ~RenderBackend() = default;

    // 3D mode management
    virtual void BeginMode3D(const Camera3D& camera) = 0;
    virtual void EndMode3D() = 0;

    // Drawing primitives
    virtual void DrawModel(Model model, Vector3 position, float scale, Color tint) = 0;
    virtual void DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint) = 0;
    virtual void DrawCube(Vector3 position, float width, float height, float depth, Color color) = 0;
    virtual void DrawCubeWires(Vector3 position, float width, float height, float depth, Color color) = 0;
    virtual void DrawLine3D(Vector3 startPos, Vector3 endPos, Color color) = 0;
    virtual void DrawSphere(Vector3 centerPos, float radius, Color color) = 0;
    virtual void DrawGrid(int slices, float spacing) = 0;

    // Shader management
    virtual void BeginShaderMode(Shader shader) = 0;
    virtual void EndShaderMode() = 0;

    // Texture rendering
    virtual void BeginTextureMode(RenderTexture2D target) = 0;
    virtual void EndTextureMode() = 0;
    virtual void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint) = 0;
    virtual void ClearBackground(Color color) = 0;
};
