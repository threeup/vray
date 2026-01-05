#include <gtest/gtest.h>
#include "boss.h"
#include "game.h"
#include "ui.h"

TEST(BossPlay, SkipsEmptyPlansImmediately) {
    Game game;
    init_game(game);
    game.hand.cards.clear();
    game.hand.resetUsage();

    Boss boss;
    boss.begin(game);

    UiActions actions;
    actions.playSequence = true;
    boss.processUi(game, actions);

    boss.update(game, 1.0f); // Progress to Play phase
    ASSERT_EQ(boss.getPhase(), Boss::Phase::Play);

    boss.update(game, 0.01f);
    EXPECT_EQ(boss.getPhase(), Boss::Phase::PlayerSelect);
    EXPECT_EQ(game.turnNumber, 2);
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

    UiActions actions;
    actions.playSequence = true;
    boss.processUi(game, actions);

    boss.update(game, 1.0f); // enter Play
    ASSERT_EQ(boss.getPhase(), Boss::Phase::Play);

    // Run for several frames with small dt to simulate blocked retries
    for (int i = 0; i < 20 && boss.getPhase() == Boss::Phase::Play; ++i) {
        boss.update(game, 0.2f);
    }

    EXPECT_EQ(boss.getPhase(), Boss::Phase::PlayerSelect);
    EXPECT_EQ(game.turnNumber, 2);
}
