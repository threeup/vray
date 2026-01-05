#include "mech_slot_container.h"
#include "game_ui_panel.h"
#include "raylib.h"
#include "raygui.h"
#include "game.h"
#include "ui.h"
#include "card.h"
#include <string>
#include <cmath>

/**
 * T_053: MechSlotContainer Implementation
 * 
 * Renders 3 mech+card pairs with drop zone detection.
 */

void MechSlotContainer::computeLayout(Rectangle containerRect) {
    const float okButtonWidth = 40.0f;  // Space reserved for OK button
    const float availableWidth = containerRect.width - okButtonWidth - 8.0f;  // Leave 8px margin
    const float slotWidth = (availableWidth - 32.0f) / 3.0f;  // 3 slots with gaps
    const float mechCircleSize = 80.0f;
    const float cardSlotWidth = 120.0f;
    const float cardSlotHeight = 160.0f;
    const float gapBetweenSlots = 16.0f;
    
    for (int i = 0; i < 3; ++i) {
        float slotX = containerRect.x + 8.0f + i * (slotWidth + gapBetweenSlots);
        float slotY = containerRect.y + (containerRect.height - mechCircleSize) / 2.0f;
        
        // Mech circle on the left
        slots[i].circleRect = {
            slotX,
            slotY,
            mechCircleSize,
            mechCircleSize
        };
        
        // Card slot on the right, vertically centered
        float cardSlotY = containerRect.y + (containerRect.height - cardSlotHeight) / 2.0f;
        slots[i].cardSlotRect = {
            slotX + mechCircleSize + 8.0f,
            cardSlotY,
            cardSlotWidth,
            cardSlotHeight
        };
    }
}

// Get faction color based on mech variant
Color getMechVariantColor(MechVariant variant) {
    switch (variant) {
    case MechVariant::Alpha:
        return Color{200, 80, 80, 220};    // Red
    case MechVariant::Bravo:
        return Color{80, 140, 200, 220};   // Blue
    case MechVariant::Charlie:
        return Color{80, 200, 100, 220};   // Green
    default:
        return Color{100, 100, 100, 220};
    }
}

void MechSlotContainer_Draw(const Rectangle& slotRect, Game& game, DragState& drag, UiActions& actions, GameUIPanel& layout) {
    // Create and compute layout
    MechSlotContainer container;
    container.computeLayout(slotRect);
    
    // Get player mechs from game
    std::vector<int> playerMechIds;
    for (const auto& entity : game.entities) {
        if (entity.type == PLAYER) {
            playerMechIds.push_back(entity.id);
            if (playerMechIds.size() >= 3) break;
        }
    }
    
    // Assign mechs to slots with variants
    MechVariant variants[] = {MechVariant::Alpha, MechVariant::Bravo, MechVariant::Charlie};
    for (size_t i = 0; i < 3; ++i) {
        if (i < playerMechIds.size()) {
            container.slots[i].mechId = playerMechIds[i];
            container.slots[i].variant = variants[i];
        }
        
        // Find assigned card for this mech
        for (const auto& assignment : game.currentPlan.assignments) {
            if (assignment.mechId == container.slots[i].mechId) {
                container.slots[i].assignedCardId = assignment.cardId;
                container.slots[i].useMirror = assignment.useMirror;
                break;
            }
        }
    }
    
    // Check for drop zone collisions
    if (drag.isDragging) {
        // Create rectangle for dragged card (100x140 standard size)
        Rectangle draggedCardRect = {
            drag.currentPos.x,
            drag.currentPos.y,
            100.0f,
            140.0f
        };
        
        for (int i = 0; i < 3; ++i) {
            Rectangle cardSlot = container.slots[i].cardSlotRect;
            // Check if dragged card overlaps with slot
            if (CheckCollisionRecs(draggedCardRect, cardSlot)) {
                container.slots[i].isDropTarget = true;
                drag.hoverSlotIndex = i;
            } else {
                container.slots[i].isDropTarget = false;
            }
        }
    } else {
        for (int i = 0; i < 3; ++i) {
            container.slots[i].isDropTarget = false;
        }
        drag.hoverSlotIndex = -1;
    }
    
    // Draw each slot
    for (int i = 0; i < 3; ++i) {
        const MechSlotUI& slot = container.slots[i];
        
        // T_057: Calculate stats for this mech
        MechStats mechStats = calculateMechStats(slot.mechId, game);
        
        // Draw mech circle
        Color mechColor = getMechVariantColor(slot.variant);
        DrawCircle((int)(slot.circleRect.x + slot.circleRect.width / 2),
                   (int)(slot.circleRect.y + slot.circleRect.height / 2),
                   slot.circleRect.width / 2,
                   mechColor);
        
        // Draw circle outline
        Color outlineColor = slot.isDropTarget ? YELLOW : DARKGRAY;
        DrawCircleLines((int)(slot.circleRect.x + slot.circleRect.width / 2),
                        (int)(slot.circleRect.y + slot.circleRect.height / 2),
                        slot.circleRect.width / 2,
                        outlineColor);
        
        // Draw mech ID
        std::string mechIdText = std::to_string(slot.mechId);
        DrawText(mechIdText.c_str(),
                 (int)(slot.circleRect.x + slot.circleRect.width / 2 - 6),
                 (int)(slot.circleRect.y + slot.circleRect.height / 2 - 8),
                 16, WHITE);
        
        // T_057: Draw stat block below mech circle
        float statY = slot.circleRect.y + slot.circleRect.height + 4.0f;
        std::string statStr = "HP: " + std::to_string(mechStats.currentHealth);
        if (mechStats.attackBonus > 0) {
            statStr += " | ATK: +" + std::to_string(mechStats.attackBonus);
        }
        if (mechStats.defenseBonus > 0) {
            statStr += " | DEF: +" + std::to_string(mechStats.defenseBonus);
        }
        DrawText(statStr.c_str(), (int)(slot.circleRect.x + 2), (int)statY, 10, LIGHTGRAY);
        
        // Draw synergy text if present
        if (!mechStats.synergyText.empty()) {
            DrawText(mechStats.synergyText.c_str(), (int)(slot.circleRect.x + 2), (int)(statY + 12), 9, YELLOW);
        }
        
        // Draw card slot background
        Color slotBgColor = slot.isDropTarget ? Color{80, 120, 80, 220} : Color{50, 50, 60, 180};
        DrawRectangleRec(slot.cardSlotRect, slotBgColor);
        
        // Card slot outline
        Color slotOutline = slot.isDropTarget ? Color{120, 220, 120, 255} : Color{100, 100, 100, 150};
        int outlineWidth = slot.isDropTarget ? 2 : 1;
        DrawRectangleLinesEx(slot.cardSlotRect, (float)outlineWidth, slotOutline);
        
        // Draw assigned card preview if present
        if (slot.assignedCardId != -1) {
            const Card* assignedCard = nullptr;
            for (const auto& c : game.hand.cards) {
                if (c.id == slot.assignedCardId) {
                    assignedCard = &c;
                    break;
                }
            }
            
            if (assignedCard) {
                // Draw card name
                DrawText(assignedCard->name.c_str(),
                         (int)(slot.cardSlotRect.x + 8),
                         (int)(slot.cardSlotRect.y + 8),
                         12, WHITE);
                
                // T_055: Draw mirror button
                Rectangle mirrorBtnRect = {
                    slot.cardSlotRect.x + slot.cardSlotRect.width - 28,
                    slot.cardSlotRect.y + 4,
                    24.0f,
                    20.0f
                };
                
                Color mirrorBtnBg = slot.useMirror ? Color{180, 140, 60, 220} : Color{80, 80, 100, 180};
                DrawRectangleRec(mirrorBtnRect, mirrorBtnBg);
                DrawRectangleLinesEx(mirrorBtnRect, 1, LIGHTGRAY);
                DrawText("M", (int)(mirrorBtnRect.x + 8), (int)(mirrorBtnRect.y + 4), 12, WHITE);
                
                // Check for mirror button click
                Vector2 mousePos = GetMousePosition();
                if (CheckCollisionPointRec(mousePos, mirrorBtnRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    actions.toggleMirrorSlot = slot.mechId;
                }
                
                // REMOVE CARD BUTTON - Down arrow or X button below mirror button
                Rectangle removeCardBtnRect = {
                    slot.cardSlotRect.x + slot.cardSlotRect.width - 28,
                    slot.cardSlotRect.y + 26,
                    24.0f,
                    20.0f
                };
                
                Color removeBtnBg = Color{150, 80, 80, 200};
                DrawRectangleRec(removeCardBtnRect, removeBtnBg);
                DrawRectangleLinesEx(removeCardBtnRect, 1, LIGHTGRAY);
                DrawText("X", (int)(removeCardBtnRect.x + 8), (int)(removeCardBtnRect.y + 3), 14, WHITE);
                
                // Check for remove card button click
                if (CheckCollisionPointRec(mousePos, removeCardBtnRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    // Remove this assignment and mark card as unused
                    for (auto it = game.currentPlan.assignments.begin(); it != game.currentPlan.assignments.end(); ++it) {
                        if (it->mechId == slot.mechId) {
                            // Log card removal before erasing
                            TraceLog(LOG_INFO, "UI: Card REMOVED - Card '%s' (ID: %d) unassigned from Mech (ID: %d), returned to hand",
                                     assignedCard ? assignedCard->name.c_str() : "Unknown",
                                     it->cardId,
                                     slot.mechId);
                            
                            game.hand.unmarkUsed(it->cardId);
                            game.currentPlan.assignments.erase(it);
                            break;
                        }
                    }
                }
                
                // Draw mirror indicator if mirrored
                if (slot.useMirror) {
                    DrawText("(MIRROR)", (int)(slot.cardSlotRect.x + 8),
                             (int)(slot.cardSlotRect.y + 25), 10, YELLOW);
                }
                
                // Draw effect type
                const char* typeStr = "";
                switch (assignedCard->type) {
                case CardType::Move:
                    typeStr = "MOV";
                    break;
                case CardType::Damage:
                    typeStr = "DMG";
                    break;
                case CardType::Heal:
                    typeStr = "HEL";
                    break;
                }
                DrawText(typeStr, (int)(slot.cardSlotRect.x + 8),
                         (int)(slot.cardSlotRect.y + slot.cardSlotRect.height - 20),
                         11, LIGHTGRAY);
            }
        } else {
            // Empty slot indicator
            DrawText("(empty)", (int)(slot.cardSlotRect.x + 8),
                     (int)(slot.cardSlotRect.y + slot.cardSlotRect.height / 2 - 8),
                     12, DARKGRAY);
        }
    }
    
    // Draw OK button at the right edge of mech container
    Rectangle okButtonRect = {
        slotRect.x + slotRect.width - 40.0f,
        slotRect.y,
        40.0f,
        slotRect.height
    };
    
    // Check if there are assignments
    bool canSubmit = !game.currentPlan.assignments.empty();
    
    // Draw button background
    Color btnColor = canSubmit ? Color{80, 200, 80, 220} : Color{80, 80, 80, 150};
    DrawRectangleRec(okButtonRect, btnColor);
    
    // Button border
    Color borderColor = canSubmit ? Color{140, 255, 140, 255} : Color{100, 100, 100, 150};
    DrawRectangleLinesEx(okButtonRect, canSubmit ? 2 : 1, borderColor);
    
    // Button text
    DrawText("OK", (int)(okButtonRect.x + 12), (int)(okButtonRect.y + okButtonRect.height / 2 - 6), 14, WHITE);
    
    // Check for click
    if (canSubmit && CheckCollisionPointRec(GetMousePosition(), okButtonRect)) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            actions.playSequence = true;
            layout.showMechRow = false;  // Hide mech row after OK is pressed
        }
    }
}

int MechSlotContainer_GetDropSlotIndex(const MechSlotContainer& container, const Rectangle& slotRect, int x, int y) {
    MechSlotContainer computedContainer = container;
    computedContainer.computeLayout(slotRect);
    
    Vector2 point = {(float)x, (float)y};
    for (int i = 0; i < 3; ++i) {
        if (CheckCollisionPointRec(point, computedContainer.slots[i].cardSlotRect)) {
            return i;
        }
    }
    return -1;
}
