#include "bossEndGameState.h"
#include "game.h"
#include "ui.h"

bool BossEndGameState::canEnter(Game& game) {
    // Check win/loss conditions
    // TODO: Implement actual end game detection
    return false;  // Never auto-enter yet
}

bool BossEndGameState::canExit(Game& game) {
    // Cannot exit end game state
    return false;
}

void BossEndGameState::enter(Game& game) {
    // Determine end reason
    // TODO: Detect win/loss conditions
    TraceLog(LOG_INFO, "[BossState] Entering EndGame");
}

void BossEndGameState::exit(Game& game) {
    // Should never be called
}

std::unique_ptr<BossState> BossEndGameState::update(Game& game, const CardActions& actions, float dt) {
    // Stay in end game state forever
    return nullptr;
}
