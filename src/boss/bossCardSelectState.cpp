#include "bossCardSelectState.h"
#include "bossNpcSelectState.h"
#include "game.h"
#include "ui.h"
#include <vector>

bool BossCardSelectState::canEnter(Game& game) {
    // Can enter if game is initialized
    return !game.entities.empty();
}

bool BossCardSelectState::canExit(Game& game) {
    // Can exit if player has submitted a valid plan
    return playerPlanValid_;
}

void BossCardSelectState::enter(Game& game) {
    playerPlanValid_ = false;
    game.hand.resetUsage();
    game.currentPlan.assignments.clear();
    TraceLog(LOG_INFO, "[BossState] Entering CardSelect");
}

void BossCardSelectState::exit(Game& game) {
    TraceLog(LOG_INFO, "[BossState] Exiting CardSelect");
}

std::unique_ptr<BossState> BossCardSelectState::update(Game& game, const CardActions& actions, float dt) {
    // Check if player submitted a plan via OK button
    if (actions.playSequence) {
        TraceLog(LOG_INFO, "[CardSelect] OK button pressed, validating player plan...");
        
        // Validate the plan
        std::vector<int> playerMechs;
        for (const auto& entity : game.entities) {
            if (entity.type == PLAYER) {
                playerMechs.push_back(entity.id);
                if (playerMechs.size() >= 3) break;
            }
        }
        
        TraceLog(LOG_DEBUG, "[CardSelect] Found %zu player mechs", playerMechs.size());
        TraceLog(LOG_DEBUG, "[CardSelect] Current plan has %zu assignments", 
                 game.currentPlan.assignments.size());

        std::string err;
        if (game.currentPlan.validate(game.hand.cards, playerMechs, &err)) {
            // Valid plan submitted
            TraceLog(LOG_INFO, "[CardSelect::PLAN_VALID] Player plan validated successfully");
            playerPlanValid_ = true;
            
            // Try to transition
            auto nextState = std::make_unique<BossNpcSelectState>();
            if (nextState->canEnter(game)) {
                TraceLog(LOG_INFO, "[CardSelect::TRANSITION_ATTEMPT] Plan valid, requesting transition to NpcSelect");
                return nextState;
            } else {
                TraceLog(LOG_WARNING, "[CardSelect] NpcSelect state rejected entry: canEnter() returned false");
                playerPlanValid_ = false;
                return nullptr;
            }
        } else {
            // Invalid plan
            TraceLog(LOG_WARNING, "[CardSelect::PLAN_INVALID] Validation failed: %s", err.c_str());
            playerPlanValid_ = false;
            return nullptr;
        }
    }

    return nullptr;  // Stay in current state
}
