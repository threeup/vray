#pragma once

#include "raylib.h" // For types like Model, Color, Vector3

/**
 * @brief An interface for a 3D renderer.
 *
 * This abstracts the underlying rendering API (like raylib's 3D mode,
 * OpenGL, Vulkan, etc.) so that game logic can issue high-level
 * drawing commands.
 */
class RendererInterface {
public:
    virtual ~RendererInterface() = default;

    virtual void Begin3D(const Camera3D& camera) = 0;
    virtual void End3D() = 0;

    virtual void DrawModel(const Model& model, Vector3 position, float scale, Color tint) = 0;
    virtual void DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis,
                             float rotationAngle, Vector3 scale, Color tint) = 0;

    virtual void DrawCube(Vector3 position, float width, float height, float depth, Color color) = 0;
    virtual void DrawCubeWires(Vector3 position, float width, float height, float depth, Color color) = 0;

    virtual void DrawLine3D(Vector3 startPos, Vector3 endPos, Color color) = 0;
    virtual void SetLineWidth(float width) = 0;
};