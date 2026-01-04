#pragma once

#include <vector>
#include <string>
#include "card.h"
#include "game.h"
#include "app.h"

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
};

// Draw card/sequence panel; returns user intents (play/clear).
UiActions draw_card_ui(Game& game, float panelWidth);

// Draw render controls (supersample toggle, FXAA toggle) anchored at bottom.
void draw_render_controls(UiState& ui, int screenWidth, int screenHeight);

// Draw the entire UI and return any user actions.
UiActions UI_Draw(AppContext& ctx);

