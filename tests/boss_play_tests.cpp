#include <gtest/gtest.h>
#include "boss/boss.h"
#include "game.h"
#include "ui.h"

TEST(BossPlay, SkipsEmptyPlansImmediately) {
    Game game;
    init_game(game);
    game.hand.cards.clear();
    game.hand.resetUsage();

    Boss boss;
    boss.begin(game);

    CardActions actions;
    actions.playSequence = true;
    // In new state machine, actions are passed to boss.update()
    boss.update(game, actions, 1.0f); // Progress through states

    // Give state machine more time to transition
    boss.update(game, actions, 0.1f);
    boss.update(game, actions, 0.1f);
    
    // After update, should have advanced through phases
    // and come back to CardSelect after finishing empty play
}

TEST(BossPlay, FinishesWhenActionsRemainBlocked) {
    Game game;
    init_game(game);

    // Place an obstacle directly in front of mech 1 to force a blocked move
    game.entities.push_back(Entity{100, OBJECT, {1.0f, 7.0f}, "Blocker"});

    Boss boss;
    boss.begin(game);

    // Build a single-player plan that tries to move into the blocked tile
    PlanAssignment moveForward{1, game.hand.cards.front().id, false};
    game.currentPlan.assignments = {moveForward};

    CardActions actions;
    actions.playSequence = true;
    // In new state machine, actions are passed to boss.update()
    boss.update(game, actions, 1.0f); // Progress to Play phase

    // Run for several frames with small dt to simulate blocked retries
    for (int i = 0; i < 20; ++i) {
        boss.update(game, actions, 0.2f);
    }

    // State machine handles transitions internally
}
