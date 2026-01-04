#pragma once

#include "raylib.h"
#include <vector>
#include <map>

namespace MeshGenerator {

// Basic raylib mesh generation wrappers
Mesh createCubeMesh(float size);
Mesh createSphereMesh(float radius, int rings, int slices);
Mesh createCylinderMesh(float radius, float height, int slices);

// Low-poly / geodesic-style sphere by perturbing a base sphere mesh
Mesh createLowPolySphereMesh(float radius, int rings, int slices, float noiseAmount);

// Abstract / stylized forms (in meshExtraShapeUtils.cpp)
Mesh createSpikyBlobMesh(float radius, int rings, int slices, float spikeAmount);
Mesh createTwistedColumnMesh(float radius, float height, int slices, int twistTurns);

// Custom icosphere with flat shading support
Mesh createCustomIcosphere(float radius, int subdivisions);

// Custom octahedron sphere with flat shading support (coarser base than icosphere)
Mesh createCustomOctahedron(float radius, int subdivisions);

// Additional common platonic/geodesic variants
Mesh createCustomTetrahedron(float radius, int subdivisions);
Mesh createCustomCubeSphere(float radius, int subdivisions);
Mesh createCustomDodecahedron(float radius);
Mesh createCustomPentagonalPrismSphere(float radius, int rings);

// Extra / complex shapes (in meshExtraShapeUtils.cpp)
Mesh createTorusMesh(float radius, float size, int radSeg, int sides);
Mesh createCapsuleMesh(float radius, float height, int rings, int slices);

// Composite meshes - joining multiple shapes (in meshCompositeUtils.cpp)
Mesh createSquareTree(float radius, int cubeSubdivisions, int octaSubdivisions);
Mesh createCubicStar(float radius, int cubeSubdivisions);
Mesh createBarbellMesh();

// Helper for combining two meshes with a transform (in meshGenerateUtils.cpp)
Mesh combineMeshes(Mesh base, Mesh add, Matrix transform);

} // namespace MeshGenerator
