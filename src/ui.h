#pragma once

#include <vector>
#include <string>
#include "card.h"
#include "game.h"
#include "app.h"

struct UiActions {
    bool playSequence = false;
    bool clearSequence = false;
};

// Draw card/sequence panel; returns user intents (play/clear).
UiActions draw_card_ui(Game& game, float panelWidth);

// Draw render controls (supersample toggle, FXAA toggle) anchored at bottom.
void draw_render_controls(UiState& ui, int screenWidth, int screenHeight);

// Draw the entire UI and return any user actions.
UiActions UI_Draw(AppContext& ctx);

