#include "hand_panel.h"
#include "raylib.h"
#include "game.h"
#include "ui.h"
#include "card.h"
#include <string>
#include <cmath>

/**
 * T_052: HandPanel Implementation
 * 
 * Renders draggable card components in the hand panel.
 */

// Helper function to draw a single card
void draw_playable_card(const PlayableCardUI& cardUI, const Card* card, bool isDragged) {
    if (!card) return;
    
    Rectangle rect = cardUI.bounds;
    
    // Determine card color based on type
    Color cardColor;
    switch (card->type) {
    case CardType::Move:
        cardColor = Color{100, 140, 180, 200};
        break;
    case CardType::Damage:
        cardColor = Color{180, 80, 80, 200};
        break;
    case CardType::Heal:
        cardColor = Color{80, 180, 100, 200};
        break;
    default:
        cardColor = Color{100, 100, 100, 200};
    }
    
    // Draw card body
    if (isDragged) {
        // Semi-transparent while dragging
        cardColor.a = 150;
    } else if (cardUI.isHovered) {
        // Brighter on hover
        cardColor.r = std::min(255, cardColor.r + 40);
        cardColor.g = std::min(255, cardColor.g + 40);
        cardColor.b = std::min(255, cardColor.b + 40);
    }
    
    DrawRectangleRec(rect, cardColor);
    
    // Border
    Color borderColor = cardUI.isHovered ? YELLOW : LIGHTGRAY;
    DrawRectangleLinesEx(rect, (float)(isDragged ? 3 : cardUI.isHovered ? 2 : 1), borderColor);
    
    // Card name
    int nameX = (int)(rect.x + 4);
    int nameY = (int)(rect.y + 4);
    DrawText(card->name.c_str(), nameX, nameY, 12, WHITE);
    
    // Card type
    const char* typeStr = "";
    switch (card->type) {
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
    DrawText(typeStr, (int)(rect.x + rect.width - 24), nameY, 12, WHITE);
    
    // Effect info
    int infoY = nameY + 16;
    switch (card->type) {
    case CardType::Move: {
        std::string moveStr = "F:" + std::to_string(card->effect.move.forward) +
                              " L:" + std::to_string(card->effect.move.lateral);
        DrawText(moveStr.c_str(), (int)(rect.x + 4), infoY, 10, LIGHTGRAY);
        break;
    }
    case CardType::Damage: {
        std::string dmgStr = "DMG: " + std::to_string(card->effect.damage);
        DrawText(dmgStr.c_str(), (int)(rect.x + 4), infoY, 10, LIGHTGRAY);
        break;
    }
    case CardType::Heal: {
        std::string healStr = "HP: +" + std::to_string(card->effect.heal);
        DrawText(healStr.c_str(), (int)(rect.x + 4), infoY, 10, LIGHTGRAY);
        break;
    }
    }
}

void HandPanel_Draw(const Rectangle& handRect, Game& game, DragState& drag, CardActions& actions, CardTooltip& tooltip) {
    // Draw panel background
    DrawRectangleRec(handRect, Color{40, 40, 50, 220});
    DrawRectangleLinesEx(handRect, 2, DARKGRAY);
    
    // Draw "HAND" label
    DrawText("HAND", (int)(handRect.x + 10), (int)(handRect.y + 10), 16, LIGHTGRAY);
    
    // Card layout parameters
    const float cardWidth = 100.0f;
    const float cardHeight = 140.0f;
    const float spacing = 12.0f;
    const float cardAreaY = handRect.y + 32.0f;
    const float cardAreaHeight = handRect.height - 42.0f;
    
    // Calculate total width needed and center cards horizontally
    int numCards = (int)game.hand.cards.size();
    float totalCardWidth = numCards * cardWidth + (numCards - 1) * spacing;
    float startX = handRect.x + (handRect.width - totalCardWidth) / 2.0f;
    
    // Clamp to panel bounds
    if (startX < handRect.x + 8.0f) {
        startX = handRect.x + 8.0f;
    }
    
    Vector2 mousePos = GetMousePosition();
    
    // T_058: Tooltip hover tracking
    bool anyCardHovered = false;
    float deltaTime = GetFrameTime();
    
    // Draw each card
    for (int i = 0; i < numCards; ++i) {
        const Card& card = game.hand.cards[i];
        
        Rectangle cardRect = {
            startX + i * (cardWidth + spacing),
            cardAreaY,
            cardWidth,
            cardHeight
        };
        
        // Check if card is already assigned
        bool isAssigned = false;
        for (const auto& a : game.currentPlan.assignments) {
            if (a.cardId == card.id) {
                isAssigned = true;
                break;
            }
        }
        
        // Skip if card is used up
        if (!game.hand.canPlay(card.id)) {
            isAssigned = true;  // Treat as unavailable
        }
        
        // Check hover
        bool isHovered = CheckCollisionPointRec(mousePos, cardRect);
        
        PlayableCardUI cardUI;
        cardUI.bounds = cardRect;
        cardUI.cardId = card.id;
        cardUI.isHovered = isHovered && !isAssigned;
        cardUI.isSelected = (drag.isDragging && drag.draggedCardId == card.id);
        
        // T_058: Track tooltip on hover
        if (cardUI.isHovered && !drag.isDragging) {
            anyCardHovered = true;
            tooltip.hoverTime += deltaTime;
            if (tooltip.hoverTime > tooltip.showDelay) {
                tooltip.visible = true;
                tooltip.cardId = card.id;
                tooltip.position = mousePos;
            }
        }
        
        // Draw card, dimmed if assigned/used
        if (isAssigned) {
            // Darken unavailable cards
            DrawRectangleRec(cardRect, Color{30, 30, 40, 120});
            DrawRectangleLinesEx(cardRect, 1, Color{80, 80, 80, 150});
            DrawText(card.name.c_str(), (int)(cardRect.x + 4), (int)(cardRect.y + 4), 12, Color{100, 100, 100, 150});
        } else {
            draw_playable_card(cardUI, &card, false);
            
            // Handle mouse down to start drag
            if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                drag.isDragging = true;
                drag.draggedCardId = card.id;
                drag.dragOffset = {mousePos.x - cardRect.x, mousePos.y - cardRect.y};
                drag.currentPos = {cardRect.x, cardRect.y};
            }
        }
    }
    
    // Draw dragged card at cursor if dragging
    if (drag.isDragging && drag.draggedCardId != -1) {
        const Card* draggedCard = nullptr;
        for (const auto& c : game.hand.cards) {
            if (c.id == drag.draggedCardId) {
                draggedCard = &c;
                break;
            }
        }
        
        if (draggedCard) {
            Rectangle dragRect = {
                mousePos.x - drag.dragOffset.x,
                mousePos.y - drag.dragOffset.y,
                cardWidth,
                cardHeight
            };
            
            PlayableCardUI dragUI;
            dragUI.bounds = dragRect;
            dragUI.cardId = drag.draggedCardId;
            dragUI.isHovered = false;
            dragUI.isSelected = true;
            
            draw_playable_card(dragUI, draggedCard, true);
        }
    }
    
    // T_058: Reset tooltip if no card hovered
    if (!anyCardHovered || drag.isDragging) {
        tooltip.hoverTime = 0.0f;
        tooltip.visible = false;
    }
}

void HandPanel_UpdateDrag(DragState& drag) {
    if (drag.isDragging) {
        // Update current position to follow mouse
        Vector2 mousePos = GetMousePosition();
        drag.currentPos = {mousePos.x - drag.dragOffset.x, mousePos.y - drag.dragOffset.y};
        
        // Check for mouse release to end drag
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            drag.isDragging = false;
            drag.draggedCardId = -1;
            // TODO: Handle drop logic (T_054)
        }
    }
}

// T_059: Draw play turn button
void draw_play_turn_button(const Rectangle& handRect, Game& game, CardActions& actions) {
    // Button positioned at bottom-right of hand panel
    Rectangle buttonRect = {
        handRect.x + handRect.width - 150,
        handRect.y + handRect.height - 30,
        140.0f,
        26.0f
    };
    
    // Check if there are assignments
    bool canSubmit = !game.currentPlan.assignments.empty();
    
    // Draw button background
    Color btnColor = canSubmit ? Color{80, 200, 80, 220} : Color{80, 80, 80, 150};
    DrawRectangleRec(buttonRect, btnColor);
    
    // Button border
    Color borderColor = canSubmit ? Color{140, 255, 140, 255} : Color{100, 100, 100, 150};
    DrawRectangleLinesEx(buttonRect, (float)(canSubmit ? 2 : 1), borderColor);
    
    // Button text
    DrawText("PLAY TURN", (int)(buttonRect.x + 12), (int)(buttonRect.y + 6), 12, WHITE);
    
    // Check for click
    if (canSubmit && CheckCollisionPointRec(GetMousePosition(), buttonRect)) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            actions.playSequence = true;
        }
    }
}
