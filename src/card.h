#pragma once

#include <string>
#include <vector>
#include <functional>
#include "entity.h"
#include "grid.h"

enum CardEffectType {
    MOVE_ENTITY,
    DAMAGE_ENTITY,
    HEAL_ENTITY,
    // Add more as needed
};

struct CardEffect {
    CardEffectType type;
    int targetEntityId = -1; // -1 for self or specific
    Vector2 moveTo = {0, 0}; // For move
    int damage = 0; // For damage
    int heal = 0; // For heal
};

struct Card {
    int id;
    std::string name;
    std::vector<CardEffect> effects;
};

using Sequence = std::vector<Card>;

struct GameState {
    Grid grid;
    std::vector<Entity> entities;
    int currentTurn = 0;
    // Add more state as needed
};

// Function to apply a card's effects to the game state
GameState applyCard(const GameState& state, const Card& card, int playerId);

// Function to apply a sequence
GameState applySequence(const GameState& state, const Sequence& sequence, int playerId);