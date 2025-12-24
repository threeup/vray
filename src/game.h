#pragma once

#include "platform/platform.h"
#include <vector>
#include "grid.h"
#include "entity.h"
#include "card.h"

struct UiActions;

struct Game {
    Grid grid;
    std::vector<Entity> entities;
    Sequence currentSeq;
    std::vector<Card> hand;
    float planetRot = 0.0f;
    float cloudsRot = 0.0f;
};

// Seed a basic scene: player, enemy, object, and sample hand.
void init_game(Game& game);

// Simple per-frame update (movement toward player, rotations).
void update_game(Game& game, float dt);

// Handle user input that affects game state.
void handle_input(Game& game, const Platform& platform);

// Handle actions triggered from the UI.
void handle_ui_actions(Game& game, const UiActions& actions);
