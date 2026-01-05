#include "bossNpcSelectState.h"
#include "bossPlayState.h"
#include "game.h"
#include "ui.h"
#include "card.h"
#include <vector>
#include <random>
#include <sstream>
#include <algorithm>

bool BossNpcSelectState::canEnter(Game& game) {
    // Can enter if player plan is locked (has assignments)
    return !game.currentPlan.assignments.empty();
}

bool BossNpcSelectState::canExit(Game& game) {
    // Can exit once NPC plan is generated
    return npcPlanReady_;
}

void BossNpcSelectState::enter(Game& game) {
    npcPlanReady_ = false;
    elapsed_ = 0.0f;
    buildNpcPlan(game);
    npcPlanReady_ = true;
    TraceLog(LOG_INFO, "[NpcSelect] Entering NpcSelect state, generated plan: %s", game.lastAiPlanText.c_str());
}

void BossNpcSelectState::exit(Game& game) {
    TraceLog(LOG_DEBUG, "[NpcSelect] Exiting NpcSelect state");
}

std::unique_ptr<BossState> BossNpcSelectState::update(Game& game, const CardActions& actions, float dt) {
    elapsed_ += dt;

    // NPC plan is generated immediately on enter, check if we can transition
    if (canExit(game)) {
        TraceLog(LOG_INFO, "[NpcSelect::NPC_PLAN_READY] NPC plan ready, requesting transition to Play");
        auto nextState = std::make_unique<BossPlayState>();
        if (nextState->canEnter(game)) {
            return nextState;
        } else {
            TraceLog(LOG_WARNING, "[NpcSelect] Play state rejected entry: canEnter() returned false");
            return nullptr;
        }
    }

    return nullptr;  // Stay in current state
}

void BossNpcSelectState::buildNpcPlan(Game& game) {
    // Collect enemy mech IDs
    std::vector<int> enemyMechIds;
    for (const auto& entity : game.entities) {
        if (entity.type == ENEMY) {
            enemyMechIds.push_back(entity.id);
            if (enemyMechIds.size() >= 3) break;
        }
    }

    if (enemyMechIds.empty()) {
        // Fallback: use player mechs if no enemies
        for (const auto& entity : game.entities) {
            if (entity.type == PLAYER) {
                enemyMechIds.push_back(entity.id);
                if (enemyMechIds.size() >= 3) break;
            }
        }
    }

    // Reset NPC plan
    game.lastAiPlan.assignments.clear();

    // Use deterministic seed based on turn number
    uint32_t seed = static_cast<uint32_t>(1000 + game.turnNumber);
    std::mt19937 rng(seed);

    // Debug: log which mechs we plan for
    {
        std::ostringstream oss;
        oss << "[NpcSelect] Mechs:";
        for (int id : enemyMechIds) oss << " " << id;
        TraceLog(LOG_INFO, "%s", oss.str().c_str());
    }

    // Pick random cards for each mech
    auto pickCardId = [&]() -> int {
        if (game.hand.cards.empty()) return -1;
        std::uniform_int_distribution<size_t> dist(0, game.hand.cards.size() - 1);
        return game.hand.cards[dist(rng)].id;
    };

    for (int mechId : enemyMechIds) {
        int cardId = pickCardId();
        if (cardId == -1) continue;
        bool useMirror = std::bernoulli_distribution(0.5f)(rng);
        game.lastAiPlan.assignments.push_back({mechId, cardId, useMirror});
        TraceLog(LOG_INFO, "[NpcSelect] mech %d -> card %d mirror %d", mechId, cardId, useMirror ? 1 : 0);
    }

    // Validate NPC plan
    std::string err;
    if (!game.lastAiPlan.validate(game.hand.cards, enemyMechIds, &err)) {
        TraceLog(LOG_WARNING, "[NpcSelect] NPC plan invalid: %s", err.c_str());
    }

    // Format for display
    {
        std::ostringstream oss;
        for (size_t i = 0; i < game.lastAiPlan.assignments.size(); ++i) {
            const auto& a = game.lastAiPlan.assignments[i];
            std::string cardName = "?";
            for (const auto& c : game.hand.cards) {
                if (c.id == a.cardId) {
                    cardName = c.name;
                    break;
                }
            }
            if (i > 0) oss << " | ";
            oss << "M" << a.mechId << ":" << cardName;
            if (a.useMirror) oss << "(M)";
        }
        game.lastAiPlanText = oss.str();
        TraceLog(LOG_INFO, "[NpcSelect] Final assignments: %s", game.lastAiPlanText.c_str());
    }

    npcPlanReady_ = true;
}
