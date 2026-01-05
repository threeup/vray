#pragma once

#include "bossState.h"
#include <memory>

/**
 * BossEndGameState: Game over
 * 
 * Final state when game ends (all enemies defeated or player loses).
 * Entry: Win/loss conditions met
 * Exit: Never (game over)
 */
class BossEndGameState : public BossState {
public:
    bool canEnter(Game& game) override;
    bool canExit(Game& game) override;
    void enter(Game& game) override;
    void exit(Game& game) override;
    std::unique_ptr<BossState> update(Game& game, const CardActions& actions, float dt) override;
    const char* getName() const override { return "EndGame"; }

private:
    enum class EndReason { PlayerVictory, PlayerDefeat, Draw };
    EndReason reason_ = EndReason::Draw;
};
