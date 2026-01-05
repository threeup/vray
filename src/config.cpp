#include "config.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <string>
#include <algorithm>
#include "utils/luaUtils.h"

AppConfig AppConfig::LoadFromFile(const std::string& filePath) {
    AppConfig config;  // Start with defaults

    // Read the Lua file
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return config;  // Return defaults if file not found
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    // Parse the Lua content
    SimpleLuaParser parser;
    try {
        parser.Parse(buffer.str());
    } catch (...) {
        return config;  // Return defaults on parse error
    }

    // Load window settings from "window" table
    config.window_width = ParseLuaInt(parser.GetTableValue("window", "width"), config.window_width);
    config.window_height = ParseLuaInt(parser.GetTableValue("window", "height"), config.window_height);
    config.target_fps = ParseLuaInt(parser.GetTableValue("window", "fps"), config.target_fps);
    config.fullscreen = ParseLuaBool(parser.GetTableValue("window", "fullscreen"), config.fullscreen);

    // Load camera settings from "camera" table
    config.camera_pitch = ParseLuaFloat(parser.GetTableValue("camera", "pitch"), config.camera_pitch);
    config.camera_yaw = ParseLuaFloat(parser.GetTableValue("camera", "yaw"), config.camera_yaw);
    config.camera_roll = ParseLuaFloat(parser.GetTableValue("camera", "roll"), config.camera_roll);
    config.camera_fovy = ParseLuaFloat(parser.GetTableValue("camera", "fovy"), config.camera_fovy);
    config.camera_distance = ParseLuaFloat(parser.GetTableValue("camera", "distance"), config.camera_distance);

    // Load input sensitivity from "input" table
    config.move_speed = ParseLuaFloat(parser.GetTableValue("input", "move_speed"), config.move_speed);
    config.rotation_speed = ParseLuaFloat(parser.GetTableValue("input", "rotation_speed"), config.rotation_speed);
    config.zoom_speed = ParseLuaFloat(parser.GetTableValue("input", "zoom_speed"), config.zoom_speed);
    config.zoom_min = ParseLuaFloat(parser.GetTableValue("input", "zoom_min"), config.zoom_min);
    config.zoom_max = ParseLuaFloat(parser.GetTableValue("input", "zoom_max"), config.zoom_max);

    config.Validate();
    return config;
}

void AppConfig::Validate() {
    // Clamp window dimensions
    window_width = std::max(320, std::min(4096, window_width));
    window_height = std::max(240, std::min(4096, window_height));
    
    // Clamp FPS
    target_fps = std::max(15, std::min(240, target_fps));
    
    // Clamp camera angles (allow full rotation)
    // No strict clamp for angles - let them wrap naturally
    
    // Clamp FOV
    camera_fovy = std::max(5.0f, std::min(120.0f, camera_fovy));
    
    // Clamp camera distance
    camera_distance = std::max(1.0f, std::min(200.0f, camera_distance));
    
    // Clamp input sensitivities
    move_speed = std::max(0.1f, std::min(100.0f, move_speed));
    rotation_speed = std::max(0.1f, std::min(50.0f, rotation_speed));
    zoom_speed = std::max(0.1f, std::min(50.0f, zoom_speed));
    
    // Ensure zoom_min < zoom_max
    if (zoom_min >= zoom_max) {
        zoom_min = 5.0f;
        zoom_max = 80.0f;
    }
    zoom_min = std::max(0.1f, std::min(100.0f, zoom_min));
    zoom_max = std::max(0.1f, std::min(200.0f, zoom_max));
}

AppConfig::AppConfig()
    : window_width(800),
      window_height(600),
      target_fps(60),
      fullscreen(false),
      camera_pitch(35.0f),
      camera_yaw(23.0f),
      camera_roll(0.0f),
      camera_fovy(45.0f),
      camera_distance(22.0f),
      move_speed(15.0f),
      rotation_speed(2.5f),
      zoom_speed(3.0f),
      zoom_min(5.0f),
      zoom_max(80.0f) {
}
