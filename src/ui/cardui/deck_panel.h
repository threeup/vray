#pragma once

#include "raylib.h"

/**
 * T_051: DeckPanel (Top-Right Deck Display)
 * 
 * Display remaining card count in top-right corner.
 * Implement clickable draw button to draw cards from deck.
 * Show visual card stack indicator.
 */

struct DeckPanel {
    int deckSize = 0;
    bool isHovered = false;
    bool drawButtonPressed = false;
};

// Forward declaration
struct Game;
struct UiActions;

// Draw the deck panel at specified rectangle
void DeckPanel_Draw(const Rectangle& deckRect, Game& game, UiActions& actions);
