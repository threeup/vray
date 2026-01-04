#pragma once

#include "raylib.h"
#include <memory>

// High-level application context scaffolding, per architecture.md

class WindowInterface; class InputInterface; class RendererInterface; class Boss;

// Faction Color Palettes
enum class FactionType {
    RedFaction = 0,
    BlueFaction = 1,
    GreenFaction = 2,
    Neutral = 3
};

struct ColorPalette {
    Vector3 highlight;    // Bright
    Vector3 base;         // Mid
    Vector3 shade;        // Dark
    Vector3 deepShadow;   // Darkest
};

// Static faction palettes
constexpr ColorPalette PALETTE_RED = {
    {255.0f/255, 120.0f/255, 120.0f/255},  // Highlight
    {232.0f/255, 32.0f/255, 32.0f/255},    // Base
    {168.0f/255, 16.0f/255, 16.0f/255},    // Shade
    {88.0f/255, 8.0f/255, 8.0f/255}        // Deep Shadow
};

constexpr ColorPalette PALETTE_BLUE = {
    {144.0f/255, 184.0f/255, 255.0f/255},  // Highlight
    {40.0f/255, 96.0f/255, 232.0f/255},    // Base
    {24.0f/255, 56.0f/255, 160.0f/255},    // Shade
    {16.0f/255, 24.0f/255, 80.0f/255}      // Deep Shadow
};

constexpr ColorPalette PALETTE_GREEN = {
    {160.0f/255, 240.0f/255, 136.0f/255},  // Highlight
    {56.0f/255, 184.0f/255, 48.0f/255},    // Base
    {32.0f/255, 120.0f/255, 24.0f/255},    // Shade
    {16.0f/255, 64.0f/255, 16.0f/255}      // Deep Shadow
};

constexpr ColorPalette PALETTE_NEUTRAL = {
    {192.0f/255, 192.0f/255, 192.0f/255},  // Highlight: Light Grey
    {144.0f/255, 144.0f/255, 152.0f/255},  // Base: Mid Grey
    {96.0f/255, 96.0f/255, 104.0f/255},    // Shade: Dark Grey
    {48.0f/255, 48.0f/255, 56.0f/255}      // Deep Shadow: Very Dark Grey
};

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
    Shader palette{0};
    Shader fxaa{0};
    Shader lighting{0};  // Lighting shader
    int lightDirLoc = -1;
    int lightColorLoc = -1;
    int fxaaResLoc = -1;
    int viewPosLoc = -1;  // For camera position in lighting
    int flatLightPosLoc = -1;  // For light position in flat shader
    int flatViewPosLoc = -1;   // For view position in flat shader
    int flatPaletteEnabledLoc = -1; // Enable/disable palette per draw
    int flatPaletteIndexLoc = -1;   // Palette selector per draw
    int flatPaletteStrengthLoc = -1;// Palette blend per draw
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
    bool paletteEnabled = false;
    float paletteStrength = 1.0f;
    bool renderControlsCollapsed = true;
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
    Boss* boss = nullptr; // optional hook to turn/phase controller
};
