#pragma once

#include "bossState.h"
#include <memory>

/**
 * BossNpcSelectState: NPC generates its plan
 * 
 * Generates random plan for enemy mechs.
 * Entry: Player has submitted valid plan
 * Exit: NPC plan generated and validated
 * Next: BossPlayState
 */
class BossNpcSelectState : public BossState {
public:
    bool canEnter(Game& game) override;
    bool canExit(Game& game) override;
    void enter(Game& game) override;
    void exit(Game& game) override;
    std::unique_ptr<BossState> update(Game& game, const CardActions& actions, float dt) override;
    const char* getName() const override { return "NpcSelect"; }

private:
    bool npcPlanReady_ = false;
    float elapsed_ = 0.0f;
    
    void buildNpcPlan(Game& game);
};
