#pragma once

#include <string>
#include "raylib.h" // For Vector2

enum EntityType {
    PLAYER,
    ENEMY,
    OBJECT
};

struct Entity {
    int id;
    EntityType type;
    Vector2 position; // Grid coordinates (0-11)
    std::string name;
    int health = 100; // Example state
    // Add more state as needed
};