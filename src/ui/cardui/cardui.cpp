#include "game_ui_panel.h"
#include "deck_panel.h"
#include "hand_panel.h"
#include "mech_slot_container.h"
#include "game_board_panel.h"
#include "raylib.h"
#include "game.h"
#include "ui.h"

/**
 * T_050: GameUIPanel Root Layout Container Implementation
 * 
 * Provides root UI layout and rendering scaffold for the new card UI system.
 */

// Draw the phase indicator bar at the top
void draw_phase_indicator(const Rectangle& phaseRect, int currentPhase) {
    // Phase bar background
    DrawRectangleRec(phaseRect, Color{240, 240, 240, 255});
    DrawRectangleLinesEx(phaseRect, 1, DARKGRAY);
    
    // Phase labels and indicators
    const char* labels[3] = {"User selects", "NPC selects", "Play phase"};
    for (int i = 0; i < 3; ++i) {
        float slotW = (phaseRect.width - 40.0f) / 3.0f;
        float cx = phaseRect.x + 20.0f + slotW * i + slotW * 0.1f;
        float cy = phaseRect.y + phaseRect.height * 0.5f;
        
        Color outline = DARKGRAY;
        DrawCircleLines((int)cx, (int)cy, 8.0f, outline);
        if (currentPhase == i) {
            DrawCircle((int)cx, (int)cy, 6.0f, BLACK);
        }
        DrawText(labels[i], (int)(cx + 14.0f), (int)(cy - 7.0f), 14, BLACK);
    }
}

// T_051: DeckPanel implementation
void DeckPanel_Draw(const Rectangle& deckRect, Game& game, UiActions& actions) {
    // Draw panel background
    DrawRectangleRec(deckRect, Color{60, 60, 70, 200});
    DrawRectangleLinesEx(deckRect, 2, LIGHTGRAY);
    
    // Check if mouse is over the deck panel
    Vector2 mousePos = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePos, deckRect);
    
    // Draw stacked card visual (3 offset rectangles to simulate depth)
    float cardW = 32.0f;
    float cardH = 44.0f;
    float stackX = deckRect.x + (deckRect.width - cardW) * 0.5f;
    float stackY = deckRect.y + 8.0f;
    
    for (int i = 2; i >= 0; --i) {
        float offsetX = i * 2.0f;
        float offsetY = i * 2.0f;
        Color cardColor = isHovered ? Color{100, 100, 120, 220} : Color{80, 80, 100, 200};
        DrawRectangle((int)(stackX + offsetX), (int)(stackY + offsetY), (int)cardW, (int)cardH, cardColor);
        DrawRectangleLinesEx({stackX + offsetX, stackY + offsetY, cardW, cardH}, 1, LIGHTGRAY);
    }
    
    // Draw deck count
    int deckCount = game.deck.remaining();
    std::string countText = "DECK: " + std::to_string(deckCount);
    DrawText(countText.c_str(), (int)(deckRect.x + 10), (int)(deckRect.y + 54), 14, WHITE);
    
    // Draw "Draw" button
    Rectangle drawBtnRect = {
        deckRect.x + 8.0f,
        deckRect.y + deckRect.height - 26.0f,
        deckRect.width - 16.0f,
        20.0f
    };
    
    Color btnColor = isHovered ? Color{100, 180, 100, 220} : Color{80, 150, 80, 200};
    DrawRectangleRec(drawBtnRect, btnColor);
    DrawRectangleLinesEx(drawBtnRect, 1, LIGHTGRAY);
    DrawText("DRAW", (int)(drawBtnRect.x + 8), (int)(drawBtnRect.y + 4), 12, WHITE);
    
    // Check for button click
    if (isHovered && CheckCollisionPointRec(mousePos, drawBtnRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        actions.drawCard = true;
    }
}

// Placeholder for deck panel (T_051)
void draw_deck_panel(const Rectangle& deckRect) {
    DrawRectangleRec(deckRect, Color{60, 60, 70, 200});
    DrawRectangleLinesEx(deckRect, 2, LIGHTGRAY);
    DrawText("DECK", (int)(deckRect.x + 10), (int)(deckRect.y + 10), 16, WHITE);
    DrawText("(T_051)", (int)(deckRect.x + 10), (int)(deckRect.y + 35), 12, GRAY);
}

// Placeholder for game board (T_056)
void draw_game_board(const Rectangle& boardRect) {
    DrawRectangleRec(boardRect, Color{40, 50, 60, 100});
    DrawRectangleLinesEx(boardRect, 1, DARKGRAY);
    DrawText("COMBAT AREA", (int)(boardRect.x + 20), (int)(boardRect.y + 20), 18, DARKGRAY);
    DrawText("(T_056: GameBoardPanel)", (int)(boardRect.x + 20), (int)(boardRect.y + 50), 12, GRAY);
}

// Placeholder for hand panel (T_052)
void draw_hand_panel(const Rectangle& handRect) {
    DrawRectangleRec(handRect, Color{40, 40, 50, 220});
    DrawRectangleLinesEx(handRect, 2, DARKGRAY);
    DrawText("HAND", (int)(handRect.x + 10), (int)(handRect.y + 10), 16, LIGHTGRAY);
    DrawText("(T_052: Drag-Enabled PlayableCards)", (int)(handRect.x + 10), (int)(handRect.y + 35), 12, GRAY);
}

// Main UI drawing function using GameUIPanel layout
void draw_cardui(GameUIPanel& layout, int currentPhase, int winW, int winH, Game& game, UiActions& actions, DragState& drag, CardTooltip& tooltip) {
    // Recompute layout in case window was resized
    layout.computeLayout(winW, winH);
    
    // Hide top row when not in PlayerSelect phase
    bool isPlayerSelectPhase = (currentPhase == 0);  // Phase::PlayerSelect = 0
    
    // Draw phase indicator - ALWAYS visible (even during AI and Play phases)
    draw_phase_indicator(layout.phaseRect, currentPhase);
    
    // NOTE: HandPanel_UpdateDrag is NOT called here anymore.
    // It's called AFTER update_cardui_drop so that drop logic can check drag state before it's cleared.
    
    // Draw all sub-panels in their designated regions
    // Top row (deck and game board) - only show in PlayerSelect phase
    if (isPlayerSelectPhase) {
        DeckPanel_Draw(layout.deckRect, game, actions);                    // T_051
        GameBoardPanel_Draw(layout.gameBoardRect, game);                   // T_056
    }
    
    // Mech row - only show during PlayerSelect phase AND when flag is true (hidden after OK pressed)
    if (isPlayerSelectPhase && layout.showMechRow) {
        MechSlotContainer_Draw(layout.mechSlotRect, game, drag, actions, layout);  // T_053, T_054, T_055, T_059: OK button
    }
    
    // Hand panel - always shown
    HandPanel_Draw(layout.handRect, game, drag, actions, tooltip);     // T_052, T_058 tooltip
}

// T_054: Handle drag-drop logic
void update_cardui_drop(Game& game, UiActions& actions, DragState& drag) {
    // If mouse was released while dragging
    if (drag.isDragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        // Find the card being dragged for logging
        const Card* draggedCard = nullptr;
        for (const auto& c : game.hand.cards) {
            if (c.id == drag.draggedCardId) {
                draggedCard = &c;
                break;
            }
        }
        
        // Log card release event
        TraceLog(LOG_INFO, "UI: Card released - ID: %d, Name: %s", 
                 drag.draggedCardId, 
                 draggedCard ? draggedCard->name.c_str() : "Unknown");
        
        // Check if dropped on a valid mech slot
        if (drag.hoverSlotIndex != -1 && drag.draggedCardId != -1) {
            // Get the mech ID at this slot
            std::vector<int> playerMechIds;
            for (const auto& entity : game.entities) {
                if (entity.type == PLAYER) {
                    playerMechIds.push_back(entity.id);
                    if (playerMechIds.size() >= 3) break;
                }
            }
            
            if (drag.hoverSlotIndex < (int)playerMechIds.size()) {
                int targetMechId = playerMechIds[drag.hoverSlotIndex];
                
                // Check if slot is already occupied
                bool slotOccupied = false;
                for (const auto& a : game.currentPlan.assignments) {
                    if (a.mechId == targetMechId) {
                        slotOccupied = true;
                        break;
                    }
                }
                
                // Check if card can be played
                bool canPlay = game.hand.canPlay(drag.draggedCardId);
                
                if (!slotOccupied && canPlay) {
                    // Valid drop - create assignment
                    PlanAssignment newAssignment;
                    newAssignment.mechId = targetMechId;
                    newAssignment.cardId = drag.draggedCardId;
                    newAssignment.useMirror = false;  // Default to no mirror
                    
                    game.currentPlan.assignments.push_back(newAssignment);
                    game.hand.markUsed(drag.draggedCardId);
                    
                    // Log successful assignment
                    TraceLog(LOG_INFO, "UI: Card ASSIGNED - Card '%s' (ID: %d) -> Mech (ID: %d)", 
                             draggedCard ? draggedCard->name.c_str() : "Unknown",
                             drag.draggedCardId,
                             targetMechId);
                } else {
                    // Invalid drop - explain why
                    if (slotOccupied) {
                        TraceLog(LOG_WARNING, "UI: Card REJECTED - Mech slot already occupied (Mech ID: %d)", targetMechId);
                    } else if (!canPlay) {
                        TraceLog(LOG_WARNING, "UI: Card REJECTED - Card already used or not playable (Card ID: %d)", drag.draggedCardId);
                    }
                }
            }
        } else if (drag.hoverSlotIndex == -1) {
            // Dropped outside any valid slot
            TraceLog(LOG_WARNING, "UI: Card BOUNCED - Dropped outside mech slots, returning to hand (Card ID: %d)", drag.draggedCardId);
        }
        
        // End drag
        drag.isDragging = false;
        drag.draggedCardId = -1;
        drag.hoverSlotIndex = -1;
    }
}

// Placeholder for mech slot container (T_053)
void draw_mech_slot_container(const Rectangle& slotRect) {
    DrawRectangleRec(slotRect, Color{50, 50, 60, 200});
    DrawRectangleLinesEx(slotRect, 2, DARKGRAY);
    DrawText("MECH SLOTS", (int)(slotRect.x + 10), (int)(slotRect.y + 10), 16, LIGHTGRAY);
    DrawText("(T_053: 3 Mech+Card Pairs)", (int)(slotRect.x + 10), (int)(slotRect.y + 35), 12, GRAY);
    DrawText("(T_054: Drag-and-Drop)", (int)(slotRect.x + 10), (int)(slotRect.y + 50), 12, GRAY);
}
