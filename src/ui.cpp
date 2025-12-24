#include "ui.h"
#include "app.h"
#include "game.h"
#include "raygui.h"
#include <string>

UiActions UI_Draw(AppContext& ctx) {
    UiActions actions;
    int winW = ctx.window->GetWidth();
    int winH = ctx.window->GetHeight();

    // --- Card Hand & Sequence Panel (Top) ---
    Rectangle panelRect = {10, 10, 780, 150};
    GuiPanel(panelRect, "Card Hand & Sequence");

    // Hand buttons
    if (ctx.game) {
        for (size_t i = 0; i < ctx.game->hand.size(); ++i) {
            if (GuiButton({20 + (float)i * 120, 40, 100, 30}, ctx.game->hand[i].name.c_str())) {
                ctx.game->currentSeq.push_back(ctx.game->hand[i]);
            }
        }

        // Sequence display
        std::string seqText = "Sequence: ";
        for (const auto& c : ctx.game->currentSeq) {
            seqText += c.name + " ";
        }
        DrawText(seqText.c_str(), 20, 80, 20, BLACK);

        // Play / Clear buttons
        if (GuiButton({20, 110, 100, 30}, "Play Sequence")) {
            actions.playSequence = true;
        }
        if (GuiButton({130, 110, 100, 30}, "Clear")) {
            actions.clearSequence = true;
        }
    }

    // --- Render Controls Panel (Bottom) ---
    float panelHeight = 170.0f;
    Rectangle renderPanel = {10, (float)winH - panelHeight - 10, (float)winW - 20.0f, panelHeight};
    GuiPanel(renderPanel, "Render Controls");
    
    // Row 1: Entity/Environment toggles
    Rectangle toggleEntities = {renderPanel.x + 20.0f, renderPanel.y + 25.0f, 180.0f, 20.0f};
    GuiToggle(toggleEntities, "Render Entities", &ctx.ui.showEntities);
    Rectangle toggleEnv = {renderPanel.x + 20.0f, renderPanel.y + 50.0f, 180.0f, 20.0f};
    GuiToggle(toggleEnv, "Render Environment", &ctx.ui.showEnvironment);

    // Row 2: AA and Bloom toggles
    Rectangle checkbox1 = {renderPanel.x + 220.0f, renderPanel.y + 25.0f, 20.0f, 20.0f};
    GuiCheckBox(checkbox1, "MSAA-ish (Supersample 2x ~4x samples)", &ctx.ui.supersample2x);
    Rectangle checkbox2 = {renderPanel.x + 220.0f, renderPanel.y + 50.0f, 20.0f, 20.0f};
    GuiCheckBox(checkbox2, "FXAA", &ctx.ui.fxaa);
    
    // Row 3: Post-processing toggles
    Rectangle toggleBloom = {renderPanel.x + 520.0f, renderPanel.y + 25.0f, 20.0f, 20.0f};
    GuiCheckBox(toggleBloom, "Enable Bloom", &ctx.ui.bloomEnabled);
    Rectangle togglePastel = {renderPanel.x + 520.0f, renderPanel.y + 50.0f, 20.0f, 20.0f};
    GuiCheckBox(togglePastel, "Enable Pastel", &ctx.ui.pastelEnabled);
    
    // Row 4: Bloom and Pastel intensity sliders
    Rectangle sliderBloomLabel = {renderPanel.x + 20.0f, renderPanel.y + 80.0f, 100.0f, 20.0f};
    DrawText("Bloom Intensity:", (int)sliderBloomLabel.x, (int)sliderBloomLabel.y, 16, DARKGRAY);
    Rectangle sliderBloom = {renderPanel.x + 150.0f, renderPanel.y + 80.0f, 150.0f, 20.0f};
    GuiSlider(sliderBloom, nullptr, nullptr,  &ctx.ui.bloomIntensity, 0.0f, 2.0f);
    
    Rectangle sliderPastelLabel = {renderPanel.x + 320.0f, renderPanel.y + 80.0f, 100.0f, 20.0f};
    DrawText("Pastel Intensity:", (int)sliderPastelLabel.x, (int)sliderPastelLabel.y, 16, DARKGRAY);
    Rectangle sliderPastel = {renderPanel.x + 450.0f, renderPanel.y + 80.0f, 150.0f, 20.0f};
    GuiSlider(sliderPastel, nullptr, nullptr, &ctx.ui.pastelIntensity, 0.0f, 2.0f);

    return actions;
}
