#include "game.h"
#include "ui.h"
#include <cmath>

void init_game(Game& game) {
    game.grid = Grid();

    game.entities.clear();
    Entity player = {1, PLAYER, {5.0f, 5.0f}, "Player"};
    Entity enemy  = {2, ENEMY,  {2.0f, 2.0f}, "Enemy1"};
    Entity obj    = {3, OBJECT, {8.0f, 8.0f}, "Object1"};
    game.entities.push_back(player);
    game.entities.push_back(enemy);
    game.entities.push_back(obj);

    game.hand.clear();
    Card moveCard   = {1, "Move",   {{MOVE_ENTITY, 1, {7.0f, 7.0f}}}};
    Card damageCard = {2, "Damage", {{DAMAGE_ENTITY, 2, {}, 20}}};
    Card healCard   = {3, "Heal",   {{HEAL_ENTITY, 1, {}, 0, 15}}};
    game.hand.push_back(moveCard);
    game.hand.push_back(damageCard);
    game.hand.push_back(healCard);

    game.planetRot = 0.0f;
    game.cloudsRot = 0.0f;
}

void update_game(Game& game, float dt) {
    // Simple enemy pursuit of player
    Vector2 playerPos = {0, 0};
    for (const auto& e : game.entities) {
        if (e.type == PLAYER) {
            playerPos = e.position;
            break;
        }
    }
    for (auto& e : game.entities) {
        if (e.type == ENEMY) {
            Vector2 dir = {playerPos.x - e.position.x, playerPos.y - e.position.y};
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len > 0.1f) {
                dir.x /= len;
                dir.y /= len;
                float speed = 0.6f; // grid units per second
                e.position.x += dir.x * speed * dt;
                e.position.y += dir.y * speed * dt;
                // Clamp to grid
                if (e.position.x < 0) e.position.x = 0;
                if (e.position.x > 11) e.position.x = 11;
                if (e.position.y < 0) e.position.y = 0;
                if (e.position.y > 11) e.position.y = 11;
            }
        }
    }

    // Spin planet/clouds
    game.planetRot += 0.6f * dt * 60.0f * 0.01f; // approximate to previous rate
    game.cloudsRot += 1.2f * dt * 60.0f * 0.01f;
}

void handle_input(Game& game, const Platform& platform) {
    // Keyboard input for triggering sample sequence (debug)
    if (platform.input->IsKeyPressed(KEY_ONE)) {
        if (game.hand.size() >= 2) {
            Sequence sampleSeq;
            sampleSeq.push_back(game.hand[0]); // e.g., Move
            sampleSeq.push_back(game.hand[1]); // e.g., Damage

            GameState gs{game.grid, game.entities};
            TraceLog(LOG_INFO, "Applying sample sequence via keyboard");
            gs = applySequence(gs, sampleSeq, 1);
            game.grid = gs.grid;
            game.entities = gs.entities;
            TraceLog(LOG_INFO, "Sample sequence applied");
        }
    }
}

void handle_ui_actions(Game& game, const UiActions& actions) {
    if (actions.playSequence) {
        GameState gs{game.grid, game.entities};
        TraceLog(LOG_INFO, "Applying sequence via UI");
        gs = applySequence(gs, game.currentSeq, 1); // Apply to player 1
        game.grid = gs.grid;
        game.entities = gs.entities;
        game.currentSeq.clear();
        TraceLog(LOG_INFO, "Sequence applied, cleared");
    }
    if (actions.clearSequence) {
        game.currentSeq.clear();
    }
}
