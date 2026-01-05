#include "ui.h"
#include "app.h"
#include "game.h"
#include "boss.h"
#include "raygui.h"
#include <string>
#include <algorithm>

/**
 * T_058: CardTooltip_Draw - Render tooltip with card details on hover
 */
void CardTooltip_Draw(CardTooltip& tooltip, const Game& game) {
    if (!tooltip.visible) return;

    // Find card in game.hand
    const Card* card = nullptr;
    for (const auto& c : game.hand.cards) {
        if (c.id == tooltip.cardId) {
            card = &c;
            break;
        }
    }
    if (!card) return;

    // Tooltip box dimensions and position with offset
    float boxWidth = 200.0f;
    float boxHeight = 120.0f;
    float offsetX = 15.0f;
    float offsetY = 15.0f;
    Rectangle tooltipBox = {
        tooltip.position.x + offsetX,
        tooltip.position.y + offsetY,
        boxWidth,
        boxHeight
    };

    // Draw shadow first
    Rectangle shadowBox = {
        tooltipBox.x + 3.0f,
        tooltipBox.y + 3.0f,
        tooltipBox.width,
        tooltipBox.height
    };
    DrawRectangleRec(shadowBox, Color{0, 0, 0, 100});

    // Draw tooltip background with border
    DrawRectangleRec(tooltipBox, Color{240, 240, 240, 255});
    DrawRectangleLinesEx(tooltipBox, 2, Color{100, 100, 100, 255});

    // Draw card name (bold)
    std::string cardName = "Card #" + std::to_string(card->id);
    DrawText(cardName.c_str(), (int)(tooltipBox.x + 8.0f), (int)(tooltipBox.y + 6.0f), 14, BLACK);

    // Draw card type and effect details
    float contentY = tooltipBox.y + 26.0f;
    std::string typeStr;
    std::string effectStr;

    switch (card->type) {
        case CardType::Move:
            typeStr = "MOVE";
            effectStr = "Fwd: +" + std::to_string(card->effect.move.forward) + " Lat: +" + std::to_string(card->effect.move.lateral);
            break;
        case CardType::Damage:
            typeStr = "DAMAGE";
            effectStr = "Damage: " + std::to_string(card->effect.damage) + " to target";
            break;
        case CardType::Heal:
            typeStr = "HEAL";
            effectStr = "Heal: +" + std::to_string(card->effect.heal) + " HP";
            break;
    }

    DrawText(typeStr.c_str(), (int)(tooltipBox.x + 8.0f), (int)contentY, 12, DARKBLUE);
    DrawText(effectStr.c_str(), (int)(tooltipBox.x + 8.0f), (int)(contentY + 18.0f), 11, DARKGRAY);

    // Show mirrored effect if available
    if (card->mirroredEffect.type != CardType::Move || card->mirroredEffect.move.forward != 0 || card->mirroredEffect.move.lateral != 0) {
        std::string mirrorStr = "Mirrored: ";
        switch (card->mirroredEffect.type) {
            case CardType::Move:
                mirrorStr += "Lat: +" + std::to_string(card->mirroredEffect.move.lateral) + " Fwd: +" + std::to_string(card->mirroredEffect.move.forward);
                break;
            case CardType::Damage:
                mirrorStr += "Self: " + std::to_string(card->mirroredEffect.damage / 2) + " dmg";
                break;
            case CardType::Heal:
                mirrorStr += "All: +" + std::to_string(card->mirroredEffect.heal / 2) + " HP";
                break;
        }
        DrawText(mirrorStr.c_str(), (int)(tooltipBox.x + 8.0f), (int)(contentY + 36.0f), 10, DARKGREEN);
    }
}

UiActions UI_Draw(AppContext& ctx) {
    UiActions actions;
    int winW = ctx.window->GetWidth();
    int winH = ctx.window->GetHeight();

    // --- Phase Indicator (Top) ---
    float phaseBarHeight = 34.0f;
    Rectangle phaseRect = {10, 6, (float)winW - 20.0f, phaseBarHeight};
    DrawRectangleRec(phaseRect, Color{240, 240, 240, 255});
    DrawRectangleLinesEx(phaseRect, 1, DARKGRAY);

    Boss::Phase phase = Boss::Phase::PlayerSelect;
    phase = ctx.boss.getPhase();

    const char* labels[3] = {"User selects cards", "NPC selects cards", "Play phase"};
    for (int i = 0; i < 3; ++i) {
        float slotW = (phaseRect.width - 40.0f) / 3.0f;
        float cx = phaseRect.x + 20.0f + slotW * i + slotW * 0.1f;
        float cy = phaseRect.y + phaseRect.height * 0.5f;
        bool active = false;
        if (phase == Boss::Phase::PlayerSelect && i == 0) active = true;
        if (phase == Boss::Phase::NpcSelect && i == 1) active = true;
        if (phase == Boss::Phase::Play && i == 2) active = true;
        Color outline = DARKGRAY;
        DrawCircleLines((int)cx, (int)cy, 8.0f, outline);
        if (active) {
            DrawCircle((int)cx, (int)cy, 6.0f, BLACK);
        }
        DrawText(labels[i], (int)(cx + 14.0f), (int)(cy - 7.0f), 15, BLACK);
    }

    // --- Card Hand & Sequence Panel (Top) ---
    bool cardPanelCollapsed = (phase != Boss::Phase::PlayerSelect);
    const float panelHeightExpanded = 260.0f;
    const float panelHeightCollapsed = 52.0f;
    float cardPanelHeight = cardPanelCollapsed ? panelHeightCollapsed : panelHeightExpanded;
    Rectangle panelRect = {10, phaseRect.y + phaseRect.height + 8.0f, (float)winW - 20.0f, cardPanelHeight};
    GuiPanel(panelRect, "Card Hand & Turn Plan");

    // Mech selection (player mechs only for now)
        if (cardPanelCollapsed) {
        DrawText("Cards are hidden during AI/Play phase", (int)(panelRect.x + 14.0f), (int)(panelRect.y + 24.0f), 16, DARKGRAY);
        } else {
        std::vector<int> mechIds;
        for (const auto& e : ctx.game.entities) {
            if (e.type == PLAYER) mechIds.push_back(e.id);
            int selectedMech = ctx.game.lastSelectedMechId;
            if (selectedMech == -1 && !mechIds.empty()) {
                selectedMech = mechIds.front();
            }
            float mechBaseY = panelRect.y + 28.0f;
            float mechBaseX = panelRect.x + 12.0f;
            int mechCount = (int)std::min<size_t>(3, mechIds.size());

            // Mirror toggle for next pick
            bool mirror = ctx.game.mirrorNext;
            GuiToggle({mechBaseX, mechBaseY, 110.0f, 24.0f}, "Mirror Next", &mirror);
            ctx.game.mirrorNext = mirror;

            // Show pending card selection
            std::string pendingText = "Selected: ";
            if (ctx.game.pendingCardId != -1) {
                const Card* pc = nullptr;
                for (const auto& c : ctx.game.hand.cards) {
                    if (c.id == ctx.game.pendingCardId) { pc = &c; break; }
                }
                pendingText += pc ? pc->name : std::to_string(ctx.game.pendingCardId);
                if (ctx.game.pendingMirror) pendingText += " (M)";
            } else {
                pendingText += "(none)";
            }
            DrawText(pendingText.c_str(), (int)(mechBaseX + 130.0f), (int)(mechBaseY + 4.0f), 16, DARKGRAY);

            // Mech slots (assignment targets) with current card display
            float slotBaseY = mechBaseY + 32.0f;
            for (size_t i = 0; i < mechIds.size() && i < 3; ++i) {
                int mechId = mechIds[i];
                std::string slotLabel = "Mech " + std::to_string(mechId) + ": ";
                const PlanAssignment* slot = nullptr;
                for (const auto& a : ctx.game.currentPlan.assignments) {
                    if (a.mechId == mechId) { slot = &a; break; }
                }
                if (slot) {
                    const Card* c = nullptr;
                    for (const auto& hc : ctx.game.hand.cards) { if (hc.id == slot->cardId) { c = &hc; break; } }
                    slotLabel += c ? c->name : std::to_string(slot->cardId);
                    if (slot->useMirror) slotLabel += " (M)";
                } else {
                    slotLabel += "(empty)";
                }
                bool assign = GuiButton({mechBaseX + (float)i * 210.0f, slotBaseY, 200.0f, 26.0f}, slotLabel.c_str());
                if (assign) {
                    actions.assignCardToMech = mechId;
                }
            }

            // Hand buttons (card selection step)
            float cardBaseY = slotBaseY + 36.0f;
            float cardBaseX = panelRect.x + 12.0f;
            for (size_t i = 0; i < ctx.game.hand.cards.size(); ++i) {
                const Card& card = ctx.game.hand.cards[i];
                bool available = ctx.game.hand.canPlay(card.id);
                // Check if card already assigned
                int assignedMech = -1;
                for (const auto& a : ctx.game.currentPlan.assignments) {
                    if (a.cardId == card.id) { assignedMech = a.mechId; break; }
                }
                std::string label = card.name;
                if (assignedMech != -1) {
                    label += " (assigned to M" + std::to_string(assignedMech) + ")";
                } else if (!available) {
                    label += " (used)";
                }
                if (GuiButton({cardBaseX + (float)i * 130, cardBaseY, 125, 28}, label.c_str())) {
                    actions.selectCardId = card.id;
                    actions.mirrorNext = ctx.game.mirrorNext;
                }
            }

            // Plan display (player selections) with per-mech removal
            const float planWidth = 240.0f;
            const float planMargin = 18.0f;
            float mechRowRight = mechBaseX + (mechCount > 0 ? (mechCount - 1) * 210.0f + 200.0f : 0.0f);
            bool stackPlan = (panelRect.x + panelRect.width) - (mechRowRight + planMargin) < planWidth + 16.0f;
            float planBaseX = stackPlan ? mechBaseX : panelRect.x + panelRect.width - planWidth - 14.0f;
            planBaseX = std::max(planBaseX, mechRowRight + planMargin);
            float planMaxX = panelRect.x + panelRect.width - planWidth - 14.0f;
            planBaseX = std::min(planBaseX, planMaxX); // keep column inside panel
            float planBaseY = stackPlan ? cardBaseY + 40.0f : panelRect.y + 28.0f;
            DrawText("Plan:", static_cast<int>(planBaseX), static_cast<int>(planBaseY) - 18, 20, BLACK);
            if (ctx.game.currentPlan.assignments.empty()) {
                DrawText("(empty)", static_cast<int>(planBaseX), static_cast<int>(planBaseY), 18, DARKGRAY);
            } else {
                for (size_t i = 0; i < ctx.game.currentPlan.assignments.size(); ++i) {
                    const auto& a = ctx.game.currentPlan.assignments[i];
                    const Card* c = nullptr;
                    for (const auto& hc : ctx.game.hand.cards) {
                        if (hc.id == a.cardId) { c = &hc; break; }
                    }
                    std::string name = c ? c->name : std::to_string(a.cardId);
                    if (a.useMirror) name += " (M)";

                    std::string label = "M" + std::to_string(a.mechId) + ": " + name;
                    Rectangle labelRect = {planBaseX, planBaseY + static_cast<float>(i) * 26.0f, planWidth - 48.0f, 22.0f};
                    GuiLabel(labelRect, label.c_str());
                    Rectangle removeRect = {planBaseX + planWidth - 44.0f, planBaseY + static_cast<float>(i) * 26.0f, 30.0f, 22.0f};
                    if (GuiButton(removeRect, "X")) {
                        actions.removeAssignmentMechId = a.mechId;
                    }
                }
            }

            std::string turnText = "Turn: " + std::to_string(ctx.game.turnNumber);
            DrawText(turnText.c_str(), static_cast<int>(panelRect.x + panelRect.width - 120.0f), static_cast<int>(panelRect.y + 16.0f), 18, DARKGRAY);

            // Play / Clear buttons
            float actionY = panelRect.y + panelRect.height - 28.0f;
            if (GuiButton({cardBaseX, actionY, 100, 24}, "Confirm")) {
                actions.playSequence = true;
                actions.confirmPlan = true;
            }
            if (GuiButton({cardBaseX + 110.0f, actionY, 100, 24}, "Clear")) {
                actions.clearSequence = true;
            }
            if (GuiButton({cardBaseX + 220.0f, actionY, 120, 24}, "Undo Last")) {
                actions.undoLast = true;
            }
        }
    }

    // --- Render Controls Panel (Bottom, collapsible) ---
    const float headerHeight = 34.0f;
    const float expandedHeight = 200.0f;
    bool collapsed = ctx.ui.renderControlsCollapsed;
    float panelHeight = collapsed ? headerHeight : expandedHeight;
    Rectangle renderPanel = {10, (float)winH - panelHeight - 10, (float)winW - 20.0f, panelHeight};
    GuiPanel(renderPanel, "Render Controls");

    Rectangle toggleRect = {renderPanel.x + 10.0f, renderPanel.y + 8.0f, 170.0f, 20.0f};
    std::string headerLabel = collapsed ? "[+] Render Controls" : "[-] Render Controls";
    if (GuiButton(toggleRect, headerLabel.c_str())) {
        collapsed = !collapsed;
    }
    ctx.ui.renderControlsCollapsed = collapsed;

    if (!collapsed) {
        const float baseY = renderPanel.y + 20.0f;
        const float rowH = 24.0f;
        const float rowGap = 6.0f;
        const float colLeft = renderPanel.x + 16.0f;
        const float colMid = renderPanel.x + 230.0f;
        const float colRight = renderPanel.x + 520.0f;

        // Row 1: Entity/Environment/Light toggles (left column)
        Rectangle toggleEntities = {colLeft, baseY, 180.0f, rowH};
        GuiToggle(toggleEntities, "Render Entities", &ctx.ui.showEntities);
        Rectangle toggleEnv = {colLeft, baseY + rowH + rowGap, 180.0f, rowH};
        GuiToggle(toggleEnv, "Render Environment", &ctx.ui.showEnvironment);
        Rectangle toggleLight = {colLeft, baseY + 2*(rowH + rowGap), 180.0f, rowH};
        GuiToggle(toggleLight, "Light Indicator", &ctx.ui.showLightIndicator);

        // Row 1-3: AA/Bloom toggles (middle column)
        Rectangle checkbox1 = {colMid, baseY, 20.0f, 20.0f};
        GuiCheckBox(checkbox1, "MSAA-ish (Supersample 2x ~4x samples)", &ctx.ui.supersample2x);
        Rectangle checkbox2 = {colMid, baseY + rowH + rowGap, 20.0f, 20.0f};
        GuiCheckBox(checkbox2, "FXAA", &ctx.ui.fxaa);

        // Row 1-3: Post-processing toggles (right column)
        Rectangle toggleBloom = {colRight, baseY, 20.0f, 20.0f};
        GuiCheckBox(toggleBloom, "Enable Bloom", &ctx.ui.bloomEnabled);
        Rectangle togglePastel = {colRight, baseY + rowH + rowGap, 20.0f, 20.0f};
        GuiCheckBox(togglePastel, "Enable Pastel", &ctx.ui.pastelEnabled);
        Rectangle togglePalette = {colRight, baseY + 2*(rowH + rowGap), 20.0f, 20.0f};
        GuiCheckBox(togglePalette, "Enable Palette", &ctx.ui.paletteEnabled);
        
        // Sliders row (separate spacing)
        float sliderY = baseY + 3*(rowH + rowGap) + 8.0f;
        DrawText("Bloom Intensity:", (int)(colLeft), (int)(sliderY), 16, DARKGRAY);
        Rectangle sliderBloom = {colLeft + 140.0f, sliderY - 2.0f, 180.0f, 20.0f};
        GuiSlider(sliderBloom, nullptr, nullptr,  &ctx.ui.bloomIntensity, 0.0f, 2.0f);
        
        DrawText("Pastel Intensity:", (int)(colRight - 40.0f), (int)(sliderY), 16, DARKGRAY);
        Rectangle sliderPastel = {colRight + 120.0f, sliderY - 2.0f, 180.0f, 20.0f};
        GuiSlider(sliderPastel, nullptr, nullptr, &ctx.ui.pastelIntensity, 0.0f, 2.0f);

        float sliderY2 = sliderY + rowH + rowGap;
        DrawText("Palette Strength:", (int)(colRight - 40.0f), (int)(sliderY2), 16, DARKGRAY);
        Rectangle sliderPalette = {colRight + 120.0f, sliderY2 - 2.0f, 180.0f, 20.0f};
        GuiSlider(sliderPalette, nullptr, nullptr, &ctx.ui.paletteStrength, 0.0f, 1.0f);
    }

    return actions;
}
