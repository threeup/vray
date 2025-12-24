#pragma once

#include "interface/renderer_interface.h"

class RaylibRenderer : public RendererInterface {
public:
    RaylibRenderer() = default;
    ~RaylibRenderer() override = default;

    void Begin3D(const Camera3D& camera) override;
    void End3D() override;
    void DrawModel(const Model& model, Vector3 position, float scale, Color tint) override;
    void DrawCube(Vector3 position, float width, float height, float depth, Color color) override;
    void DrawLine3D(Vector3 startPos, Vector3 endPos, Color color) override;
    void SetLineWidth(float width) override;
    void DrawCubeWires(Vector3 position, float width, float height, float depth, Color color) override;
    void DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint) override;
};