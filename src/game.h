#pragma once

#include "platform/platform.h"
#include <vector>
#include "grid.h"
#include "entity.h"
#include "card.h"

struct CardActions;

struct Game {
    Grid grid;
    std::vector<Entity> entities;
    Hand hand;
    Deck deck;          // T_051: Separate deck for drawing cards
    TurnPlan currentPlan;
    TurnPlan lastAiPlan;
    std::string lastAiPlanText;
    int lastSelectedMechId = -1;
    bool mirrorNext = false;
    int pendingCardId = -1;
    bool pendingMirror = false;
    int turnNumber = 0;
    float planetRot = 0.0f;
    float cloudsRot = 0.0f;
};

// Reset per-turn state (hand usage, sequence).
void begin_turn(Game& game);

// Increment turn counter after a full round completes.
void advance_turn(Game& game);

// Execute a full round: player plan then AI response, advancing the turn.
void resolve_round(Game& game, const TurnPlan& playerPlan, uint32_t seed = 0, float mirrorChance = 0.5f);

// Seed a basic scene: player, enemy, object, and sample hand.
void init_game(Game& game);

// Simple per-frame update (movement toward player, rotations).
void update_game(Game& game, float dt);

// Handle user input that affects game state.
void handle_input(Game& game, const Platform& platform);

// Handle actions triggered from the UI. Optional flag to suppress auto-resolve.
void handle_ui_actions(Game& game, const CardActions& actions, bool allowResolve = true);

// Execute a random AI turn for provided seed (uses enemy mech ids if present).
void execute_ai_random_turn(Game& game, uint32_t seed, float mirrorChance = 0.5f);
