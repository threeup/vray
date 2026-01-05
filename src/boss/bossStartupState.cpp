#include "bossStartupState.h"
#include "bossCardSelectState.h"
#include "game.h"
#include "ui.h"
#include <iostream>

bool BossStartupState::canEnter(Game& game) {
    // Can always enter startup as initial state
    return true;
}

bool BossStartupState::canExit(Game& game) {
    // Can exit after initialization completes
    return elapsed_ >= STARTUP_DURATION;
}

void BossStartupState::enter(Game& game) {
    elapsed_ = 0.0f;
    TraceLog(LOG_INFO, "[BossState] Entering Startup");
}

void BossStartupState::exit(Game& game) {
    TraceLog(LOG_INFO, "[BossState] Exiting Startup");
}

std::unique_ptr<BossState> BossStartupState::update(Game& game, const CardActions& actions, float dt) {
    elapsed_ += dt;
    
    TraceLog(LOG_DEBUG, "[Startup] elapsed=%.2fs / %.2fs", elapsed_, STARTUP_DURATION);

    // Check if we can transition to CardSelect
    if (canExit(game)) {
        TraceLog(LOG_INFO, "[Startup::STARTUP_COMPLETE] Startup phase complete, requesting transition to CardSelect");
        auto nextState = std::make_unique<BossCardSelectState>();
        if (nextState->canEnter(game)) {
            return nextState;
        } else {
            TraceLog(LOG_WARNING, "[Startup] CardSelect state rejected entry: canEnter() returned false");
            return nullptr;  // Stay in startup
        }
    }

    return nullptr;  // Stay in current state
}
