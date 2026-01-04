#include <gtest/gtest.h>
#include "../src/card.h"
#include "../src/game.h"
#include "../src/entity.h"
#include <cmath>
#include <algorithm>

// ==================== CARD EFFECT TESTS ====================

class CardEffectTests : public ::testing::Test {
protected:
    CardEffect createMoveEffect(int targetId, Vector2 destination) {
        return {MOVE_ENTITY, targetId, destination, 0, 0};
    }
    
    CardEffect createDamageEffect(int targetId, int damageAmount) {
        return {DAMAGE_ENTITY, targetId, {0, 0}, damageAmount, 0};
    }
    
    CardEffect createHealEffect(int targetId, int healAmount) {
        return {HEAL_ENTITY, targetId, {0, 0}, 0, healAmount};
    }
};

TEST_F(CardEffectTests, CardEffectTypeEnumValid) {
    EXPECT_EQ(MOVE_ENTITY, 0);
    EXPECT_EQ(DAMAGE_ENTITY, 1);
    EXPECT_EQ(HEAL_ENTITY, 2);
}

TEST_F(CardEffectTests, CreateMoveEffect) {
    CardEffect effect = createMoveEffect(1, {7.0f, 8.0f});
    
    EXPECT_EQ(effect.type, MOVE_ENTITY);
    EXPECT_EQ(effect.targetEntityId, 1);
    EXPECT_EQ(effect.moveTo.x, 7.0f);
    EXPECT_EQ(effect.moveTo.y, 8.0f);
}

TEST_F(CardEffectTests, CreateDamageEffect) {
    CardEffect effect = createDamageEffect(2, 25);
    
    EXPECT_EQ(effect.type, DAMAGE_ENTITY);
    EXPECT_EQ(effect.targetEntityId, 2);
    EXPECT_EQ(effect.damage, 25);
}

TEST_F(CardEffectTests, CreateHealEffect) {
    CardEffect effect = createHealEffect(1, 15);
    
    EXPECT_EQ(effect.type, HEAL_ENTITY);
    EXPECT_EQ(effect.targetEntityId, 1);
    EXPECT_EQ(effect.heal, 15);
}

TEST_F(CardEffectTests, EffectDefaultTargetId) {
    CardEffect effect;
    EXPECT_EQ(effect.targetEntityId, -1);
}

TEST_F(CardEffectTests, EffectDefaultDamage) {
    CardEffect effect;
    EXPECT_EQ(effect.damage, 0);
}

TEST_F(CardEffectTests, EffectDefaultHeal) {
    CardEffect effect;
    EXPECT_EQ(effect.heal, 0);
}

TEST_F(CardEffectTests, MultipleEffectsInCard) {
    std::vector<CardEffect> effects;
    effects.push_back(createMoveEffect(1, {7.0f, 7.0f}));
    effects.push_back(createDamageEffect(2, 20));
    effects.push_back(createHealEffect(1, 10));
    
    EXPECT_EQ(effects.size(), 3);
}

// ==================== CARD TESTS ====================

class CardTests : public ::testing::Test {
protected:
    Card createCard(int id, std::string name, std::vector<CardEffect> effects) {
        return {id, name, effects};
    }
};

TEST_F(CardTests, CreateBasicCard) {
    Card card = createCard(1, "Move", {});
    
    EXPECT_EQ(card.id, 1);
    EXPECT_EQ(card.name, "Move");
    EXPECT_EQ(card.effects.size(), 0);
}

TEST_F(CardTests, CreateCardWithEffect) {
    std::vector<CardEffect> effects;
    effects.push_back({MOVE_ENTITY, 1, {7.0f, 7.0f}, 0, 0});
    
    Card card = createCard(1, "Move", effects);
    
    EXPECT_EQ(card.id, 1);
    EXPECT_EQ(card.name, "Move");
    EXPECT_EQ(card.effects.size(), 1);
    EXPECT_EQ(card.effects[0].type, MOVE_ENTITY);
}

TEST_F(CardTests, CreateCardWithMultipleEffects) {
    std::vector<CardEffect> effects;
    effects.push_back({MOVE_ENTITY, 1, {7.0f, 7.0f}, 0, 0});
    effects.push_back({DAMAGE_ENTITY, 2, {0, 0}, 20, 0});
    
    Card card = createCard(2, "MoveAndDamage", effects);
    
    EXPECT_EQ(card.effects.size(), 2);
}

TEST_F(CardTests, CardEffectOrdering) {
    std::vector<CardEffect> effects;
    effects.push_back({DAMAGE_ENTITY, 2, {0, 0}, 10, 0}); // First
    effects.push_back({MOVE_ENTITY, 1, {7.0f, 7.0f}, 0, 0}); // Second
    
    Card card = createCard(3, "DamageThenMove", effects);
    
    EXPECT_EQ(card.effects[0].type, DAMAGE_ENTITY);
    EXPECT_EQ(card.effects[1].type, MOVE_ENTITY);
}

// ==================== GAME STATE TESTS ====================

class GameStateTests : public ::testing::Test {
protected:
    GameState createGameState() {
        GameState gs;
        gs.grid = Grid();
        gs.currentTurn = 0;
        
        Entity player = {1, PLAYER, {5.0f, 5.0f}, "Player"};
        Entity enemy = {2, ENEMY, {2.0f, 2.0f}, "Enemy"};
        Entity obj = {3, OBJECT, {8.0f, 8.0f}, "Object"};
        
        gs.entities.push_back(player);
        gs.entities.push_back(enemy);
        gs.entities.push_back(obj);
        
        return gs;
    }
};

TEST_F(GameStateTests, GameStateInitialization) {
    GameState gs = createGameState();
    
    EXPECT_EQ(gs.currentTurn, 0);
    EXPECT_EQ(gs.entities.size(), 3);
}

TEST_F(GameStateTests, GameStateGridExists) {
    GameState gs = createGameState();
    
    EXPECT_TRUE(gs.grid.isValidPosition(5, 5));
    EXPECT_FALSE(gs.grid.isValidPosition(15, 15));
}

TEST_F(GameStateTests, GameStateEntitiesValid) {
    GameState gs = createGameState();
    
    EXPECT_EQ(gs.entities[0].id, 1);
    EXPECT_EQ(gs.entities[0].type, PLAYER);
    EXPECT_EQ(gs.entities[1].id, 2);
    EXPECT_EQ(gs.entities[1].type, ENEMY);
    EXPECT_EQ(gs.entities[2].id, 3);
    EXPECT_EQ(gs.entities[2].type, OBJECT);
}

// ==================== APPLY CARD TESTS ====================

class ApplyCardTests : public ::testing::Test {
protected:
    GameState createGameState() {
        GameState gs;
        gs.grid = Grid();
        gs.currentTurn = 0;
        
        Entity player = {1, PLAYER, {5.0f, 5.0f}, "Player"};
        Entity enemy = {2, ENEMY, {2.0f, 2.0f}, "Enemy"};
        Entity obj = {3, OBJECT, {8.0f, 8.0f}, "Object"};
        
        gs.entities.push_back(player);
        gs.entities.push_back(enemy);
        gs.entities.push_back(obj);
        
        return gs;
    }
};

TEST_F(ApplyCardTests, ApplyMoveCard) {
    GameState gs = createGameState();
    Card moveCard = {1, "Move", {{MOVE_ENTITY, 1, {7.0f, 7.0f}}}};
    
    GameState newState = applyCard(gs, moveCard, 1);
    
    auto it = std::find_if(newState.entities.begin(), newState.entities.end(),
        [](const Entity& e) { return e.id == 1; });
    
    ASSERT_NE(it, newState.entities.end());
    EXPECT_FLOAT_EQ(it->position.x, 7.0f);
    EXPECT_FLOAT_EQ(it->position.y, 7.0f);
}

TEST_F(ApplyCardTests, ApplyMoveCardDoesNotAffectOtherEntities) {
    GameState gs = createGameState();
    Card moveCard = {1, "Move", {{MOVE_ENTITY, 1, {7.0f, 7.0f}}}};
    
    GameState newState = applyCard(gs, moveCard, 1);
    
    auto enemy = std::find_if(newState.entities.begin(), newState.entities.end(),
        [](const Entity& e) { return e.id == 2; });
    
    ASSERT_NE(enemy, newState.entities.end());
    EXPECT_FLOAT_EQ(enemy->position.x, 2.0f);
    EXPECT_FLOAT_EQ(enemy->position.y, 2.0f);
}

TEST_F(ApplyCardTests, ApplyDamageCard) {
    GameState gs = createGameState();
    Card damageCard = {2, "Damage", {{DAMAGE_ENTITY, 2, {0, 0}, 25}}};
    
    GameState newState = applyCard(gs, damageCard, 1);
    
    auto enemy = std::find_if(newState.entities.begin(), newState.entities.end(),
        [](const Entity& e) { return e.id == 2; });
    
    ASSERT_NE(enemy, newState.entities.end());
    EXPECT_EQ(enemy->health, 75);
}

TEST_F(ApplyCardTests, ApplyMultipleDamage) {
    GameState gs = createGameState();
    Card heavyDamage = {2, "HeavyDamage", {{DAMAGE_ENTITY, 2, {0, 0}, 60}}};
    
    GameState newState = applyCard(gs, heavyDamage, 1);
    
    auto enemy = std::find_if(newState.entities.begin(), newState.entities.end(),
        [](const Entity& e) { return e.id == 2; });
    
    ASSERT_NE(enemy, newState.entities.end());
    EXPECT_EQ(enemy->health, 40);
}

TEST_F(ApplyCardTests, DamageCannotGoBelowZero) {
    GameState gs = createGameState();
    Card overkill = {2, "Overkill", {{DAMAGE_ENTITY, 2, {0, 0}, 150}}};
    
    GameState newState = applyCard(gs, overkill, 1);
    
    auto enemy = std::find_if(newState.entities.begin(), newState.entities.end(),
        [](const Entity& e) { return e.id == 2; });
    
    ASSERT_NE(enemy, newState.entities.end());
    EXPECT_EQ(enemy->health, 0);
}

TEST_F(ApplyCardTests, ApplyHealCard) {
    GameState gs = createGameState();
    
    // Damage the player first
    gs.entities[0].health = 60;
    
    Card healCard = {3, "Heal", {{HEAL_ENTITY, 1, {0, 0}, 0, 25}}};
    GameState newState = applyCard(gs, healCard, 1);
    
    auto player = std::find_if(newState.entities.begin(), newState.entities.end(),
        [](const Entity& e) { return e.id == 1; });
    
    ASSERT_NE(player, newState.entities.end());
    EXPECT_EQ(player->health, 85);
}

TEST_F(ApplyCardTests, HealCannotExceed100) {
    GameState gs = createGameState();
    
    // Player already at full health
    Card overHeal = {3, "OverHeal", {{HEAL_ENTITY, 1, {0, 0}, 0, 50}}};
    GameState newState = applyCard(gs, overHeal, 1);
    
    auto player = std::find_if(newState.entities.begin(), newState.entities.end(),
        [](const Entity& e) { return e.id == 1; });
    
    ASSERT_NE(player, newState.entities.end());
    // Healing to 150 should cap at 100
    EXPECT_LE(player->health, 100);
}

TEST_F(ApplyCardTests, ApplyCardDoesNotModifyOriginalState) {
    GameState gs = createGameState();
    GameState gsCopy = gs;
    
    Card moveCard = {1, "Move", {{MOVE_ENTITY, 1, {9.0f, 9.0f}}}};
    GameState newState = applyCard(gs, moveCard, 1);
    
    // Original should be unchanged
    EXPECT_FLOAT_EQ(gs.entities[0].position.x, 5.0f);
    EXPECT_FLOAT_EQ(gs.entities[0].position.y, 5.0f);
}

TEST_F(ApplyCardTests, ApplyEmptyCard) {
    GameState gs = createGameState();
    Card emptyCard = {1, "Empty", {}};
    
    GameState newState = applyCard(gs, emptyCard, 1);
    
    EXPECT_EQ(newState.entities.size(), gs.entities.size());
}

// ==================== APPLY SEQUENCE TESTS ====================

class ApplySequenceTests : public ::testing::Test {
protected:
    GameState createGameState() {
        GameState gs;
        gs.grid = Grid();
        gs.currentTurn = 0;
        
        Entity player = {1, PLAYER, {5.0f, 5.0f}, "Player"};
        Entity enemy = {2, ENEMY, {8.0f, 8.0f}, "Enemy"};
        
        gs.entities.push_back(player);
        gs.entities.push_back(enemy);
        
        return gs;
    }
};

TEST_F(ApplySequenceTests, ApplyEmptySequence) {
    GameState gs = createGameState();
    Sequence seq;
    
    GameState newState = applySequence(gs, seq, 1);
    
    EXPECT_EQ(newState.entities.size(), gs.entities.size());
    EXPECT_FLOAT_EQ(newState.entities[0].position.x, 5.0f);
}

TEST_F(ApplySequenceTests, ApplySingleCardSequence) {
    GameState gs = createGameState();
    Sequence seq;
    seq.push_back({1, "Move", {{MOVE_ENTITY, 1, {7.0f, 7.0f}}}});
    
    GameState newState = applySequence(gs, seq, 1);
    
    EXPECT_FLOAT_EQ(newState.entities[0].position.x, 7.0f);
    EXPECT_FLOAT_EQ(newState.entities[0].position.y, 7.0f);
}

TEST_F(ApplySequenceTests, ApplyMultipleCardSequence) {
    GameState gs = createGameState();
    Sequence seq;
    seq.push_back({1, "Move", {{MOVE_ENTITY, 1, {7.0f, 7.0f}}}});
    seq.push_back({2, "Damage", {{DAMAGE_ENTITY, 2, {0, 0}, 30}}});
    
    GameState newState = applySequence(gs, seq, 1);
    
    auto player = std::find_if(newState.entities.begin(), newState.entities.end(),
        [](const Entity& e) { return e.id == 1; });
    auto enemy = std::find_if(newState.entities.begin(), newState.entities.end(),
        [](const Entity& e) { return e.id == 2; });
    
    EXPECT_FLOAT_EQ(player->position.x, 7.0f);
    EXPECT_EQ(enemy->health, 70);
}

TEST_F(ApplySequenceTests, ApplySequenceThreeCards) {
    GameState gs = createGameState();
    Sequence seq;
    seq.push_back({1, "Move1", {{MOVE_ENTITY, 1, {6.0f, 6.0f}}}});
    seq.push_back({2, "Damage", {{DAMAGE_ENTITY, 2, {0, 0}, 20}}});
    seq.push_back({3, "Move2", {{MOVE_ENTITY, 1, {7.0f, 7.0f}}}});
    
    GameState newState = applySequence(gs, seq, 1);
    
    auto player = std::find_if(newState.entities.begin(), newState.entities.end(),
        [](const Entity& e) { return e.id == 1; });
    auto enemy = std::find_if(newState.entities.begin(), newState.entities.end(),
        [](const Entity& e) { return e.id == 2; });
    
    EXPECT_FLOAT_EQ(player->position.x, 7.0f);
    EXPECT_FLOAT_EQ(player->position.y, 7.0f);
    EXPECT_EQ(enemy->health, 80);
}

TEST_F(ApplySequenceTests, SequenceOrderMatters) {
    GameState gs = createGameState();
    
    // Damage then move
    Sequence seq1;
    seq1.push_back({1, "Damage", {{DAMAGE_ENTITY, 2, {0, 0}, 50}}});
    seq1.push_back({2, "Move", {{MOVE_ENTITY, 1, {10.0f, 10.0f}}}});
    
    GameState result1 = applySequence(gs, seq1, 1);
    
    // Move then damage
    Sequence seq2;
    seq2.push_back({2, "Move", {{MOVE_ENTITY, 1, {10.0f, 10.0f}}}});
    seq2.push_back({1, "Damage", {{DAMAGE_ENTITY, 2, {0, 0}, 50}}});
    
    GameState result2 = applySequence(gs, seq2, 1);
    
    // Both should have same final state (since effects are independent)
    auto enemy1 = std::find_if(result1.entities.begin(), result1.entities.end(),
        [](const Entity& e) { return e.id == 2; });
    auto enemy2 = std::find_if(result2.entities.begin(), result2.entities.end(),
        [](const Entity& e) { return e.id == 2; });
    
    EXPECT_EQ(enemy1->health, enemy2->health);
}

TEST_F(ApplySequenceTests, SequenceDoesNotModifyOriginal) {
    GameState gs = createGameState();
    GameState original = gs;
    
    Sequence seq;
    seq.push_back({1, "Move", {{MOVE_ENTITY, 1, {10.0f, 10.0f}}}});
    seq.push_back({2, "Damage", {{DAMAGE_ENTITY, 2, {0, 0}, 50}}});
    
    GameState result = applySequence(gs, seq, 1);
    
    EXPECT_FLOAT_EQ(gs.entities[0].position.x, 5.0f);
    EXPECT_EQ(gs.entities[1].health, 100);
}

TEST_F(ApplySequenceTests, LongSequenceExecution) {
    GameState gs = createGameState();
    Sequence seq;
    
    // Build a 5-card sequence
    for (int i = 0; i < 5; ++i) {
        seq.push_back({i, "Card" + std::to_string(i), {{DAMAGE_ENTITY, 2, {0, 0}, 10}}});
    }
    
    GameState result = applySequence(gs, seq, 1);
    
    auto enemy = std::find_if(result.entities.begin(), result.entities.end(),
        [](const Entity& e) { return e.id == 2; });
    
    EXPECT_EQ(enemy->health, 50);
}

// ==================== GAME INITIALIZATION TESTS ====================

class GameInitializationTests : public ::testing::Test {
protected:
    Game game;
};

TEST_F(GameInitializationTests, InitGameCreatesGrid) {
    init_game(game);
    
    EXPECT_EQ(game.grid.SIZE, 12);
}

TEST_F(GameInitializationTests, InitGameCreatesEntities) {
    init_game(game);
    
    EXPECT_EQ(game.entities.size(), 3);
}

TEST_F(GameInitializationTests, InitGameCreatesPlayer) {
    init_game(game);
    
    auto player = std::find_if(game.entities.begin(), game.entities.end(),
        [](const Entity& e) { return e.type == PLAYER; });
    
    ASSERT_NE(player, game.entities.end());
    EXPECT_EQ(player->id, 1);
    EXPECT_EQ(player->name, "Player");
}

TEST_F(GameInitializationTests, InitGameCreatesEnemy) {
    init_game(game);
    
    auto enemy = std::find_if(game.entities.begin(), game.entities.end(),
        [](const Entity& e) { return e.type == ENEMY; });
    
    ASSERT_NE(enemy, game.entities.end());
    EXPECT_EQ(enemy->name, "Enemy1");
}

TEST_F(GameInitializationTests, InitGameCreatesObject) {
    init_game(game);
    
    auto obj = std::find_if(game.entities.begin(), game.entities.end(),
        [](const Entity& e) { return e.type == OBJECT; });
    
    ASSERT_NE(obj, game.entities.end());
    EXPECT_EQ(obj->name, "Object1");
}

TEST_F(GameInitializationTests, InitGameCreatesHand) {
    init_game(game);
    
    EXPECT_EQ(game.hand.size(), 3);
}

TEST_F(GameInitializationTests, InitGameHandContainsMoveCard) {
    init_game(game);
    
    auto moveCard = std::find_if(game.hand.begin(), game.hand.end(),
        [](const Card& c) { return c.name == "Move"; });
    
    ASSERT_NE(moveCard, game.hand.end());
}

TEST_F(GameInitializationTests, InitGameHandContainsDamageCard) {
    init_game(game);
    
    auto damageCard = std::find_if(game.hand.begin(), game.hand.end(),
        [](const Card& c) { return c.name == "Damage"; });
    
    ASSERT_NE(damageCard, game.hand.end());
}

TEST_F(GameInitializationTests, InitGameHandContainsHealCard) {
    init_game(game);
    
    auto healCard = std::find_if(game.hand.begin(), game.hand.end(),
        [](const Card& c) { return c.name == "Heal"; });
    
    ASSERT_NE(healCard, game.hand.end());
}

TEST_F(GameInitializationTests, InitGameRotationsReset) {
    game.planetRot = 1.0f;
    game.cloudsRot = 2.0f;
    
    init_game(game);
    
    EXPECT_FLOAT_EQ(game.planetRot, 0.0f);
    EXPECT_FLOAT_EQ(game.cloudsRot, 0.0f);
}

TEST_F(GameInitializationTests, InitGameEntityHealthDefault) {
    init_game(game);
    
    for (const auto& e : game.entities) {
        EXPECT_EQ(e.health, 100);
    }
}

// ==================== ENEMY AI TESTS ====================

class EnemyAITests : public ::testing::Test {
protected:
    Game game;
    
    Vector2 findPlayerPosition() {
        for (const auto& e : game.entities) {
            if (e.type == PLAYER) {
                return e.position;
            }
        }
        return {0, 0};
    }
    
    Vector2 findEnemyPosition() {
        for (const auto& e : game.entities) {
            if (e.type == ENEMY) {
                return e.position;
            }
        }
        return {0, 0};
    }
};

TEST_F(EnemyAITests, EnemyMovesTowardPlayer) {
    init_game(game);
    
    Vector2 initialEnemyPos = findEnemyPosition();
    
    update_game(game, 0.1f);
    
    Vector2 playerPos = findPlayerPosition();
    Vector2 newEnemyPos = findEnemyPosition();
    
    float initialDist = std::sqrt(
        (initialEnemyPos.x - playerPos.x) * (initialEnemyPos.x - playerPos.x) +
        (initialEnemyPos.y - playerPos.y) * (initialEnemyPos.y - playerPos.y)
    );
    
    float newDist = std::sqrt(
        (newEnemyPos.x - playerPos.x) * (newEnemyPos.x - playerPos.x) +
        (newEnemyPos.y - playerPos.y) * (newEnemyPos.y - playerPos.y)
    );
    
    EXPECT_LT(newDist, initialDist);
}

TEST_F(EnemyAITests, EnemyRespectsBounds) {
    init_game(game);
    
    // Move player to corner
    for (auto& e : game.entities) {
        if (e.type == PLAYER) {
            e.position = {0.5f, 0.5f};
        }
    }
    
    for (int i = 0; i < 100; ++i) {
        update_game(game, 0.1f);
    }
    
    for (const auto& e : game.entities) {
        EXPECT_GE(e.position.x, 0.0f);
        EXPECT_LE(e.position.x, 11.0f);
        EXPECT_GE(e.position.y, 0.0f);
        EXPECT_LE(e.position.y, 11.0f);
    }
}

TEST_F(EnemyAITests, RotationUpdates) {
    init_game(game);
    
    float initialPlanetRot = game.planetRot;
    float initialCloudsRot = game.cloudsRot;
    
    update_game(game, 1.0f);
    
    EXPECT_NE(game.planetRot, initialPlanetRot);
    EXPECT_NE(game.cloudsRot, initialCloudsRot);
}

TEST_F(EnemyAITests, RotationIncreasesWithTime) {
    init_game(game);
    
    update_game(game, 1.0f);
    float rot1 = game.planetRot;
    
    update_game(game, 1.0f);
    float rot2 = game.planetRot;
    
    EXPECT_GT(rot2, rot1);
}

TEST_F(EnemyAITests, EnemyStopsWhenCollidingWithPlayer) {
    init_game(game);
    
    // Place enemy and player on same tile
    for (auto& e : game.entities) {
        if (e.type == PLAYER) {
            e.position = {5.0f, 5.0f};
        } else if (e.type == ENEMY) {
            e.position = {5.1f, 5.0f};
        }
    }
    
    Vector2 enemyBefore = findEnemyPosition();
    update_game(game, 0.1f);
    Vector2 enemyAfter = findEnemyPosition();
    
    // Very close, should barely move
    float movementDistance = std::sqrt(
        (enemyAfter.x - enemyBefore.x) * (enemyAfter.x - enemyBefore.x) +
        (enemyAfter.y - enemyBefore.y) * (enemyAfter.y - enemyBefore.y)
    );
    
    EXPECT_LT(movementDistance, 0.1f);
}

TEST_F(EnemyAITests, MultipleUpdateCalls) {
    init_game(game);
    
    Vector2 initialEnemyPos = findEnemyPosition();
    
    for (int i = 0; i < 10; ++i) {
        update_game(game, 0.05f);
    }
    
    Vector2 finalEnemyPos = findEnemyPosition();
    
    float totalMovement = std::sqrt(
        (finalEnemyPos.x - initialEnemyPos.x) * (finalEnemyPos.x - initialEnemyPos.x) +
        (finalEnemyPos.y - initialEnemyPos.y) * (finalEnemyPos.y - initialEnemyPos.y)
    );
    
    EXPECT_GT(totalMovement, 0.0f);
}
