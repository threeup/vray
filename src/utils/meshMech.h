#pragma once
#include "raylib.h"
#include <string>

// Build a single merged mech mesh (matte shading applied by caller's shader)
// Variants: "alpha" (chunky), "bravo" (default), "charlie" (sleek)
Mesh CreateMechMesh(const std::string& variant = "bravo");
