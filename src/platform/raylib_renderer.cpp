#include "raylib_renderer.h"
#include "rlgl.h" // For rlSetLineWidth

void RaylibRenderer::Begin3D(const Camera3D& camera) {
    ::BeginMode3D(camera);
}

void RaylibRenderer::End3D() {
    ::EndMode3D();
}

void RaylibRenderer::DrawModel(const Model& model, Vector3 position, float scale, Color tint) {
    ::DrawModel(model, position, scale, tint);
}

void RaylibRenderer::DrawCube(Vector3 position, float width, float height, float depth, Color color) {
    ::DrawCube(position, width, height, depth, color);
}

void RaylibRenderer::DrawLine3D(Vector3 startPos, Vector3 endPos, Color color) {
    ::DrawLine3D(startPos, endPos, color);
}

void RaylibRenderer::SetLineWidth(float width) {
    ::rlSetLineWidth(width);
}

void RaylibRenderer::DrawCubeWires(Vector3 position, float width, float height, float depth, Color color) {
    ::DrawCubeWires(position, width, height, depth, color);
}

void RaylibRenderer::DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint) {
    ::DrawModelEx(model, position, rotationAxis, rotationAngle, scale, tint);
}