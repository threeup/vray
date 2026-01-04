#include "utils/meshGenerateUtils.h"
#include "utils/meshMathUtils.h"
#include "utils/meshProcessUtils.h"
#include <cmath>
#include <vector>
#include <map>

#include "raylib.h"
#include "raymath.h"
namespace MeshGenerator {

Mesh createBarbellMesh() {

    // 1. Create parts
    Mesh weightL = createSphereMesh(1.0f, 16, 16);
    Mesh weightR = createSphereMesh(1.0f, 16, 16);
    Mesh handle  = createCylinderMesh(0.2f, 4.0f, 12);

    // 2. Define transforms
    // Move Left Weight to -2.0
    Matrix matL = MatrixTranslate(-2.0f, 0, 0);
    // Move Right Weight to +2.0
    Matrix matR = MatrixTranslate(2.0f, 0, 0);
    // Rotate handle to lie flat (Cylinder usually defaults to standing Y)
    Matrix matH = MatrixRotateZ(PI / 2.0f);

    // 3. Combine
    // First combine Left Weight + Handle
    Mesh temp = combineMeshes(weightL, handle, matH); // Note: Logic depends on which you treat as "base"
    // Actually, better: combine Handle (base) + WeightL (offset)
    Mesh step1 = combineMeshes(handle, weightL, matL);
    // Then combine that result + WeightR (offset)
    Mesh barbell = combineMeshes(step1, weightR, matR);
    return barbell;
}

Mesh createSquareTree(float radius, int cubeSubdivisions, int octaSubdivisions)
{
    MeshUtils::PolySoup base;
    
    // Create normal cube (not squashed)
    Vector3 baseVerts[8] = {
        {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},
        {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1}
    };
    base.verts.assign(baseVerts, baseVerts + 8);
    base.indices = {
        // front (z = -1)
        0,2,1,  0,3,2,
        // back (z = 1)
        4,5,7,  5,6,7,
        // left (x = -1)
        4,3,0,  4,7,3,
        // right (x = 1)
        1,2,5,  2,6,5,
        // top (y = 1)
        3,6,2,  3,7,6,
        // bottom (y = -1)
        4,1,5,  4,0,1
    };

    // Scale base into a trunk-like rectangular prism (planar faces, no normalization)
    MeshUtils::PolySoup baseSubdiv = MeshUtils::subdivideSoup(base, MeshUtils::clampSubdiv(cubeSubdivisions), false);
    const float trunkScaleX = radius * 0.37f;
    const float trunkScaleY = radius * 0.85f;
    const float trunkScaleZ = radius * 0.37f;
    for (auto& v : baseSubdiv.verts) {
        v.x *= trunkScaleX;
        v.y *= trunkScaleY;
        v.z *= trunkScaleZ;
    }

    // Build a rounded octahedron canopy perched on top of the cube
    MeshUtils::PolySoup canopy;
    Vector3 octVerts[6] = {
        { 1,  0,  0}, {-1,  0,  0}, { 0,  1,  0}, { 0, -1,  0}, { 0,  0,  1}, { 0,  0, -1}
    };
    canopy.verts.assign(octVerts, octVerts + 6);
    canopy.indices = {
        0, 2, 4,   2, 1, 4,   1, 3, 4,   3, 0, 4,
        0, 3, 5,   3, 1, 5,   1, 2, 5,   2, 0, 5
    };

    MeshUtils::PolySoup canopySubdiv = MeshUtils::subdivideSoup(canopy, MeshUtils::clampSubdiv(octaSubdivisions), true);
    float canopyRadius = radius * 1.05f;
    float canopyOffsetY = trunkScaleY + canopyRadius * 0.6f; // keep canopy dominant and nestled
    for (auto& v : canopySubdiv.verts) {
        float len = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
        if (len > 0.0001f) { v.x /= len; v.y /= len; v.z /= len; }
        v.x *= canopyRadius;
        v.y = v.y * canopyRadius + canopyOffsetY;
        v.z *= canopyRadius;
    }

    MeshUtils::PolySoup merged = MeshUtils::mergeSoups(baseSubdiv, canopySubdiv);

    // Convert merged soup to mesh
    Mesh mesh = { 0 };
    int numTriangles = (int)merged.indices.size() / 3;
    if (numTriangles <= 0) return mesh;

    mesh.triangleCount = numTriangles;
    mesh.vertexCount = numTriangles * 3;

    mesh.vertices = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.normals  = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = mesh.texcoords2 = mesh.tangents = nullptr;
    mesh.boneIds = mesh.colors = nullptr;
    mesh.indices = nullptr;
    mesh.boneWeights = mesh.animVertices = mesh.animNormals = nullptr;
    mesh.vaoId = 0; mesh.vboId = NULL;

    for (int i = 0; i < numTriangles * 3; i++) {
        Vector3 v = merged.verts[merged.indices[i]];
        mesh.vertices[i*3+0] = v.x;
        mesh.vertices[i*3+1] = v.y;
        mesh.vertices[i*3+2] = v.z;
    }

    MeshUtils::computeMeshNormals(&mesh);
    return mesh;
}

Mesh createCubicStar(float radius, int cubeSubdivisions)
{
    MeshUtils::PolySoup base;
    Vector3 baseVerts[8] = {
        {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},
        {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1}
    };
    base.verts.assign(baseVerts, baseVerts + 8);
    base.indices = {
        // front (z = -1)
        0,2,1,  0,3,2,
        // back (z = 1)
        4,5,7,  5,6,7,
        // left (x = -1)
        4,3,0,  4,7,3,
        // right (x = 1)
        1,2,5,  2,6,5,
        // top (y = 1)
        3,6,2,  3,7,6,
        // bottom (y = -1)
        4,1,5,  4,0,1
    };

    // Use normalized midpoint subdivision to pull faces outward for a stellated look
    MeshUtils::PolySoup baseSubdiv = MeshUtils::subdivideSoup(base, MeshUtils::clampSubdiv(cubeSubdivisions), true);
    for (auto& v : baseSubdiv.verts) {
        // Project each vertex toward unit sphere for star-like spikes
        float len = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
        if (len > 0.0001f) { v.x /= len; v.y /= len; v.z /= len; }
        v.x *= radius;
        v.y *= radius;
        v.z *= radius;
    }

    Mesh mesh = { 0 };
    int numTriangles = (int)baseSubdiv.indices.size() / 3;
    if (numTriangles <= 0) return mesh;

    mesh.triangleCount = numTriangles;
    mesh.vertexCount = numTriangles * 3;

    mesh.vertices = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.normals  = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = mesh.texcoords2 = mesh.tangents = nullptr;
    mesh.boneIds = mesh.colors = nullptr;
    mesh.indices = nullptr;
    mesh.boneWeights = mesh.animVertices = mesh.animNormals = nullptr;
    mesh.vaoId = 0; mesh.vboId = NULL;

    for (int i = 0; i < numTriangles * 3; i++) {
        Vector3 v = baseSubdiv.verts[baseSubdiv.indices[i]];
        mesh.vertices[i*3+0] = v.x;
        mesh.vertices[i*3+1] = v.y;
        mesh.vertices[i*3+2] = v.z;
    }

    MeshUtils::computeMeshNormals(&mesh);
    return mesh;
}

} // namespace MeshGenerator