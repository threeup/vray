#pragma once

#include "raylib.h"
#include <cstdint>

/**
 * T_053: MechSlotContainer with 3 Mech+Card Pairs
 * 
 * Create MechSlotContainer managing 3 horizontal mech+card pairs.
 * Implement MechCircle (visual circle, color-coded by variant Alpha/Bravo/Charlie).
 * Implement CardSlotPanel (darker rounded rectangle) adjacent to each mech.
 * Each slot is a drop target zone with hover highlight.
 */

enum class MechVariant {
    Alpha,   // Red
    Bravo,   // Blue
    Charlie  // Green
};

struct MechSlotUI {
    int mechId = -1;
    MechVariant variant = MechVariant::Alpha;
    Rectangle circleRect;           // bounding box for mech circle
    Rectangle cardSlotRect;         // adjacent card slot drop zone
    bool isDropTarget = false;      // true when card dragged over
    int assignedCardId = -1;        // -1 if empty
    bool useMirror = false;
};

struct MechSlotContainer {
    MechSlotUI slots[3];
    
    // Compute layout for the 3 mech slots
    void computeLayout(Rectangle containerRect);
};

// Forward declarations
struct Game;
struct DragState;
struct CardActions;

// Draw the mech slot container with 3 mech+card pairs
struct GameUIPanel;  // Forward declaration

void MechSlotContainer_Draw(const Rectangle& slotRect, Game& game, DragState& drag, CardActions& actions, GameUIPanel& layout);

// Check if a point is over a card slot (for drop zone detection)
int MechSlotContainer_GetDropSlotIndex(const MechSlotContainer& container, const Rectangle& slotRect, int x, int y);
