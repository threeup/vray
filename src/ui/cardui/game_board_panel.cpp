#include "game_board_panel.h"
#include "raylib.h"
#include "game.h"
#include "entity.h"
#include <string>
#include <cmath>

/**
 * T_056: GameBoardPanel Implementation
 * 
 * Renders enemy and player mech areas with health bars.
 */

void GameBoardPanel::computeLayout(Rectangle boardRect) {
    const float dividerWidth = 4.0f;
    
    // Left side for enemies
    enemyArea = {
        boardRect.x,
        boardRect.y,
        (boardRect.width - dividerWidth) / 2.0f,
        boardRect.height
    };
    
    // Center divider
    centerLine = {
        boardRect.x + enemyArea.width,
        boardRect.y,
        dividerWidth,
        boardRect.height
    };
    
    // Right side for player
    playerArea = {
        boardRect.x + enemyArea.width + dividerWidth,
        boardRect.y,
        (boardRect.width - dividerWidth) / 2.0f,
        boardRect.height
    };
}

void GameBoardPanel_Draw(const Rectangle& rect, Game& game) {
    // Compute layout
    GameBoardPanel panel;
    panel.computeLayout(rect);
    
    // Draw background
    DrawRectangleRec(rect, Color{40, 50, 60, 100});
    DrawRectangleLinesEx(rect, 1, DARKGRAY);
    
    // Draw center divider
    DrawRectangleRec(panel.centerLine, Color{60, 60, 70, 150});
    
    // Collect enemy and player mechs
    std::vector<const Entity*> enemyMechs;
    std::vector<const Entity*> playerMechs;
    
    for (const auto& entity : game.entities) {
        if (entity.type == ENEMY) {
            enemyMechs.push_back(&entity);
        } else if (entity.type == PLAYER) {
            playerMechs.push_back(&entity);
        }
    }
    
    // Draw enemy mechs (left side)
    DrawText("ENEMIES", (int)(panel.enemyArea.x + 10), (int)(panel.enemyArea.y + 10), 16, Color{200, 100, 100, 220});
    
    float enemyYStart = panel.enemyArea.y + 35.0f;
    float enemyYSpacing = 80.0f;
    
    for (size_t i = 0; i < enemyMechs.size(); ++i) {
        const Entity* enemy = enemyMechs[i];
        float yPos = enemyYStart + i * enemyYSpacing;
        
        // Draw mech circle
        Color enemyColor = Color{200, 80, 80, 220};
        DrawCircle((int)(panel.enemyArea.x + 40), (int)(yPos + 15), 20.0f, enemyColor);
        DrawCircleLines((int)(panel.enemyArea.x + 40), (int)(yPos + 15), 20.0f, WHITE);
        
        // Draw mech ID
        std::string idStr = "E" + std::to_string(enemy->id);
        DrawText(idStr.c_str(), (int)(panel.enemyArea.x + 32), (int)(yPos + 8), 12, WHITE);
        
        // Draw health bar background
        Rectangle healthBarBg = {
            panel.enemyArea.x + 65.0f,
            yPos + 5.0f,
            panel.enemyArea.width - 75.0f,
            12.0f
        };
        DrawRectangleRec(healthBarBg, Color{40, 40, 40, 200});
        DrawRectangleLinesEx(healthBarBg, 1, DARKGRAY);
        
        // Draw health bar fill
        float healthPercent = std::max(0, std::min(100, enemy->health)) / 100.0f;
        Rectangle healthBarFill = {
            healthBarBg.x,
            healthBarBg.y,
            healthBarBg.width * healthPercent,
            healthBarBg.height
        };
        Color healthColor = healthPercent > 0.5f ? Color{80, 200, 80, 220} :
                           healthPercent > 0.25f ? Color{200, 180, 80, 220} :
                                                   Color{200, 80, 80, 220};
        DrawRectangleRec(healthBarFill, healthColor);
        
        // Draw health text
        std::string healthStr = std::to_string(enemy->health) + "/100";
        DrawText(healthStr.c_str(), (int)(healthBarBg.x + 5), (int)(yPos + 18), 10, WHITE);
    }
    
    // Draw player mechs (right side)
    DrawText("PLAYER", (int)(panel.playerArea.x + 10), (int)(panel.playerArea.y + 10), 16, Color{100, 140, 200, 220});
    
    float playerYStart = panel.playerArea.y + 35.0f;
    float playerYSpacing = 80.0f;
    
    for (size_t i = 0; i < playerMechs.size(); ++i) {
        const Entity* player = playerMechs[i];
        float yPos = playerYStart + i * playerYSpacing;
        
        // Draw mech circle
        Color playerColor = Color{80, 140, 200, 220};
        DrawCircle((int)(panel.playerArea.x + panel.playerArea.width - 40), (int)(yPos + 15), 20.0f, playerColor);
        DrawCircleLines((int)(panel.playerArea.x + panel.playerArea.width - 40), (int)(yPos + 15), 20.0f, WHITE);
        
        // Draw mech ID
        std::string idStr = "P" + std::to_string(player->id);
        DrawText(idStr.c_str(), (int)(panel.playerArea.x + panel.playerArea.width - 48), (int)(yPos + 8), 12, WHITE);
        
        // Draw health bar background
        Rectangle healthBarBg = {
            panel.playerArea.x + 10.0f,
            yPos + 5.0f,
            panel.playerArea.width - 75.0f,
            12.0f
        };
        DrawRectangleRec(healthBarBg, Color{40, 40, 40, 200});
        DrawRectangleLinesEx(healthBarBg, 1, DARKGRAY);
        
        // Draw health bar fill
        float healthPercent = std::max(0, std::min(100, player->health)) / 100.0f;
        Rectangle healthBarFill = {
            healthBarBg.x,
            healthBarBg.y,
            healthBarBg.width * healthPercent,
            healthBarBg.height
        };
        Color healthColor = healthPercent > 0.5f ? Color{80, 200, 80, 220} :
                           healthPercent > 0.25f ? Color{200, 180, 80, 220} :
                                                   Color{200, 80, 80, 220};
        DrawRectangleRec(healthBarFill, healthColor);
        
        // Draw health text
        std::string healthStr = std::to_string(player->health) + "/100";
        DrawText(healthStr.c_str(), (int)(healthBarBg.x + 5), (int)(yPos + 18), 10, WHITE);
    }
    
    // Draw placeholder combat area text
    DrawText("COMBAT AREA", (int)(panel.centerLine.x - 40), (int)(panel.centerLine.y + panel.centerLine.height / 2 - 8), 14, DARKGRAY);
}
