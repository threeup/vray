#pragma once

#include <string>
#include "raylib.h" // For Vector2

enum EntityType {
    PLAYER,
    ENEMY,
    OBJECT
};

enum class Facing {
    North = 0,
    East  = 1,
    South = 2,
    West  = 3
};

struct Entity {
    int id;
    EntityType type;
    Vector2 position; // Grid coordinates (0-11)
    std::string name;
    int health = 100; // Example state
    Facing facing = Facing::North;
    // Add more state as needed
};