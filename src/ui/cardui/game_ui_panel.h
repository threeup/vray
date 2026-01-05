#pragma once

#include "raylib.h"
#include <cmath>

/**
 * T_050: GameUIPanel Root Layout Container
 * 
 * Root container managing viewport/screen coordinate mapping.
 * Divides screen into 5 sub-regions:
 * - phaseRect: top bar showing game phase (existing)
 * - deckRect: top-right corner for deck display
 * - gameBoardRect: center arena for mechs and combat
 * - mechSlotRect: bottom-center band for mech+card pairs
 * - handRect: very bottom for hand cards
 */

struct PanelMetrics {
    float margin = 10.0f;      // outer margin from window edges
    float padding = 12.0f;     // inner padding within panels
    float cornerRadius = 4.0f; // for rounded rectangles
};

struct GameUIPanel {
    Rectangle phaseRect;       // top bar (reduced height when hidden)
    Rectangle deckRect;        // top-left (50% width)
    Rectangle gameBoardRect;   // top-right (50% width)
    Rectangle mechSlotRect;    // middle band (160px height, hidden after OK)
    Rectangle handRect;        // very bottom (120px height)
    
    PanelMetrics metrics;
    bool showMechRow = true;   // visibility flag for mech slot row
    
    // Compute layout based on window dimensions
    void computeLayout(int winW, int winH);
};

// Helper function to compute all layout regions
inline void GameUIPanel::computeLayout(int winW, int winH) {
    float margin = metrics.margin;
    float usableW = (float)(winW - 2 * (int)margin);
    float usableH = (float)(winH - 2 * (int)margin);
    
    // Phase bar: top, full width, 60% of original height (34px * 0.6 ≈ 20px)
    float phaseHeight = 20.0f;
    phaseRect = {
        margin,
        margin,
        usableW,
        phaseHeight
    };
    
    // Top row split 50/50: left (deck) and right (game board)
    float topRowHeight = 80.0f * 0.6f;  // 60% of original deck panel height
    float topRowWidth = usableW * 0.5f;
    
    // Deck panel: top-left, 50% width
    deckRect = {
        margin,
        margin + phaseHeight + metrics.padding,
        topRowWidth,
        topRowHeight
    };
    
    // Game board: top-right, 50% width
    gameBoardRect = {
        margin + topRowWidth + metrics.padding,
        margin + phaseHeight + metrics.padding,
        topRowWidth,
        topRowHeight
    };
    
    // Hand panel: very bottom, full width (minus margins), 120px height
    float handHeight = 120.0f;
    handRect = {
        margin,
        (float)(winH - (int)handHeight - (int)margin),
        usableW,
        handHeight
    };
    
    // Mech slot container: above hand, full width, 160px height (conditional visibility)
    float mechSlotHeight = 160.0f;
    mechSlotRect = {
        margin,
        handRect.y - mechSlotHeight - metrics.padding,
        usableW,
        mechSlotHeight
    };
}

