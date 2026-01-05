#pragma once

#include "bossState.h"
#include <memory>

/**
 * BossStartupState: Initial phase
 * 
 * Initializes the game and transitions to CardSelect.
 * Entry: Game not yet started
 * Exit: When ready to accept player input
 * Next: BossCardSelectState
 */
class BossStartupState : public BossState {
public:
    bool canEnter(Game& game) override;
    bool canExit(Game& game) override;
    void enter(Game& game) override;
    void exit(Game& game) override;
    std::unique_ptr<BossState> update(Game& game, const CardActions& actions, float dt) override;
    const char* getName() const override { return "Startup"; }

private:
    float elapsed_ = 0.0f;
    static constexpr float STARTUP_DURATION = 0.5f;  // Transition after 0.5s
};
