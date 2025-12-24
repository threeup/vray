#include "character_renderer.h"
#include "platform/interface/renderer_interface.h"

// Simple Tunic-inspired low-poly fox adventurer made from cubes
void DrawFoxAdventurer(RendererInterface* renderer, const Vector3& basePos) {
    // Pastel-ish color palette
    Color foxOrange = { 255, 180, 120, 255 };
    Color foxWhite  = { 245, 240, 235, 255 };
    Color cloakMint = { 160, 220, 210, 255 };
    Color beltBrown = { 140, 110, 90, 255 };
    Color swordMetal = { 210, 215, 230, 255 };
    Color swordHilt  = { 170, 140, 110, 255 };

    // Scale for the whole character
    float s = 0.8f;

    // Body (slightly rectangular)
    Vector3 bodyPos = { basePos.x, basePos.y + 0.6f * s, basePos.z };
    renderer->DrawCube(bodyPos, 0.7f * s, 0.9f * s, 0.4f * s, foxOrange);

    // Cloak (slightly bigger, thinner cube)
    Vector3 cloakPos = { bodyPos.x, bodyPos.y, bodyPos.z - 0.02f };
    renderer->DrawCube(cloakPos, 0.8f * s, 0.95f * s, 0.35f * s, cloakMint);

    // Head (oversized cube)
    Vector3 headPos = { basePos.x, basePos.y + 1.3f * s, basePos.z };
    renderer->DrawCube(headPos, 0.9f * s, 0.8f * s, 0.7f * s, foxOrange);

    // Muzzle (white front part)
    Vector3 muzzlePos = { headPos.x, headPos.y - 0.1f * s, headPos.z - 0.4f * s };
    renderer->DrawCube(muzzlePos, 0.5f * s, 0.3f * s, 0.2f * s, foxWhite);

    // Ears (two small tall cubes)
    Vector3 leftEarPos  = { headPos.x - 0.25f * s, headPos.y + 0.45f * s, headPos.z - 0.05f * s };
    Vector3 rightEarPos = { headPos.x + 0.25f * s, headPos.y + 0.45f * s, headPos.z - 0.05f * s };
    renderer->DrawCube(leftEarPos, 0.18f * s, 0.4f * s, 0.18f * s, foxOrange);
    renderer->DrawCube(rightEarPos, 0.18f * s, 0.4f * s, 0.18f * s, foxOrange);

    // Legs (four little cubes)
    float legOffsetX = 0.25f * s;
    float legOffsetZ = 0.18f * s;
    float legHeight  = 0.35f * s;
    float legWidth   = 0.18f * s;
    Vector3 legFL = { basePos.x - legOffsetX, basePos.y + legHeight * 0.5f, basePos.z - legOffsetZ };
    Vector3 legFR = { basePos.x + legOffsetX, basePos.y + legHeight * 0.5f, basePos.z - legOffsetZ };
    Vector3 legBL = { basePos.x - legOffsetX, basePos.y + legHeight * 0.5f, basePos.z + legOffsetZ };
    Vector3 legBR = { basePos.x + legOffsetX, basePos.y + legHeight * 0.5f, basePos.z + legOffsetZ };
    renderer->DrawCube(legFL, legWidth, legHeight, legWidth, foxWhite);
    renderer->DrawCube(legFR, legWidth, legHeight, legWidth, foxWhite);
    renderer->DrawCube(legBL, legWidth, legHeight, legWidth, foxWhite);
    renderer->DrawCube(legBR, legWidth, legHeight, legWidth, foxWhite);

    // Tail, belt, and sword
    Vector3 tailPos = { basePos.x, basePos.y + 0.7f * s, basePos.z + 0.4f * s };
    renderer->DrawCube(tailPos, 0.18f * s, 0.18f * s, 0.7f * s, foxOrange);
    Vector3 beltPos = { bodyPos.x, bodyPos.y - 0.15f * s, bodyPos.z };
    renderer->DrawCube(beltPos, 0.75f * s, 0.12f * s, 0.42f * s, beltBrown);
    Vector3 hiltPos  = { basePos.x + 0.6f * s, basePos.y + 0.7f * s, basePos.z };
    renderer->DrawCube(hiltPos, 0.15f * s, 0.25f * s, 0.15f * s, swordHilt); // hilt
    Vector3 bladePos = { basePos.x + 0.9f * s, basePos.y + 0.9f * s, basePos.z };
    renderer->DrawCube(bladePos, 0.12f * s, 0.7f * s, 0.12f * s, swordMetal); // blade
}