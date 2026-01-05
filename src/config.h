#pragma once

#include <string>
#include <stdexcept>

struct lua_State;

/**
 * Application configuration loaded from Lua file.
 * Provides centralized settings for window, camera, and input parameters.
 * Falls back to defaults if file is missing or invalid.
 */
struct AppConfig {
    // Window settings
    int window_width;
    int window_height;
    int target_fps;
    bool fullscreen;

    // Camera settings
    float camera_pitch;      // degrees
    float camera_yaw;        // degrees
    float camera_roll;       // degrees
    float camera_fovy;       // field of view y
    float camera_distance;   // initial zoom distance

    // Input sensitivity
    float move_speed;
    float rotation_speed;
    float zoom_speed;
    float zoom_min;
    float zoom_max;

    // Constructor with defaults
    AppConfig();

    // Load from Lua file. If file doesn't exist or is invalid, uses defaults.
    static AppConfig LoadFromFile(const std::string& filePath);

    // Validate and clamp all values to safe ranges
    void Validate();
};
