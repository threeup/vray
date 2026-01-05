#pragma once

#include "game_ui_panel.h"

// Forward declarations
struct Game;
struct CardActions;
struct DragState;
struct CardTooltip;

/**
 * T_050-T_059: Card UI System Implementation
 * 
 * Main entry point for the new card UI system.
 */

// Draw the complete card UI layout using GameUIPanel
void draw_cardui(GameUIPanel& layout, int currentPhase, int winW, int winH, Game& game, CardActions& actions, DragState& drag, CardTooltip& tooltip);

// Update card placements based on drag-drop (T_054)
void update_cardui_drop(Game& game, CardActions& actions, DragState& drag);
