#include "game.h"
#include "ui.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include "raylib.h" // TraceLog

namespace {

void log_spawn(const Entity& e) {
    const char* who = (e.type == PLAYER) ? "Player" : (e.type == ENEMY ? "Enemy" : "Object");
    TraceLog(LOG_INFO, "[Spawn] %s %d at (%.0f,%.0f)", who, e.id, e.position.x, e.position.y);
}

std::vector<int> collect_enemy_mech_ids(const std::vector<Entity>& entities) {
    std::vector<int> ids;
    for (const auto& e : entities) {
        if (e.type == ENEMY) {
            ids.push_back(e.id);
        }
    }
    return ids;
}

std::vector<int> collect_player_mech_ids(const std::vector<Entity>& entities) {
    std::vector<int> ids;
    for (const auto& e : entities) {
        if (e.type == PLAYER) {
            ids.push_back(e.id);
        }
    }
    return ids;
}

std::string format_plan(const TurnPlan& plan, const std::vector<Card>& hand) {
    auto findName = [&hand](int cardId) -> std::string {
        for (const auto& c : hand) {
            if (c.id == cardId) return c.name;
        }
        return "?";
    };
    std::ostringstream oss;
    for (size_t i = 0; i < plan.assignments.size(); ++i) {
        const auto& a = plan.assignments[i];
        if (i > 0) oss << " | ";
        oss << "M" << a.mechId << ":" << findName(a.cardId);
        if (a.useMirror) oss << "(M)";
    }
    return oss.str();
}

} // namespace

void begin_turn(Game& game) {
    game.hand.resetUsage();
}

void advance_turn(Game& game) {
    game.turnNumber++;
}

void init_game(Game& game) {
    game.grid = Grid();

    game.entities.clear();
    // Align with world spawn tiles: heroes on bottom-left cluster
    Entity p1 = {1, PLAYER, {1.0f, 6.0f}, "Mech A"};
    Entity p2 = {2, PLAYER, {2.0f, 6.0f}, "Mech B"};
    Entity p3 = {3, PLAYER, {1.0f, 5.0f}, "Mech C"};
    p1.facing = Facing::North;
    p2.facing = Facing::North;
    p3.facing = Facing::North;
    // Enemies on bottom-right cluster
    Entity enemy1  = {4, ENEMY,  {6.0f, 6.0f}, "Enemy1"};
    enemy1.facing = Facing::South;
    Entity enemy2  = {5, ENEMY,  {5.0f, 6.0f}, "Enemy2"};
    enemy2.facing = Facing::South;
    Entity enemy3  = {6, ENEMY,  {6.0f, 5.0f}, "Enemy3"};
    enemy3.facing = Facing::South;
    Entity obj    = {10, OBJECT, {8.0f, 8.0f}, "Object1"};
    game.entities.push_back(p1);
    game.entities.push_back(p2);
    game.entities.push_back(p3);
    game.entities.push_back(enemy1);
    game.entities.push_back(enemy2);
    game.entities.push_back(enemy3);
    game.entities.push_back(obj);

    for (const auto& e : game.entities) {
        log_spawn(e);
    }

    game.hand.clear();
    auto makeMoveCard = [](int id, const char* name, MoveVector mv) {
        Card c;
        c.id = id;
        c.name = name;
        c.type = CardType::Move;
        c.effect = {CardType::Move, mv};
        c.mirroredEffect = mirrorEffect(c.effect);
        return c;
    };

    game.hand.addCard(makeMoveCard(1, "Advance", {1, 0}));
    game.hand.addCard(makeMoveCard(2, "StrafeLeft", {0, -1}));
    game.hand.addCard(makeMoveCard(3, "StrafeRight", {0, 1}));
    game.hand.addCard(makeMoveCard(4, "Lunge", {2, 0}));
    game.hand.addCard(makeMoveCard(5, "Retreat", {-1, 0}));
    game.hand.addCard(makeMoveCard(6, "HookLeft", {1, -1}));

    game.turnNumber = 1;
    begin_turn(game);

    game.pendingCardId = -1;
    game.pendingMirror = false;

    game.planetRot = 0.0f;
    game.cloudsRot = 0.0f;
}

void update_game(Game& game, float dt) {
    // Spin planet/clouds
    game.planetRot += 0.6f * dt * 60.0f * 0.01f; // approximate to previous rate
    game.cloudsRot += 1.2f * dt * 60.0f * 0.01f;
}

void handle_input(Game& game, const Platform& platform) {
    // Keyboard input for triggering a sample round (debug)
    if (platform.input->IsKeyPressed(KEY_ONE)) {
        if (game.hand.cards.size() >= 2) {
            TurnPlan plan;
            auto players = collect_player_mech_ids(game.entities);
            int mechId = players.empty() ? 1 : players.front();
            plan.assignments.push_back({mechId, game.hand.cards[0].id, false});
            plan.assignments.push_back({mechId, game.hand.cards[1].id, true});
            resolve_round(game, plan);
        }
    }

    // Keyboard input for triggering AI random turn (debug)
    if (platform.input->IsKeyPressed(KEY_TWO)) {
        execute_ai_random_turn(game, 42u);
        advance_turn(game);
    }
}

void handle_ui_actions(Game& game, const CardActions& actions, bool allowResolve) {
    // Step 1: select a card (no assignment yet)
    if (actions.selectCardId != -1) {
        const Card* cardPtr = nullptr;
        for (const auto& c : game.hand.cards) {
            if (c.id == actions.selectCardId) { cardPtr = &c; break; }
        }
        if (cardPtr) {
            game.pendingCardId = cardPtr->id;
            game.pendingMirror = actions.mirrorNext;
        }
    }

    // Step 2: assign selected card to a mech
    if (actions.assignCardToMech != -1 && game.pendingCardId != -1) {
        int mechId = actions.assignCardToMech;
        const Card* cardPtr = nullptr;
        for (const auto& c : game.hand.cards) {
            if (c.id == game.pendingCardId) { cardPtr = &c; break; }
        }
        if (cardPtr) {
            int existingIdx = -1;
            for (size_t i = 0; i < game.currentPlan.assignments.size(); ++i) {
                if (game.currentPlan.assignments[i].mechId == mechId) {
                    existingIdx = static_cast<int>(i);
                    break;
                }
            }

            bool planFull = game.currentPlan.assignments.size() >= 3 && existingIdx == -1;
            if (planFull) {
                TraceLog(LOG_WARNING, "Plan already has 3 mech assignments; ignoring selection");
            } else {
                int previousCardId = -1;
                if (existingIdx != -1) {
                    previousCardId = game.currentPlan.assignments[existingIdx].cardId;
                    game.hand.unmarkUsed(previousCardId);
                }

                if (game.hand.markUsed(cardPtr->id)) {
                    PlanAssignment updated{mechId, cardPtr->id, game.pendingMirror};
                    if (existingIdx != -1) {
                        game.currentPlan.assignments[existingIdx] = updated;
                    } else {
                        game.currentPlan.assignments.push_back(updated);
                    }
                    game.lastSelectedMechId = mechId;
                    game.pendingCardId = -1; // clear pending after assignment
                } else if (existingIdx != -1 && previousCardId != -1) {
                    game.hand.markUsed(previousCardId);
                    game.currentPlan.assignments[existingIdx].cardId = previousCardId;
                }
            }
        }
    }

    // Undo last assignment
    if (actions.undoLast && !game.currentPlan.assignments.empty()) {
        PlanAssignment last = game.currentPlan.assignments.back();
        game.currentPlan.assignments.pop_back();
        game.hand.unmarkUsed(last.cardId);
    }

    // Remove specific mech assignment
    if (actions.removeAssignmentMechId != -1) {
        auto it = std::find_if(game.currentPlan.assignments.begin(), game.currentPlan.assignments.end(),
            [&actions](const PlanAssignment& a) { return a.mechId == actions.removeAssignmentMechId; });
        if (it != game.currentPlan.assignments.end()) {
            game.hand.unmarkUsed(it->cardId);
            game.currentPlan.assignments.erase(it);
        }
    }

    if (allowResolve && actions.playSequence) {
        resolve_round(game, game.currentPlan);
    }
    if (actions.clearSequence) {
        game.currentPlan.assignments.clear();
        game.hand.resetUsage();
        game.pendingCardId = -1;
        game.pendingMirror = false;
    }
}

void execute_ai_random_turn(Game& game, uint32_t seed, float mirrorChance) {
    begin_turn(game);
    std::vector<int> mechIds = collect_enemy_mech_ids(game.entities);
    if (mechIds.empty()) {
        mechIds.push_back(1); // fallback to player
    }

    TurnPlan plan = buildRandomPlan(mechIds, game.hand, seed, mirrorChance);
    std::string error;
    if (!plan.validate(game.hand.cards, mechIds, &error)) {
        TraceLog(LOG_WARNING, "AI plan invalid: %s", error.c_str());
        return;
    }

    GameState gs{game.grid, game.entities};
    gs = plan.apply(gs, game.hand.cards, game.grid);
    game.grid = gs.grid;
    game.entities = gs.entities;
    game.lastAiPlan = plan;
    game.lastAiPlanText = format_plan(plan, game.hand.cards);
    begin_turn(game);
    TraceLog(LOG_INFO, "AI random turn executed (%zu assignments)", plan.assignments.size());
}

void resolve_round(Game& game, const TurnPlan& playerPlan, uint32_t seed, float mirrorChance) {
    begin_turn(game); // player turn usage reset

    std::vector<int> playerMechs = collect_player_mech_ids(game.entities);

    std::string playerErr;
    if (!playerPlan.validate(game.hand.cards, playerMechs, &playerErr)) {
        TraceLog(LOG_WARNING, "Player plan invalid: %s", playerErr.c_str());
    } else {
        GameState gs{game.grid, game.entities};
        TraceLog(LOG_INFO, "Player applying plan (%zu cards)", playerPlan.assignments.size());
        gs = playerPlan.apply(gs, game.hand.cards, game.grid);
        game.grid = gs.grid;
        game.entities = gs.entities;
    }

    // Clear player plan and hand usage to allow AI use if sharing hand
    game.currentPlan.assignments.clear();
    game.hand.resetUsage();

    uint32_t planSeed = seed ? seed : static_cast<uint32_t>(1000 + game.turnNumber);
    TraceLog(LOG_INFO, "Executing AI round with seed %u", planSeed);
    execute_ai_random_turn(game, planSeed, mirrorChance);

    advance_turn(game);
}
