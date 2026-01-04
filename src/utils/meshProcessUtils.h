#pragma once

#include "raylib.h"

namespace MeshUtils {

// Compute per-triangle flat normals for the mesh
// Allocates normals buffer if not present
void computeMeshNormals(Mesh* mesh);

// Unshare mesh vertices to create hard edges per face (flat/faceted shading)
// Converts indexed mesh to non-indexed, expanding vertices
void unshareMeshVertices(Mesh* mesh);

} // namespace MeshUtils
