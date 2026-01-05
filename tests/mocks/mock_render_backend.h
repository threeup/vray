#pragma once

#include "platform/interface/render_backend.h"
#include "raylib.h"
#include <vector>
#include <string>

/**
 * @brief Mock implementation of RenderBackend for testing.
 * 
 * Records all calls without actually rendering. Useful for:
 * - Headless testing (no GPU required)
 * - Verifying render call sequences
 * - Performance analysis without GPU overhead
 */
class MockRenderBackend : public RenderBackend {
public:
    // Call tracking
    struct Call {
        std::string name;
        // Extended with data as needed for specific tests
    };

    std::vector<Call> calls;

    MockRenderBackend() = default;
    virtual ~MockRenderBackend() = default;

    // Clear recorded calls
    void ClearCalls() {
        calls.clear();
    }

    // Get count of specific call type
    size_t GetCallCount(const std::string& callName) const {
        size_t count = 0;
        for (const auto& call : calls) {
            if (call.name == callName) {
                count++;
            }
        }
        return count;
    }

    // Check if a call was made
    bool WasCallMade(const std::string& callName) const {
        return GetCallCount(callName) > 0;
    }

    // 3D mode management
    void BeginMode3D(const Camera3D& camera) override {
        calls.push_back({"BeginMode3D"});
    }

    void EndMode3D() override {
        calls.push_back({"EndMode3D"});
    }

    // Drawing primitives
    void DrawModel(Model model, Vector3 position, float scale, Color tint) override {
        calls.push_back({"DrawModel"});
    }

    void DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint) override {
        calls.push_back({"DrawModelEx"});
    }

    void DrawCube(Vector3 position, float width, float height, float depth, Color color) override {
        calls.push_back({"DrawCube"});
    }

    void DrawCubeWires(Vector3 position, float width, float height, float depth, Color color) override {
        calls.push_back({"DrawCubeWires"});
    }

    void DrawLine3D(Vector3 startPos, Vector3 endPos, Color color) override {
        calls.push_back({"DrawLine3D"});
    }

    void DrawSphere(Vector3 centerPos, float radius, Color color) override {
        calls.push_back({"DrawSphere"});
    }

    void DrawGrid(int slices, float spacing) override {
        calls.push_back({"DrawGrid"});
    }

    // Shader management
    void BeginShaderMode(Shader shader) override {
        calls.push_back({"BeginShaderMode"});
    }

    void EndShaderMode() override {
        calls.push_back({"EndShaderMode"});
    }

    // Texture rendering
    void BeginTextureMode(RenderTexture2D target) override {
        calls.push_back({"BeginTextureMode"});
    }

    void EndTextureMode() override {
        calls.push_back({"EndTextureMode"});
    }

    void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint) override {
        calls.push_back({"DrawTexturePro"});
    }

    void ClearBackground(Color color) override {
        calls.push_back({"ClearBackground"});
    }
};
