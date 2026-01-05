// Phase/turn manager ("Boss") to coordinate player/NPC selection and play phases.
#pragma once

#include "card.h"

struct Game;
struct UiActions;

class Boss {
public:
    enum class Phase { PlayerSelect, NpcSelect, Play };

    Boss();

    Phase getPhase() const { return phase_; }

    // Reset state for a new game/round.
    void begin(Game& game);

    // Process UI intents during the player-select phase. Ignores input in other phases.
    void processUi(Game& game, const UiActions& actions);

    // Advance automatic phases (NPC select -> Play -> back to Player).
    void update(Game& game, float dt);

private:
    Phase phase_ = Phase::PlayerSelect;
    float phaseTime_ = 0.0f;
    bool npcPlanReady_ = false;
    bool playQueued_ = false;
    bool playerPlanLocked_ = false;
    TurnPlan playerPlan_;
    TurnPlan npcPlan_;
    std::vector<PlanAssignment> pendingPlayer_;
    std::vector<PlanAssignment> pendingNpc_;
    int playSubphase_ = 0;
    float playSubphaseTime_ = 0.0f;

    void buildNpcPlan(Game& game);
    bool executePlay(Game& game);
    void runPlaySubphase(Game& game);
    void finishRound(Game& game);
    bool applyAssignment(Game& game, const PlanAssignment& a);
    void enterPhase(Phase p);
};
