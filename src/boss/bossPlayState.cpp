#include "bossPlayState.h"
#include "bossCardSelectState.h"
#include "bossEndGameState.h"
#include "game.h"
#include "ui.h"
#include "card.h"
#include "entity.h"
#include "world/world.h"
#include <algorithm>

bool BossPlayState::canEnter(Game& game) {
    // Can enter if both plans are ready
    return !game.currentPlan.assignments.empty() && !game.lastAiPlan.assignments.empty();
}

bool BossPlayState::canExit(Game& game) {
    // Can exit when all subphases complete
    return pendingPlayer_.empty() && pendingNpc_.empty();
}

void BossPlayState::enter(Game& game) {
    TraceLog(LOG_INFO, "[Play] Entering Play state");
    TraceLog(LOG_INFO, "[Play] Player assignments: %zu, NPC assignments: %zu",
             game.currentPlan.assignments.size(), game.lastAiPlan.assignments.size());
    
    // Copy plans to pending execution
    pendingPlayer_ = game.currentPlan.assignments;
    pendingNpc_ = game.lastAiPlan.assignments;
    playSubphase_ = 0;
    playSubphaseTime_ = 0.0f;

    if (pendingPlayer_.empty() && pendingNpc_.empty()) {
        TraceLog(LOG_WARNING, "[Play] No assignments to execute");
    }
}

void BossPlayState::exit(Game& game) {
    TraceLog(LOG_DEBUG, "[Play] Exiting Play state");
    
    // Clean up for next round
    game.turnNumber++;
    game.hand.resetUsage();
    game.currentPlan.assignments.clear();
    game.lastAiPlan.assignments.clear();
}

std::unique_ptr<BossState> BossPlayState::update(Game& game, const CardActions& actions, float dt) {
    playSubphaseTime_ += dt;

    // Execute one subphase per SUBPHASE_DURATION
    if (playSubphaseTime_ >= SUBPHASE_DURATION) {
        playSubphaseTime_ -= SUBPHASE_DURATION;
        runPlaySubphase(game);
    }

    // Check if play phase is complete
    if (canExit(game)) {
        TraceLog(LOG_INFO, "[Play::PLAY_COMPLETE] All assignments executed, requesting transition to CardSelect");
        // Decide next state: check for end game conditions
        // For now, always return to CardSelect for next round
        auto nextState = std::make_unique<BossCardSelectState>();
        if (nextState->canEnter(game)) {
            return nextState;
        } else {
            TraceLog(LOG_WARNING, "[Play] CardSelect state rejected entry: canEnter() returned false");
            return nullptr;
        }
    }

    return nullptr;  // Stay in current state
}

void BossPlayState::runPlaySubphase(Game& game) {
    // Apply one assignment from player, one from NPC
    bool playerDone = pendingPlayer_.empty();
    bool npcDone = pendingNpc_.empty();

    if (!playerDone) {
        PlanAssignment assignment = pendingPlayer_.front();
        pendingPlayer_.erase(pendingPlayer_.begin());
        applyAssignment(game, assignment);
    }

    if (!npcDone) {
        PlanAssignment assignment = pendingNpc_.front();
        pendingNpc_.erase(pendingNpc_.begin());
        applyAssignment(game, assignment);
    }

    TraceLog(LOG_INFO, "[PlaySubphase] remaining P:%zu N:%zu", pendingPlayer_.size(), pendingNpc_.size());
}

bool BossPlayState::applyAssignment(Game& game, const PlanAssignment& a) {
    // Find card
    const Card* card = nullptr;
    for (const auto& c : game.hand.cards) {
        if (c.id == a.cardId) {
            card = &c;
            break;
        }
    }
    if (!card) return true;

    // Record state before
    GameState before{game.grid, game.entities};
    GameState after = applyCard(before, *card, a.mechId, a.useMirror);

    // Check if move was blocked
    bool moved = false;
    if (card->type == CardType::Move) {
        Vector2 beforePos{}, afterPos{};
        auto findPos = [](const std::vector<Entity>& ents, int id, Vector2& out) -> bool {
            for (const auto& e : ents) {
                if (e.id == id) {
                    out = e.position;
                    return true;
                }
            }
            return false;
        };

        bool haveBefore = findPos(before.entities, a.mechId, beforePos);
        bool haveAfter = findPos(after.entities, a.mechId, afterPos);
        
        if (haveBefore && haveAfter) {
            if (std::round(beforePos.x) != std::round(afterPos.x) ||
                std::round(beforePos.y) != std::round(afterPos.y)) {
                moved = true;
            }
        }
    }

    // Apply the change
    game.grid = after.grid;
    game.entities = after.entities;

    if (moved) {
        Vector2 pos{};
        for (const auto& e : game.entities) {
            if (e.id == a.mechId) {
                pos = e.position;
                break;
            }
        }
        TraceLog(LOG_INFO, "[Move] Mech %d: -> (%.0f,%.0f)", a.mechId, pos.x, pos.y);
    }

    return true;
}
