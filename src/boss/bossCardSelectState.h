#pragma once

#include "bossState.h"
#include <memory>

/**
 * BossCardSelectState: Player selects cards
 * 
 * Player assigns cards to their mechs via drag-drop UI.
 * Entry: Game initialized
 * Exit: Player submits valid plan with OK button
 * Next: BossNpcSelectState
 */
class BossCardSelectState : public BossState {
public:
    bool canEnter(Game& game) override;
    bool canExit(Game& game) override;
    void enter(Game& game) override;
    void exit(Game& game) override;
    std::unique_ptr<BossState> update(Game& game, const CardActions& actions, float dt) override;
    const char* getName() const override { return "CardSelect"; }

private:
    bool playerPlanValid_ = false;
};
