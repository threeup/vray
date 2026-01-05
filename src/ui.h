#pragma once

#include <vector>
#include <string>
#include "card.h"
#include "game.h"
#include "app.h"
#include "raylib.h"

/**
 * T_057: Mech stat calculations
 */
struct MechStats {
    int baseHealth = 100;
    int currentHealth = 100;
    int attackBonus = 0;
    int defenseBonus = 0;
    std::string synergyText = "";
};

/**
 * T_052: Drag-and-drop state management
 */
struct DragState {
    bool isDragging = false;
    int draggedCardId = -1;
    Vector2 dragOffset = {0, 0};
    Vector2 currentPos = {0, 0};
    int hoverSlotIndex = -1;    // which MechSlotUI is under cursor (T_054)
    bool dropValid = false;
};

struct PlayableCardUI {
    Rectangle bounds;
    int cardId = -1;
    bool isHovered = false;
    bool isSelected = false;
};

/**
 * T_058: Tooltip for card hover information
 */
struct CardTooltip {
    bool visible = false;
    int cardId = -1;
    Vector2 position = {0, 0};
    float showDelay = 0.3f;
    float hoverTime = 0.0f;
};

struct UiActions {
    bool playSequence = false;
    bool clearSequence = false;
    bool mirrorNext = false;
    int selectedMechId = -1;
    int selectedCardId = -1;
    bool undoLast = false;
    int removeAssignmentMechId = -1;
    int selectCardId = -1;       // choose a card for assignment
    int assignCardToMech = -1;   // assign the selected card to this mech
    bool confirmPlan = false;    // explicit confirm/advance
    bool drawCard = false;       // T_051: draw a card from deck
    int toggleMirrorSlot = -1;   // T_055: toggle mirror for mech slot (-1 = no toggle)
};

// Draw card/sequence panel; returns user intents (play/clear).
UiActions draw_card_ui(Game& game, float panelWidth);

// T_057: Calculate mech stats based on assigned cards
MechStats calculateMechStats(int mechId, const Game& game);

// T_058: Draw card tooltip on hover with effect details
void CardTooltip_Draw(CardTooltip& tooltip, const Game& game);

// Draw render controls (supersample toggle, FXAA toggle) anchored at bottom.
void draw_render_controls(UiState& ui, int screenWidth, int screenHeight);

// Draw the entire UI and return any user actions.
UiActions UI_Draw(AppContext& ctx);

