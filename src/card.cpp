#include "card.h"
#include <algorithm>

GameState applyCard(const GameState& state, const Card& card, int playerId) {
    GameState newState = state; // Copy

    for (const auto& effect : card.effects) {
        switch (effect.type) {
        case MOVE_ENTITY: {
            // Find entity by ID or player
            auto it = std::find_if(newState.entities.begin(), newState.entities.end(),
                [playerId](const Entity& e) { return e.id == playerId; });
            if (it != newState.entities.end()) {
                it->position = effect.moveTo;
                // Update grid occupancy if needed
            }
            break;
        }
        case DAMAGE_ENTITY: {
            auto it = std::find_if(newState.entities.begin(), newState.entities.end(),
                [effect](const Entity& e) { return e.id == effect.targetEntityId; });
            if (it != newState.entities.end()) {
                it->health -= effect.damage;
                if (it->health < 0) it->health = 0;
            }
            break;
        }
        case HEAL_ENTITY: {
            auto it = std::find_if(newState.entities.begin(), newState.entities.end(),
                [playerId](const Entity& e) { return e.id == playerId; });
            if (it != newState.entities.end()) {
                it->health += effect.heal;
                if (it->health > 100) it->health = 100;
            }
            break;
        }
        }
    }

    return newState;
}

GameState applySequence(const GameState& state, const Sequence& sequence, int playerId) {
    GameState currentState = state;
    for (const auto& card : sequence) {
        currentState = applyCard(currentState, card, playerId);
    }
    return currentState;
}