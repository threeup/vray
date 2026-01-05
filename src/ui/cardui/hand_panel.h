#pragma once

#include "raylib.h"
#include "ui.h"
#include <vector>

/**
 * T_052: HandPanel with Drag-Enabled PlayableCards
 * 
 * Create HandPanel with darker rounded rectangle background.
 * Implement PlayableCard components that display card art, stats (power, effect).
 * Enable drag-and-drop from hand; visual feedback on hover/selected.
 */

struct HandPanel {
    Rectangle bounds;
    std::vector<PlayableCardUI> cards;
};

// Forward declarations
struct Game;
struct DragState;
struct CardActions;
struct CardTooltip;

// Draw the hand panel with draggable cards
void HandPanel_Draw(const Rectangle& handRect, Game& game, DragState& drag, CardActions& actions, CardTooltip& tooltip);

// Update drag state during frame (mouse movement, etc)
void HandPanel_UpdateDrag(DragState& drag);

// T_059: Draw play turn button
void draw_play_turn_button(const Rectangle& handRect, Game& game, CardActions& actions);
