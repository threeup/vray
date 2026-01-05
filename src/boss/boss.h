#pragma once

#include "bossState.h"
#include "common/statemachine.h"

struct Game;
struct CardActions;

/**
 * Boss: State Machine Orchestrator (uses generic StateMachine foundation)
 */
class Boss {
public:
    Boss();

    void begin(Game& game);
    void update(Game& game, const CardActions& actions, float dt);

    BossState* getCurrentState() const { return machine_.getCurrentState(); }
    const char* getCurrentStateName() const { return machine_.getCurrentStateName(); }

private:
    StateMachine<Game, CardActions> machine_;
};
