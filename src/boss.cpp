#include "boss.h"
#include "game.h"
#include "ui.h"
#include <algorithm>
#include <sstream>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <random>

namespace {

std::vector<int> collect_mech_ids(const std::vector<Entity>& entities, EntityType type) {
    std::vector<int> ids;
    for (const auto& e : entities) {
        if (e.type == type) {
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

Boss::Boss() = default;

namespace {
void log_phase(const char* msg) {
    TraceLog(LOG_INFO, "[BossPhase] %s", msg);
}

void log_move(int id, EntityType type, const Vector2& from, const Vector2& to) {
    const char* who = (type == PLAYER) ? "Player" : (type == ENEMY ? "Enemy" : "Other");
    TraceLog(LOG_INFO, "[Move] %s %d: (%.0f,%.0f) -> (%.0f,%.0f)", who, id, from.x, from.y, to.x, to.y);
}
}

void Boss::begin(Game& game) {
    phase_ = Phase::PlayerSelect;
    phaseTime_ = 0.0f;
    npcPlanReady_ = false;
    playQueued_ = false;
    playerPlanLocked_ = false;
    playerPlan_.assignments.clear();
    npcPlan_.assignments.clear();
    pendingPlayer_.clear();
    pendingNpc_.clear();
    playSubphase_ = 0;
    playSubphaseTime_ = 0.0f;
    game.currentPlan.assignments.clear();
    game.hand.resetUsage();
    phaseTime_ = 0.0f;
    log_phase("Begin PlayerSelect");
}

void Boss::processUi(Game& game, const UiActions& actions) {
    if (phase_ != Phase::PlayerSelect) {
        return;
    }

    // Allow plan edits but prevent auto-resolve; only Boss advances phases.
    handle_ui_actions(game, actions, false);

    if (actions.playSequence) {
        std::vector<int> playerMechs = collect_mech_ids(game.entities, PLAYER);
        std::string err;
        if (!game.currentPlan.validate(game.hand.cards, playerMechs, &err)) {
            TraceLog(LOG_WARNING, "Player plan invalid: %s", err.c_str());
            return;
        }
        playerPlan_ = game.currentPlan;
        playerPlanLocked_ = true;
        enterPhase(Phase::NpcSelect);
        npcPlanReady_ = false;
        playQueued_ = false;
        game.hand.resetUsage(); // Free hand usage for NPC plan selection
    }
}

void Boss::buildNpcPlan(Game& game) {
    std::vector<int> mechIds = collect_mech_ids(game.entities, ENEMY);
    if (mechIds.empty()) {
        mechIds = collect_mech_ids(game.entities, PLAYER); // fallback
    }

    begin_turn(game); // reset usage for NPC selection
    uint32_t seed = static_cast<uint32_t>(1000 + game.turnNumber);
    std::mt19937 rng(seed);
    npcPlan_.assignments.clear();

    // Debug: log which mechs we plan for
    {
        std::ostringstream oss;
        oss << "[NPCPlan] Mechs:";
        for (int id : mechIds) oss << " " << id;
        TraceLog(LOG_INFO, "%s", oss.str().c_str());
    }

    auto pickCardId = [&]() -> int {
        if (game.hand.cards.empty()) return -1;
        std::uniform_int_distribution<size_t> dist(0, game.hand.cards.size() - 1);
        return game.hand.cards[dist(rng)].id;
    };

    for (int mechId : mechIds) {
        int cardId = pickCardId();
        if (cardId == -1) continue;
        bool useMirror = std::bernoulli_distribution(0.5f)(rng);
        npcPlan_.assignments.push_back({mechId, cardId, useMirror});
        TraceLog(LOG_INFO, "[NPCPlan] mech %d -> card %d mirror %d", mechId, cardId, useMirror ? 1 : 0);
    }

    if (npcPlan_.assignments.size() < mechIds.size()) {
        TraceLog(LOG_WARNING, "[NPCPlan] Only %zu/%zu assignments generated", npcPlan_.assignments.size(), mechIds.size());
    }

    std::string err;
    if (!npcPlan_.validate(game.hand.cards, mechIds, &err)) {
        TraceLog(LOG_WARNING, "NPC plan invalid: %s", err.c_str());
    }

    TraceLog(LOG_INFO, "[NPCPlan] Final assignments: %s", format_plan(npcPlan_, game.hand.cards).c_str());

    game.lastAiPlan = npcPlan_;
    game.lastAiPlanText = format_plan(npcPlan_, game.hand.cards);
}

void Boss::executePlay(Game& game) {
    begin_turn(game);

    pendingPlayer_ = playerPlan_.assignments;
    pendingNpc_ = npcPlan_.assignments;
    playSubphase_ = 0;
    playSubphaseTime_ = 0.0f;
}

bool Boss::applyAssignment(Game& game, const PlanAssignment& a) {
    const Card* card = nullptr;
    for (const auto& c : game.hand.cards) {
        if (c.id == a.cardId) { card = &c; break; }
    }
    if (!card) return true; // nothing to do

    GameState before{game.grid, game.entities};
    GameState after = applyCard(before, *card, a.mechId, a.useMirror);

    // Find actor
    auto findEntityPos = [](const std::vector<Entity>& ents, int id, Vector2& out) -> bool {
        for (const auto& e : ents) {
            if (e.id == id) { out = e.position; return true; }
        }
        return false;
    };

    bool executed = true;
    bool moved = false;
    if (card->type == CardType::Move) {
        Vector2 beforePos{}, afterPos{};
        bool haveBefore = findEntityPos(before.entities, a.mechId, beforePos);
        bool haveAfter = findEntityPos(after.entities, a.mechId, afterPos);
        if (haveBefore && haveAfter) {
            if (std::round(beforePos.x) == std::round(afterPos.x) && std::round(beforePos.y) == std::round(afterPos.y)) {
                executed = false; // blocked
            } else {
                moved = true;
            }
        }
    }

    if (executed) {
        game.grid = after.grid;
        game.entities = after.entities;
    }

    if (moved && executed) {
        EntityType t = PLAYER;
        for (const auto& e : game.entities) {
            if (e.id == a.mechId) { t = e.type; break; }
        }
        Vector2 beforePos{}, afterPos{};
        findEntityPos(before.entities, a.mechId, beforePos);
        findEntityPos(game.entities, a.mechId, afterPos);
        log_move(a.mechId, t, beforePos, afterPos);
    }

    return executed;
}

void Boss::runPlaySubphase(Game& game) {
    // Try player assignments first, then NPC
    auto processList = [&](std::vector<PlanAssignment>& list) {
        std::vector<PlanAssignment> blocked;
        for (const auto& a : list) {
            bool ok = applyAssignment(game, a);
            if (!ok) {
                blocked.push_back(a);
            }
        }
        list.swap(blocked);
    };

    processList(pendingPlayer_);
    processList(pendingNpc_);

    // Log remaining after subphase for debugging
    if (!pendingPlayer_.empty() || !pendingNpc_.empty()) {
        std::ostringstream oss;
        oss << "[PlaySubphase] remaining P:" << pendingPlayer_.size() << " N:" << pendingNpc_.size();
        TraceLog(LOG_INFO, "%s", oss.str().c_str());
    }

    // After last subphase finishes, logging and cleanup happen in update()
}

void Boss::enterPhase(Phase p) {
    phase_ = p;
    phaseTime_ = 0.0f;
    switch (p) {
    case Phase::PlayerSelect: log_phase("Enter PlayerSelect"); break;
    case Phase::NpcSelect: log_phase("Enter NPCSelect"); break;
    case Phase::Play: log_phase("Enter Play"); break;
    }
}

void Boss::update(Game& game, float dt) {
    phaseTime_ += dt;

    switch (phase_) {
    case Phase::PlayerSelect:
        // Waiting for user to lock plan
        break;
    case Phase::NpcSelect:
        if (!playerPlanLocked_) {
            // Safety: don't progress if somehow entered without a locked plan
            enterPhase(Phase::PlayerSelect);
            break;
        }
        if (!npcPlanReady_) {
            buildNpcPlan(game);
            npcPlanReady_ = true;
        }
        if (phaseTime_ >= 1.0f) {
            enterPhase(Phase::Play);
            playQueued_ = true;
        }
        break;
    case Phase::Play:
        if (playQueued_) {
            executePlay(game);
            playQueued_ = false;
        }
        // Execute timed subphases of 0.75s each
        playSubphaseTime_ += dt;
        const float subphaseDuration = 0.75f;
        while (playSubphase_ < 3 && playSubphaseTime_ >= subphaseDuration) {
            runPlaySubphase(game);
            playSubphase_++;
            playSubphaseTime_ -= subphaseDuration;
        }

        if (playSubphase_ >= 3) {
            advance_turn(game);

            // Reset for next round
            game.currentPlan.assignments.clear();
            game.hand.resetUsage();
            playerPlan_.assignments.clear();
            npcPlan_.assignments.clear();
            pendingPlayer_.clear();
            pendingNpc_.clear();
            playerPlanLocked_ = false;
            npcPlanReady_ = false;
            playQueued_ = false;
            enterPhase(Phase::PlayerSelect);
        }
        break;
    }
}
