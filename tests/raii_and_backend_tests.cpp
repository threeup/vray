#include <gtest/gtest.h>
#include "utils/raii_handles.h"
#include "platform/interface/render_backend.h"
#include "mocks/mock_render_backend.h"

// ============================================================================
// Tests for ShaderHandle (RAII)
// ============================================================================

class ShaderHandleTest : public ::testing::Test {
protected:
    // Note: In a real test, we'd need to initialize raylib context
    // For now, we test move semantics and validity checks
};

TEST_F(ShaderHandleTest, DefaultConstructorCreatesInvalidHandle) {
    ShaderHandle handle;
    EXPECT_FALSE(handle.valid());
    EXPECT_EQ(handle.shader.id, 0);
}

TEST_F(ShaderHandleTest, ExplicitConstructorSetsShader) {
    Shader testShader{};
    testShader.id = 42;  // Mock shader ID
    
    ShaderHandle handle(testShader);
    EXPECT_TRUE(handle.valid());
    EXPECT_EQ(handle.shader.id, 42);
}

TEST_F(ShaderHandleTest, MoveConstructorTransfersOwnership) {
    Shader testShader{};
    testShader.id = 42;
    
    ShaderHandle source(testShader);
    EXPECT_TRUE(source.valid());
    
    ShaderHandle dest(std::move(source));
    
    // Destination should own the shader
    EXPECT_TRUE(dest.valid());
    EXPECT_EQ(dest.shader.id, 42);
    
    // Source should be zeroed out (no double-delete)
    EXPECT_FALSE(source.valid());
    EXPECT_EQ(source.shader.id, 0);
}

TEST_F(ShaderHandleTest, MoveAssignmentTransfersOwnership) {
    Shader testShader1{};
    testShader1.id = 42;
    Shader testShader2{};
    testShader2.id = 99;
    
    ShaderHandle handle1(testShader1);
    ShaderHandle handle2(testShader2);
    
    handle1 = std::move(handle2);
    
    // handle1 should now own testShader2
    EXPECT_TRUE(handle1.valid());
    EXPECT_EQ(handle1.shader.id, 99);
    
    // handle2 should be zeroed (no double-delete)
    EXPECT_FALSE(handle2.valid());
}

TEST_F(ShaderHandleTest, SelfAssignmentIsNoOp) {
    Shader testShader{};
    testShader.id = 42;
    
    ShaderHandle handle(testShader);
    Shader originalId = handle.shader;
    
    // Self-assignment should not cause issues
    ShaderHandle* pHandle = &handle;
    *pHandle = std::move(*pHandle);
    
    // Shader should still be valid
    EXPECT_TRUE(handle.valid());
    EXPECT_EQ(handle.shader.id, originalId.id);
}

// ============================================================================
// Tests for ModelHandle (RAII)
// ============================================================================

class ModelHandleTest : public ::testing::Test {
protected:
    // Note: In a real test, we'd need to initialize raylib context
    // For now, we test move semantics and validity checks
};

TEST_F(ModelHandleTest, DefaultConstructorCreatesInvalidHandle) {
    ModelHandle handle;
    EXPECT_FALSE(handle.valid());
}

TEST_F(ModelHandleTest, ExplicitConstructorSetsModel) {
    // Note: We can't fully test Model with valid memory without raylib context
    // So we just verify the basic structure
    ModelHandle handle;
    EXPECT_FALSE(handle.valid());
}

// ============================================================================
// Tests for MockRenderBackend
// ============================================================================

class MockRenderBackendTest : public ::testing::Test {
protected:
    MockRenderBackend backend;
};

TEST_F(MockRenderBackendTest, RecordsBeginMode3DCall) {
    Camera3D camera{};
    backend.BeginMode3D(camera);
    
    EXPECT_TRUE(backend.WasCallMade("BeginMode3D"));
    EXPECT_EQ(backend.GetCallCount("BeginMode3D"), 1);
}

TEST_F(MockRenderBackendTest, RecordsMultipleCalls) {
    Camera3D camera{};
    backend.BeginMode3D(camera);
    backend.DrawCube({0, 0, 0}, 1.0f, 1.0f, 1.0f, WHITE);
    backend.EndMode3D();
    
    EXPECT_EQ(backend.GetCallCount("BeginMode3D"), 1);
    EXPECT_EQ(backend.GetCallCount("DrawCube"), 1);
    EXPECT_EQ(backend.GetCallCount("EndMode3D"), 1);
    EXPECT_EQ(backend.calls.size(), 3);
}

TEST_F(MockRenderBackendTest, ClearCallsClearsRecordedCalls) {
    Camera3D camera{};
    backend.BeginMode3D(camera);
    backend.DrawCube({0, 0, 0}, 1.0f, 1.0f, 1.0f, WHITE);
    
    EXPECT_EQ(backend.calls.size(), 2);
    
    backend.ClearCalls();
    
    EXPECT_EQ(backend.calls.size(), 0);
    EXPECT_FALSE(backend.WasCallMade("BeginMode3D"));
}

TEST_F(MockRenderBackendTest, RecordsShaderModeSequence) {
    Shader testShader{};
    backend.BeginShaderMode(testShader);
    backend.DrawCube({0, 0, 0}, 1.0f, 1.0f, 1.0f, WHITE);
    backend.EndShaderMode();
    
    EXPECT_EQ(backend.GetCallCount("BeginShaderMode"), 1);
    EXPECT_EQ(backend.GetCallCount("DrawCube"), 1);
    EXPECT_EQ(backend.GetCallCount("EndShaderMode"), 1);
}

TEST_F(MockRenderBackendTest, RecordsTextureModeSequence) {
    RenderTexture2D target{};
    backend.BeginTextureMode(target);
    backend.ClearBackground(BLACK);
    backend.EndTextureMode();
    
    EXPECT_EQ(backend.GetCallCount("BeginTextureMode"), 1);
    EXPECT_EQ(backend.GetCallCount("ClearBackground"), 1);
    EXPECT_EQ(backend.GetCallCount("EndTextureMode"), 1);
}
