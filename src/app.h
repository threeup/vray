#pragma once

#include "raylib.h"
#include <memory>

// High-level application context scaffolding, per architecture.md

class WindowInterface; class InputInterface; class RendererInterface;

struct RenderTargets {
    RenderTexture2D scene = {0};
    RenderTexture2D post = {0};
    float scale = 1.0f; // supersample scale
    int width = 0;
    int height = 0;
};

// Shaders for the rendering pipeline
struct RenderShaders {
    Shader flat{0};
    Shader bloom{0};
    Shader pastel{0};
    Shader fxaa{0};
    Shader lighting{0};  // Lighting shader
    int lightDirLoc = -1;
    int lightColorLoc = -1;
    int fxaaResLoc = -1;
    int viewPosLoc = -1;  // For camera position in lighting
    int flatLightPosLoc = -1;  // For light position in flat shader
    int flatViewPosLoc = -1;   // For view position in flat shader
};

// Models used in the scene
struct RenderModels {
    Model cube{0};
    Model spiky{0};
    Model twisted{0};
    Model planet{0};
    Model clouds{0};
    Model sphere{0};  // High-res sphere for lighting demo
    Model sideSphere{0}; // Hexagonal pyramid
    Model pyramid{0}; // Hexagonal pyramid
    Model tree{0}; // Hexagonal pyramid
};

struct UiState {
    bool supersample2x = false;
    bool fxaa = true;
    bool showEntities = true;
    bool showEnvironment = true;
    bool showLightIndicator = true;
    bool bloomEnabled = true;
    float bloomIntensity = 1.0f;
    bool pastelEnabled = true;
    float pastelIntensity = 1.0f;
};

struct Game; // forward declare until game module exists

struct AppContext {
    // Platform systems (owned by Platform, referenced here)
    std::unique_ptr<WindowInterface>& window;
    std::unique_ptr<InputInterface>& input;
    std::unique_ptr<RendererInterface>& renderer;

    Camera3D camera{};
    RenderTargets targets;
    RenderShaders shaders;
    RenderModels models;
    UiState ui;
    Game* game = nullptr; // optional hook to game state when introduced
};
