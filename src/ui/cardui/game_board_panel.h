#pragma once

#include "raylib.h"
#include <vector>
#include <string>

/**
 * T_056: GameBoardPanel (Center Arena Display)
 * 
 * Render enemy mech area (left side) and your mech area (right side).
 * Display game state information (health, effects, modifiers).
 */

struct MechBoardDisplay {
    int entityId = -1;
    Vector2 position = {0, 0};
    int health = 100;
    std::vector<std::string> activeEffects;
};

struct GameBoardPanel {
    Rectangle enemyArea;    // left half
    Rectangle playerArea;   // right half
    Rectangle centerLine;   // divider
    
    void computeLayout(Rectangle boardRect);
};

// Forward declarations
struct Game;

// Draw the game board panel with enemy and player mech areas
void GameBoardPanel_Draw(const Rectangle& rect, Game& game);
