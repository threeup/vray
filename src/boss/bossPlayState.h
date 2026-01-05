#pragma once

#include "bossState.h"
#include <memory>
#include <vector>

struct PlanAssignment;

/**
 * BossPlayState: Execute both player and NPC plans
 * 
 * Applies card effects in sequence, one subphase per frame.
 * Entry: Both plans are locked and valid
 * Exit: All assignments executed
 * Next: BossCardSelectState (new round) or BossEndGameState (game over)
 */
class BossPlayState : public BossState {
public:
    bool canEnter(Game& game) override;
    bool canExit(Game& game) override;
    void enter(Game& game) override;
    void exit(Game& game) override;
    std::unique_ptr<BossState> update(Game& game, const CardActions& actions, float dt) override;
    const char* getName() const override { return "Play"; }

private:
    std::vector<PlanAssignment> pendingPlayer_;
    std::vector<PlanAssignment> pendingNpc_;
    int playSubphase_ = 0;
    float playSubphaseTime_ = 0.0f;
    static constexpr float SUBPHASE_DURATION = 0.5f;

    void runPlaySubphase(Game& game);
    bool applyAssignment(Game& game, const PlanAssignment& a);
};
