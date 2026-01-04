#include "utils/meshGenerateUtils.h"
#include "utils/meshMathUtils.h"
#include "utils/meshProcessUtils.h"
#include "raymath.h"
#include <cmath>
#include <vector>
#include <map>

namespace MeshGenerator {

Mesh createCubeMesh(float size) {
    return GenMeshCube(size, size, size);
}

Mesh createSphereMesh(float radius, int rings, int slices) {
    return GenMeshSphere(radius, rings, slices);
}

Mesh createCylinderMesh(float radius, float height, int slices) {
    return GenMeshCylinder(radius, height, slices);
}

Mesh createLowPolySphereMesh(float radius, int rings, int slices, float noiseAmount) {
    // Start from a regular sphere
    Mesh mesh = GenMeshSphere(radius, rings, slices);

    // Raylib stores vertices as a flat float array (x, y, z) * vertexCount
    float* verts = mesh.vertices;
    if (!verts || mesh.vertexCount <= 0 || noiseAmount <= 0.0f) return mesh;

    for (int i = 0; i < mesh.vertexCount; ++i) {
        int idx = i * 3;
        float x = verts[idx + 0];
        float y = verts[idx + 1];
        float z = verts[idx + 2];

        float len = std::sqrt(x * x + y * y + z * z);
        if (len > 0.0001f) {
            // Slightly vary radius per vertex, centered around 1.0
            float r = MeshUtils::pseudoRandom01(i);          // [0,1]
            float offset = (r - 0.5f) * noiseAmount; // [-noiseAmount/2, noiseAmount/2]
            float scale = (len + offset) / len;

            verts[idx + 0] = x * scale;
            verts[idx + 1] = y * scale;
            verts[idx + 2] = z * scale;
        }
    }

    // We keep the original normals for a slightly faceted but still coherent look.
    // For even harsher facets, we could recompute flat normals per triangle.

    return mesh;
}

Mesh createCustomOctahedron(float radius, int subdivisions)
{
    MeshUtils::PolySoup soup;
    Vector3 baseVertices[6] = {
        { 1,  0,  0}, {-1,  0,  0}, { 0,  1,  0}, { 0, -1,  0}, { 0,  0,  1}, { 0,  0, -1}
    };
    soup.verts.assign(baseVertices, baseVertices + 6);
    soup.indices = {
        0, 2, 4,   2, 1, 4,   1, 3, 4,   3, 0, 4,
        0, 3, 5,   3, 1, 5,   1, 2, 5,   2, 0, 5
    };
    MeshUtils::PolySoup refined = MeshUtils::subdivideSoup(soup, MeshUtils::clampSubdiv(subdivisions), true);
    return MeshUtils::bakeSoupToSphere(refined, radius);
}

Mesh createCustomTetrahedron(float radius, int subdivisions)
{
    MeshUtils::PolySoup soup;
    const float s = 1.0f;
    Vector3 baseVertices[4] = {
        {  s,  s,  s}, { -s, -s,  s}, { -s,  s, -s}, {  s, -s, -s}
    };
    soup.verts.assign(baseVertices, baseVertices + 4);
    soup.indices = { 0,1,2, 0,3,1, 0,2,3, 1,3,2 };
    MeshUtils::PolySoup refined = MeshUtils::subdivideSoup(soup, MeshUtils::clampSubdiv(subdivisions), true);
    return MeshUtils::bakeSoupToSphere(refined, radius);
}

Mesh createCustomCubeSphere(float radius, int subdivisions)
{
    MeshUtils::PolySoup soup;
    Vector3 baseVertices[8] = {
        {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},
        {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1}
    };
    soup.verts.assign(baseVertices, baseVertices + 8);
    soup.indices = {
        0,1,2,  0,2,3,  // front
        5,4,7,  5,7,6,  // back
        4,0,3,  4,3,7,  // left
        1,5,6,  1,6,2,  // right
        3,2,6,  3,6,7,  // top
        4,5,1,  4,1,0   // bottom
    };
    MeshUtils::PolySoup refined = MeshUtils::subdivideSoup(soup, MeshUtils::clampSubdiv(subdivisions));
    return MeshUtils::bakeSoupToSphere(refined, radius);
}


// ----------------------------------------------------------------------------
// 3. COMBINER - The "Glue" to join shapes
//    Combines two meshes into one new mesh, applying a transform to the second.
// ----------------------------------------------------------------------------
Mesh combineMeshes(Mesh base, Mesh add, Matrix transform) {
    Mesh mesh = { 0 };

    // Calculate totals
    mesh.vertexCount = base.vertexCount + add.vertexCount;
    mesh.triangleCount = base.triangleCount + add.triangleCount;

    // Allocate new buffers
    mesh.vertices = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.normals = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float*)MemAlloc(mesh.vertexCount * 2 * sizeof(float));
    
    // We assume both meshes have indices. If not, you'd need to generate them.
    if (base.indices && add.indices) {
        mesh.indices = (unsigned short*)MemAlloc(mesh.triangleCount * 3 * sizeof(unsigned short));
    }

    // --- COPY BASE MESH (No Transform) ---
    // Vertices/Normals
    memcpy(mesh.vertices, base.vertices, base.vertexCount * 3 * sizeof(float));
    memcpy(mesh.normals, base.normals, base.vertexCount * 3 * sizeof(float));
    // Texcoords (Check if exist)
    if (base.texcoords) memcpy(mesh.texcoords, base.texcoords, base.vertexCount * 2 * sizeof(float));
    // Indices
    if (base.indices && mesh.indices) memcpy(mesh.indices, base.indices, base.triangleCount * 3 * sizeof(unsigned short));


    // --- COPY ADD MESH (With Transform) ---
    int vOffset = base.vertexCount;
    int iOffset = base.triangleCount * 3;

    for (int i = 0; i < add.vertexCount; i++) {
        // Transform Vertex
        Vector3 v = { add.vertices[i*3], add.vertices[i*3+1], add.vertices[i*3+2] };
        Vector3 vt = Vector3Transform(v, transform);
        
        mesh.vertices[(vOffset + i)*3 + 0] = vt.x;
        mesh.vertices[(vOffset + i)*3 + 1] = vt.y;
        mesh.vertices[(vOffset + i)*3 + 2] = vt.z;

        // Transform Normal (Rotate only, no translation)
        Vector3 n = { add.normals[i*3], add.normals[i*3+1], add.normals[i*3+2] };
        // Cleanest way to rotate normal is usually removing translation from matrix
        // For simplicity here, we assume uniform scale. If non-uniform, use InverseTranspose.
        Matrix rotOnly = transform;
        rotOnly.m12 = 0; rotOnly.m13 = 0; rotOnly.m14 = 0;
        Vector3 nt = Vector3Transform(n, rotOnly);
        
        mesh.normals[(vOffset + i)*3 + 0] = nt.x;
        mesh.normals[(vOffset + i)*3 + 1] = nt.y;
        mesh.normals[(vOffset + i)*3 + 2] = nt.z;

        // Texcoords
        if (add.texcoords && mesh.texcoords) {
            mesh.texcoords[(vOffset + i)*2 + 0] = add.texcoords[i*2];
            mesh.texcoords[(vOffset + i)*2 + 1] = add.texcoords[i*2 + 1];
        }
    }

    // Merge Indices and offset them
    if (add.indices && mesh.indices) {
        for (int i = 0; i < add.triangleCount * 3; i++) {
            mesh.indices[iOffset + i] = add.indices[i] + (unsigned short)vOffset;
        }
    }

    mesh.vboId = NULL;
    return mesh;
}
} // namespace MeshGenerator
