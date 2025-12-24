#pragma once

#include "raylib.h" // For Vector3

// Forward-declare the interface to avoid including the full header
class RendererInterface;

// Renders the fox adventurer character model
void DrawFoxAdventurer(RendererInterface* renderer, const Vector3& basePos);