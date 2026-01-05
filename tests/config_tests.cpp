#include <gtest/gtest.h>
#include "config.h"
#include <fstream>
#include <filesystem>
#include <cstdio>

namespace fs = std::filesystem;

class ConfigTest : public ::testing::Test {
protected:
    const std::string testConfigPath = "test_vars.lua";

    void TearDown() override {
        // Clean up test config file
        if (fs::exists(testConfigPath)) {
            fs::remove(testConfigPath);
        }
    }

    void WriteLuaConfigFile(const std::string& content) {
        std::ofstream file(testConfigPath);
        file << content;
        file.close();
    }
};

TEST_F(ConfigTest, DefaultConstructor) {
    AppConfig config;
    
    EXPECT_EQ(config.window_width, 800);
    EXPECT_EQ(config.window_height, 600);
    EXPECT_EQ(config.target_fps, 60);
    EXPECT_FALSE(config.fullscreen);
    EXPECT_EQ(config.camera_pitch, 35.0f);
    EXPECT_EQ(config.camera_yaw, 23.0f);
    EXPECT_EQ(config.camera_roll, 0.0f);
    EXPECT_EQ(config.camera_fovy, 45.0f);
    EXPECT_EQ(config.camera_distance, 22.0f);
    EXPECT_EQ(config.move_speed, 15.0f);
    EXPECT_EQ(config.rotation_speed, 2.5f);
    EXPECT_EQ(config.zoom_speed, 3.0f);
    EXPECT_EQ(config.zoom_min, 5.0f);
    EXPECT_EQ(config.zoom_max, 80.0f);
}

TEST_F(ConfigTest, LoadFromMissingFile) {
    // Should return defaults if file doesn't exist
    AppConfig config = AppConfig::LoadFromFile("nonexistent_file.json");
    
    EXPECT_EQ(config.window_width, 800);
    EXPECT_EQ(config.window_height, 600);
    EXPECT_EQ(config.target_fps, 60);
}

TEST_F(ConfigTest, LoadValidLuaConfig) {
    std::string lua = R"(
window = {
    width = 1024,
    height = 768,
    fps = 120,
    fullscreen = true
}

camera = {
    pitch = 40.0,
    yaw = 45.0,
    roll = 0.0,
    fovy = 50.0,
    distance = 25.0
}

input = {
    move_speed = 20.0,
    rotation_speed = 3.0,
    zoom_speed = 4.0,
    zoom_min = 3.0,
    zoom_max = 100.0
}
)";
    
    WriteLuaConfigFile(lua);
    AppConfig config = AppConfig::LoadFromFile(testConfigPath);
    
    EXPECT_EQ(config.window_width, 1024);
    EXPECT_EQ(config.window_height, 768);
    EXPECT_EQ(config.target_fps, 120);
    EXPECT_TRUE(config.fullscreen);
    EXPECT_EQ(config.camera_pitch, 40.0f);
    EXPECT_EQ(config.camera_yaw, 45.0f);
    EXPECT_EQ(config.camera_fovy, 50.0f);
    EXPECT_EQ(config.camera_distance, 25.0f);
    EXPECT_EQ(config.move_speed, 20.0f);
}

TEST_F(ConfigTest, InvalidLuaSyntaxReturnsDefaults) {
    WriteLuaConfigFile("window = { width = 1024 broken syntax");
    AppConfig config = AppConfig::LoadFromFile(testConfigPath);
    
    // Should return defaults on parse error
    EXPECT_EQ(config.window_width, 800);
    EXPECT_EQ(config.target_fps, 60);
}

TEST_F(ConfigTest, PartialLuaConfigUsesFallbacks) {
    std::string lua = R"(
window = {
    width = 1280,
    fps = 75
}
)";
    
    WriteLuaConfigFile(lua);
    AppConfig config = AppConfig::LoadFromFile(testConfigPath);
    
    // Loaded values
    EXPECT_EQ(config.window_width, 1280);
    EXPECT_EQ(config.target_fps, 75);
    
    // Fallback to defaults
    EXPECT_EQ(config.window_height, 600);
    EXPECT_EQ(config.camera_fovy, 45.0f);
}

TEST_F(ConfigTest, ValidateClampWindowSize) {
    AppConfig config;
    config.window_width = 100;   // Too small
    config.window_height = 10000; // Too large
    
    config.Validate();
    
    EXPECT_GE(config.window_width, 320);
    EXPECT_LE(config.window_width, 4096);
    EXPECT_GE(config.window_height, 240);
    EXPECT_LE(config.window_height, 4096);
}

TEST_F(ConfigTest, ValidateClampFPS) {
    AppConfig config;
    config.target_fps = 5;   // Too low
    config.Validate();
    EXPECT_GE(config.target_fps, 15);
    
    config.target_fps = 500;  // Too high
    config.Validate();
    EXPECT_LE(config.target_fps, 240);
}

TEST_F(ConfigTest, ValidateClampFOV) {
    AppConfig config;
    config.camera_fovy = 1.0f;  // Too low
    config.Validate();
    EXPECT_GE(config.camera_fovy, 5.0f);
    
    config.camera_fovy = 200.0f;  // Too high
    config.Validate();
    EXPECT_LE(config.camera_fovy, 120.0f);
}

TEST_F(ConfigTest, ValidateClampCameraDistance) {
    AppConfig config;
    config.camera_distance = 0.1f;  // Too low
    config.Validate();
    EXPECT_GE(config.camera_distance, 1.0f);
    
    config.camera_distance = 500.0f;  // Too high
    config.Validate();
    EXPECT_LE(config.camera_distance, 200.0f);
}

TEST_F(ConfigTest, ValidateClampSensitivity) {
    AppConfig config;
    config.move_speed = 0.01f;  // Too low
    config.rotation_speed = 200.0f;  // Too high
    config.zoom_speed = -5.0f;  // Negative
    
    config.Validate();
    
    EXPECT_GE(config.move_speed, 0.1f);
    EXPECT_LE(config.rotation_speed, 50.0f);
    EXPECT_GE(config.zoom_speed, 0.1f);
}

TEST_F(ConfigTest, ValidateZoomMinMax) {
    AppConfig config;
    config.zoom_min = 100.0f;
    config.zoom_max = 50.0f;  // Swapped
    
    config.Validate();
    
    EXPECT_LT(config.zoom_min, config.zoom_max);
    EXPECT_EQ(config.zoom_min, 5.0f);   // Reset to defaults
    EXPECT_EQ(config.zoom_max, 80.0f);
}

TEST_F(ConfigTest, EmptyLuaFileReturnsDefaults) {
    WriteLuaConfigFile("");
    AppConfig config = AppConfig::LoadFromFile(testConfigPath);
    
    EXPECT_EQ(config.window_width, 800);
    EXPECT_EQ(config.window_height, 600);
    EXPECT_EQ(config.target_fps, 60);
}

TEST_F(ConfigTest, BackwardCompatibilityWithMissingTables) {
    // Config file with only some tables defined
    std::string lua = R"(
window = {
    width = 1024
}
)";
    
    WriteLuaConfigFile(lua);
    AppConfig config = AppConfig::LoadFromFile(testConfigPath);
    
    // Should have loaded value and defaults for others
    EXPECT_EQ(config.window_width, 1024);
    EXPECT_EQ(config.window_height, 600);  // Default
    EXPECT_EQ(config.target_fps, 60);      // Default
    EXPECT_EQ(config.camera_fovy, 45.0f);  // Default
}

TEST_F(ConfigTest, LuaCommentsAreIgnored) {
    std::string lua = R"(
-- This is a comment
window = {
    width = 1152,  -- Custom width
    height = 864,
    fps = 100
}
)";
    
    WriteLuaConfigFile(lua);
    AppConfig config = AppConfig::LoadFromFile(testConfigPath);
    
    EXPECT_EQ(config.window_width, 1152);
    EXPECT_EQ(config.window_height, 864);
    EXPECT_EQ(config.target_fps, 100);
}

TEST_F(ConfigTest, LuaExpressions) {
    std::string lua = R"(
local base_size = 600
window = {
    width = 800,
    height = base_size,
    fps = 30 * 2,
    fullscreen = false
}
)";
    
    WriteLuaConfigFile(lua);
    AppConfig config = AppConfig::LoadFromFile(testConfigPath);
    
    EXPECT_EQ(config.window_width, 800);
    EXPECT_EQ(config.window_height, 600);
    EXPECT_EQ(config.target_fps, 60);  // 30 * 2
}

TEST_F(ConfigTest, DefaultsMatchHardcodedBehavior) {
    // Verify defaults match the original hardcoded values
    AppConfig config;
    
    // Window
    EXPECT_EQ(config.window_width, 800);
    EXPECT_EQ(config.window_height, 600);
    EXPECT_EQ(config.target_fps, 60);
    
    // Camera
    EXPECT_EQ(config.camera_pitch, 35.0f);
    EXPECT_EQ(config.camera_yaw, 23.0f);
    EXPECT_EQ(config.camera_roll, 0.0f);
    EXPECT_EQ(config.camera_fovy, 45.0f);
    EXPECT_EQ(config.camera_distance, 22.0f);
    
    // Input sensitivity
    EXPECT_EQ(config.move_speed, 15.0f);
    EXPECT_EQ(config.rotation_speed, 2.5f);
    EXPECT_EQ(config.zoom_speed, 3.0f);
    EXPECT_EQ(config.zoom_min, 5.0f);
    EXPECT_EQ(config.zoom_max, 80.0f);
}
