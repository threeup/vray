#pragma once
#include <vector>
#include <array>
#include "raylib.h"
#include "rlights.h" // For Light type and MAX_LIGHTS

struct AppContext; // forward

// Tile definitions for an 8x8 board
enum class TileType {
    Dirt,
    Forest,
    Skyscraper,
    Water,
    Mountain,
    SpawnHero,
    SpawnEnemy
};

enum class Occupant {
    None,
    Hero,
    Enemy
};

// Represents a single object in the world
struct WorldEntity {
    Model model;          // The visual representation
    Vector3 position;     // Where it is
    Vector3 scale;        // Size
    Color color;          // Tint applied when drawing
    // Add rotation/ID/Type here as needed
};

// Holds all game state data
struct World {
    static constexpr int kTilesWide = 8;
    static constexpr int kTilesHigh = 8;
    static constexpr float kTileSize = 2.0f; // enlarge board spacing

    std::array<TileType, kTilesWide * kTilesHigh> tiles{};
    std::array<Occupant, kTilesWide * kTilesHigh> occupants{};

    std::vector<WorldEntity> entities;
    Light lights[MAX_LIGHTS];
    int lightCount;
    int activeLight; // Index of the main light
};

// Build initial world entities/lights using shaders from AppContext
void World_Init(World& world, const AppContext& appCtx);

// Draw ground/grid for the world
void World_DrawGround(const World& world);