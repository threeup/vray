#include "boss.h"
#include "bossStartupState.h"
#include "game.h"
#include "ui.h"
#include <raylib.h>

Boss::Boss() : machine_("Boss") {}

void Boss::begin(Game& game) {
    auto startupState = std::make_unique<BossStartupState>();
    machine_.begin(game, std::move(startupState));
}

void Boss::update(Game& game, const CardActions& actions, float dt) {
    if (actions.playSequence) {
        TraceLog(LOG_INFO, "[Boss] UI action: playSequence=true in state=%s", getCurrentStateName());
    }
    machine_.update(game, actions, dt);
}
