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

// Shared ground constants/utilities (keep rendering and placement consistent)
inline constexpr float kTileSlabThickness = 0.80f; // slab height used when drawing tiles

inline float TileBaseHeight(TileType type) {
    switch (type) {
    case TileType::Water: return -0.05f;
    case TileType::Mountain: return 0.6f;
    case TileType::Skyscraper: return 0.12f; // footprint pad; building added as prop
    default: return 0.0f;
    }
}

inline float TileSurfaceHeight(TileType type) {
    return TileBaseHeight(type) + kTileSlabThickness;
}

inline float ActorBaseHeight(TileType type) {
    constexpr float kFootClearance = 0.02f; // tiny lift to avoid z-fighting with slabs
    return TileSurfaceHeight(type) + kFootClearance;
}

// Represents a single object in the world
struct WorldEntity {
    Model model;          // The visual representation
    Vector3 position;     // Where it is (real-time, may be interpolated)
    Vector3 startPos;     // Position at the beginning of the current turn
    Vector3 targetPos;    // Target position for movement
    Vector3 scale;        // Size
    Color color;          // Tint applied when drawing
    int id;               // Entity ID for lookup
    float moveProgress;   // 0.0 = at current waypoint, 1.0 = at target (for lerp animation)
    bool isActor = false; // True for mechs (heroes/enemies), false for props
    bool isEnemy = false; // Patrol only enemies
    std::array<Vector3, 4> patrolPoints{}; // Clockwise square loop
    int patrolIndex = 0;  // Index into patrolPoints for the next target
    // Add rotation/ID/Type here as needed
};

// Holds all game state data
struct World {
    static constexpr int kTilesWide = 8;
    static constexpr int kTilesHigh = 8;
    static constexpr float kTileSize = 2.0f; // enlarge board spacing
    static constexpr float kTurnDuration = 5.0f; // 5-second turns

    std::array<TileType, kTilesWide * kTilesHigh> tiles{};
    std::array<Occupant, kTilesWide * kTilesHigh> occupants{};

    std::vector<WorldEntity> entities;
    Light lights[MAX_LIGHTS];
    int lightCount;
    int activeLight; // Index of the main light
    
    // Turn system
    int currentTurn = 0;
    float turnElapsedTime = 0.0f;  // Time within current turn
};

// Build initial world entities/lights using shaders from AppContext
void World_Init(World& world, const AppContext& appCtx);

// Update world state (including light cycling)
void World_Update(World& world, float elapsedTime);

// Draw ground/grid for the world
void World_DrawGround(const World& world, const AppContext& appCtx);