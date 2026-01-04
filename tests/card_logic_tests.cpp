#include <gtest/gtest.h>
#include "card.h"
#include "grid.h"
#include "entity.h"
#include <string>

namespace {

GameState makeState(const Vector2& playerPos, Facing facing = Facing::North, const std::vector<Entity>& extras = {}) {
    GameState gs;
    gs.grid = Grid();
    gs.entities.clear();
    Entity player{1, PLAYER, playerPos, "Player"};
    player.facing = facing;
    gs.entities.push_back(player);
    gs.entities.insert(gs.entities.end(), extras.begin(), extras.end());
    return gs;
}

Card makeMoveCard(int id, const char* name, int fwd, int lat) {
    Card c;
    c.id = id;
    c.name = name;
    c.type = CardType::Move;
    c.effect.type = CardType::Move;
    c.effect.move.forward = fwd;
    c.effect.move.lateral = lat;
    c.mirroredEffect = mirrorEffect(c.effect);
    return c;
}

} // namespace

TEST(CardLogic, MirrorEffectForwardsFlip) {
    CardEffect eff{CardType::Move, {1, 0}};
    CardEffect mirrored = mirrorEffect(eff);
    EXPECT_EQ(mirrored.move.forward, -1);
    EXPECT_EQ(mirrored.move.lateral, 0);
}

TEST(CardLogic, MirrorEffectSwapsLateral) {
    CardEffect eff{CardType::Move, {0, 1}};
    CardEffect mirrored = mirrorEffect(eff);
    EXPECT_EQ(mirrored.move.forward, 0);
    EXPECT_EQ(mirrored.move.lateral, -1);
}

TEST(CardLogic, ApplyCardClampsToGrid) {
    GameState gs = makeState({10.0f, 5.0f});
    Card move = makeMoveCard(1, "Clamp", 0, 5); // would overshoot to x=15
    GameState out = applyCard(gs, move, 1, false);
    ASSERT_EQ(out.entities.size(), 1u);
    EXPECT_FLOAT_EQ(out.entities[0].position.x, Grid::SIZE - 1);
    EXPECT_FLOAT_EQ(out.entities[0].position.y, 5.0f);
}

TEST(CardLogic, ApplyCardRespectsFacingEast) {
    GameState gs = makeState({5.0f, 5.0f}, Facing::East);
    Card move = makeMoveCard(1, "Forward", 1, 0); // local forward
    GameState out = applyCard(gs, move, 1, false);
    EXPECT_FLOAT_EQ(out.entities[0].position.x, 6.0f); // forward rotates to +X
    EXPECT_FLOAT_EQ(out.entities[0].position.y, 5.0f);
}

TEST(CardLogic, ApplyCardRespectsFacingSouthLateral) {
    GameState gs = makeState({5.0f, 5.0f}, Facing::South);
    Card move = makeMoveCard(1, "Right", 0, 1); // local right
    GameState out = applyCard(gs, move, 1, false);
    EXPECT_FLOAT_EQ(out.entities[0].position.x, 4.0f); // right when facing south -> -X
    EXPECT_FLOAT_EQ(out.entities[0].position.y, 5.0f);
}

TEST(CardLogic, ApplyCardBlocksOnCollision) {
    Entity blocker{2, ENEMY, {6.0f, 5.0f}, "Blocker"};
    GameState gs = makeState({5.0f, 5.0f}, {blocker});
    Card move = makeMoveCard(1, "Right", 0, 1);
    GameState out = applyCard(gs, move, 1, false);
    ASSERT_EQ(out.entities.size(), 2u);
    EXPECT_FLOAT_EQ(out.entities[0].position.x, 5.0f);
    EXPECT_FLOAT_EQ(out.entities[0].position.y, 5.0f);
}

TEST(CardLogic, TurnPlanRejectsDuplicateMech) {
    std::vector<Card> hand{makeMoveCard(1, "A", 1, 0), makeMoveCard(2, "B", 0, 1)};
    TurnPlan plan;
    plan.assignments.push_back({1, 1, false});
    plan.assignments.push_back({1, 2, false});
    std::string err;
    EXPECT_FALSE(plan.validate(hand, &err));
}

TEST(CardLogic, TurnPlanRejectsOveruse) {
    std::vector<Card> hand{makeMoveCard(1, "A", 1, 0)}; // only one copy
    TurnPlan plan;
    plan.assignments.push_back({1, 1, false});
    plan.assignments.push_back({2, 1, true});
    std::string err;
    EXPECT_FALSE(plan.validate(hand, &err));
}

TEST(CardLogic, TurnPlanRejectsMoreThanThree) {
    std::vector<Card> hand{
        makeMoveCard(1, "A", 1, 0),
        makeMoveCard(2, "B", 0, 1),
        makeMoveCard(3, "C", -1, 0)
    };
    TurnPlan plan;
    plan.assignments.push_back({1, 1, false});
    plan.assignments.push_back({2, 2, false});
    plan.assignments.push_back({3, 3, false});
    plan.assignments.push_back({4, 1, false});
    std::string err;
    EXPECT_FALSE(plan.validate(hand, &err));
}

TEST(CardLogic, TurnPlanAppliesToSpecificMechFacing) {
    Entity enemy{2, ENEMY, {4.0f, 4.0f}, "Enemy"};
    enemy.facing = Facing::West;
    GameState gs = makeState({5.0f, 5.0f}, Facing::North, {enemy});

    std::vector<Card> hand{makeMoveCard(1, "EnemyForward", 1, 0)};
    TurnPlan plan;
    plan.assignments.push_back({2, 1, false});

    GameState out = plan.apply(gs, hand, gs.grid);
    ASSERT_EQ(out.entities.size(), 2u);
    // Enemy facing West, forward should move -X
    EXPECT_FLOAT_EQ(out.entities[1].position.x, 3.0f);
    EXPECT_FLOAT_EQ(out.entities[1].position.y, 4.0f);
}

TEST(CardLogic, TurnPlanRejectsMissingMechRoster) {
    std::vector<Card> hand{makeMoveCard(1, "A", 1, 0)};
    std::vector<int> mechRoster{1, 2};
    TurnPlan plan;
    plan.assignments.push_back({3, 1, false}); // mech not in roster
    std::string err;
    EXPECT_FALSE(plan.validate(hand, mechRoster, &err));
}

TEST(CardLogic, TurnPlanAcceptsRosteredMechs) {
    std::vector<Card> hand{makeMoveCard(1, "A", 1, 0), makeMoveCard(2, "B", 0, 1)};
    std::vector<int> mechRoster{1, 2};
    TurnPlan plan;
    plan.assignments.push_back({1, 1, false});
    plan.assignments.push_back({2, 2, true});
    std::string err;
    EXPECT_TRUE(plan.validate(hand, mechRoster, &err)) << err;
}

TEST(CardLogic, SerializationRoundTrip) {
    Card card = makeMoveCard(7, "Hook", 1, -1);
    Hand hand;
    hand.addCard(card);
    TurnPlan plan;
    plan.assignments.push_back({42, card.id, true});

    std::string cardJson = serializeCard(card);
    std::string handJson = serializeHand(hand);
    std::string planJson = serializeTurnPlan(plan);

    Card card2;
    Hand hand2;
    TurnPlan plan2;
    ASSERT_TRUE(deserializeCard(cardJson, card2));
    ASSERT_TRUE(deserializeHand(handJson, hand2));
    ASSERT_TRUE(deserializeTurnPlan(planJson, plan2));

    EXPECT_EQ(card2.id, card.id);
    EXPECT_EQ(card2.effect.move.forward, card.effect.move.forward);
    EXPECT_EQ(hand2.cards.size(), 1u);
    EXPECT_EQ(hand2.cards[0].id, card.id);
    ASSERT_EQ(plan2.assignments.size(), 1u);
    EXPECT_EQ(plan2.assignments[0].mechId, 42);
    EXPECT_TRUE(plan2.assignments[0].useMirror);
}

TEST(CardAI, PlanDeterministicWithSeed) {
    Hand handA;
    handA.addCard(makeMoveCard(1, "A", 1, 0));
    handA.addCard(makeMoveCard(2, "B", 0, 1));
    std::vector<int> mechIds{10, 11};

    Hand handB = handA; // fresh usage
    TurnPlan p1 = buildRandomPlan(mechIds, handA, 123u, 0.5f);
    TurnPlan p2 = buildRandomPlan(mechIds, handB, 123u, 0.5f);

    ASSERT_EQ(p1.assignments.size(), p2.assignments.size());
    for (size_t i = 0; i < p1.assignments.size(); ++i) {
        EXPECT_EQ(p1.assignments[i].mechId, p2.assignments[i].mechId);
        EXPECT_EQ(p1.assignments[i].cardId, p2.assignments[i].cardId);
        EXPECT_EQ(p1.assignments[i].useMirror, p2.assignments[i].useMirror);
    }
}

TEST(CardAI, MirrorChanceAllTrueWhenForced) {
    Hand hand;
    hand.addCard(makeMoveCard(1, "A", 1, 0));
    hand.addCard(makeMoveCard(2, "B", 0, 1));
    std::vector<int> mechIds{10, 11};

    TurnPlan plan = buildRandomPlan(mechIds, hand, 5u, 1.0f); // always mirror
    for (const auto& a : plan.assignments) {
        EXPECT_TRUE(a.useMirror);
    }
}

TEST(CardAI, MirrorChanceAllFalseWhenZero) {
    Hand hand;
    hand.addCard(makeMoveCard(1, "A", 1, 0));
    hand.addCard(makeMoveCard(2, "B", 0, 1));
    std::vector<int> mechIds{10, 11};

    TurnPlan plan = buildRandomPlan(mechIds, hand, 7u, 0.0f); // never mirror
    for (const auto& a : plan.assignments) {
        EXPECT_FALSE(a.useMirror);
    }
}
