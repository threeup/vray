#pragma once

#include "raylib.h"
#include <vector>
#include <map>

class MeshGenerator {
public:
    static Mesh createCubeMesh(float size);
    static Mesh createSphereMesh(float radius, int rings, int slices);
    static Mesh createCylinderMesh(float radius, float height, int slices);
    // Low-poly / geodesic-style sphere by perturbing a base sphere mesh
    static Mesh createLowPolySphereMesh(float radius, int rings, int slices, float noiseAmount);
    // Abstract / stylized forms
    static Mesh createSpikyBlobMesh(float radius, int rings, int slices, float spikeAmount);
    static Mesh createTwistedColumnMesh(float radius, float height, int slices, int twistTurns);
    // Custom icosphere with flat shading support
    static Mesh createCustomIcosphere(float radius, int subdivisions);
    // Custom octahedron sphere with flat shading support (coarser base than icosphere)
    static Mesh createCustomOctahedron(float radius, int subdivisions);
    // Additional common platonic/geodesic variants
    static Mesh createCustomTetrahedron(float radius, int subdivisions);
    static Mesh createCustomCubeSphere(float radius, int subdivisions);
    static Mesh createCustomDodecahedron(float radius);
    static Mesh createCustomPentagonalPrismSphere(float radius, int rings);
    static Mesh createSquareTree(float radius, int cubeSubdivisions, int octaSubdivisions);
    static Mesh createCubicStar(float radius, int cubeSubdivisions);
};

// Mesh utility functions for normal and vertex processing
namespace MeshUtils {
    void computeMeshNormals(Mesh *mesh);
    void unshareMeshVertices(Mesh *mesh);
}