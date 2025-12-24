#include "mesh.h"
#include <cmath>
#include <vector>
#include <map>

// Simple deterministic pseudo-random helper based on an integer index
static float pseudoRandom01(int i)
{
    // LCG-style hash, keep it cheap and deterministic
    unsigned int x = static_cast<unsigned int>(i * 747796405u + 2891336453u);
    x ^= x >> 13;
    x *= 1597334677u;
    x ^= x >> 16;
    return (x & 0xFFFFFF) / static_cast<float>(0xFFFFFF); // [0,1]
}

Mesh MeshGenerator::createCubeMesh(float size) {
    return GenMeshCube(size, size, size);
}

Mesh MeshGenerator::createSphereMesh(float radius, int rings, int slices) {
    return GenMeshSphere(radius, rings, slices);
}

Mesh MeshGenerator::createCylinderMesh(float radius, float height, int slices) {
    return GenMeshCylinder(radius, height, slices);
}

Mesh MeshGenerator::createLowPolySphereMesh(float radius, int rings, int slices, float noiseAmount) {
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
            float r = pseudoRandom01(i);          // [0,1]
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

Mesh MeshGenerator::createSpikyBlobMesh(float radius, int rings, int slices, float spikeAmount) {
    // Start from a low-poly sphere, then push vertices outward with stronger noise
    Mesh mesh = GenMeshSphere(radius, rings, slices);
    float* verts = mesh.vertices;
    if (!verts || mesh.vertexCount <= 0 || spikeAmount <= 0.0f) return mesh;

    for (int i = 0; i < mesh.vertexCount; ++i) {
        int idx = i * 3;
        float x = verts[idx + 0];
        float y = verts[idx + 1];
        float z = verts[idx + 2];
        float len = std::sqrt(x * x + y * y + z * z);
        if (len > 0.0001f) {
            float r = pseudoRandom01(i * 17 + 3); // scramble a bit differently
            float offset = (r - 0.5f) * spikeAmount; // [-spike/2, spike/2]
            float scale = (len + offset) / len;
            verts[idx + 0] = x * scale;
            verts[idx + 1] = y * scale;
            verts[idx + 2] = z * scale;
        }
    }
    return mesh;
}

Mesh MeshGenerator::createTwistedColumnMesh(float radius, float height, int slices, int twistTurns) {
    // Use a cylinder base, then twist vertices along Y
    Mesh mesh = GenMeshCylinder(radius, height, slices);
    float* verts = mesh.vertices;
    if (!verts || mesh.vertexCount <= 0 || twistTurns == 0) return mesh;

    float halfH = height * 0.5f;
    float twistTotal = static_cast<float>(twistTurns) * 2.0f * PI;

    for (int i = 0; i < mesh.vertexCount; ++i) {
        int idx = i * 3;
        float x = verts[idx + 0];
        float y = verts[idx + 1];
        float z = verts[idx + 2];

        // Normalize y to [-1,1] based on height; avoid divide-by-zero
        float t = (halfH != 0.0f) ? (y / halfH) : 0.0f;
        float angle = atan2f(z, x) + twistTotal * 0.5f * t; // twist more towards top
        float r = std::sqrt(x * x + z * z);
        verts[idx + 0] = r * cosf(angle);
        verts[idx + 2] = r * sinf(angle);
    }

    return mesh;
}

// ---- Shared poly-soup helpers ----

namespace {
    struct PolySoup {
        std::vector<Vector3> verts;
        std::vector<int> indices; // multiples of 3
    };

    inline int clampSubdiv(int s) {
        return (s < 0) ? 0 : (s > 2) ? 2 : s;
    }

    inline int midpointIndex(int a, int b, std::vector<Vector3>& verts, std::map<std::pair<int,int>, int>& edgeMap, bool normalize) {
        if (a > b) std::swap(a, b);
        auto key = std::make_pair(a, b);
        auto it = edgeMap.find(key);
        if (it != edgeMap.end()) return it->second;
        Vector3 mid = {
            (verts[a].x + verts[b].x) * 0.5f,
            (verts[a].y + verts[b].y) * 0.5f,
            (verts[a].z + verts[b].z) * 0.5f
        };
        if (normalize) {
            float len = sqrtf(mid.x*mid.x + mid.y*mid.y + mid.z*mid.z);
            if (len > 0.0001f) { mid.x /= len; mid.y /= len; mid.z /= len; }
        }
        int idx = (int)verts.size();
        verts.push_back(mid);
        edgeMap[key] = idx;
        return idx;
    }

    PolySoup subdivideSoup(const PolySoup& soup, int levels, bool normalizeMidpoints = true) {
        PolySoup current = soup;
        for (int l = 0; l < levels; l++) {
            std::vector<int> newIndices;
            newIndices.reserve(current.indices.size() * 4);
            std::map<std::pair<int,int>, int> edgeMap;

            for (size_t tri = 0; tri < current.indices.size(); tri += 3) {
                int i0 = current.indices[tri];
                int i1 = current.indices[tri + 1];
                int i2 = current.indices[tri + 2];

                int m01 = midpointIndex(i0, i1, current.verts, edgeMap, normalizeMidpoints);
                int m12 = midpointIndex(i1, i2, current.verts, edgeMap, normalizeMidpoints);
                int m20 = midpointIndex(i2, i0, current.verts, edgeMap, normalizeMidpoints);

                newIndices.insert(newIndices.end(), {
                    i0, m01, m20,
                    i1, m12, m01,
                    i2, m20, m12,
                    m01, m12, m20
                });
            }
            current.indices.swap(newIndices);
        }
        return current;
    }

    PolySoup mergeSoups(const PolySoup& a, const PolySoup& b) {
        PolySoup out = a;
        int offset = (int)out.verts.size();
        out.verts.insert(out.verts.end(), b.verts.begin(), b.verts.end());
        out.indices.reserve(out.indices.size() + b.indices.size());
        for (int idx : b.indices) out.indices.push_back(idx + offset);
        return out;
    }

    Mesh bakeSoupToSphere(const PolySoup& soup, float radius) {
        Mesh mesh = { 0 };
        int numTriangles = (int)soup.indices.size() / 3;
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
            Vector3 v = soup.verts[soup.indices[i]];
            float len = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
            if (len > 0.0001f) {
                mesh.vertices[i*3+0] = (v.x/len) * radius;
                mesh.vertices[i*3+1] = (v.y/len) * radius;
                mesh.vertices[i*3+2] = (v.z/len) * radius;
            } else {
                mesh.vertices[i*3+0] = mesh.vertices[i*3+1] = mesh.vertices[i*3+2] = 0.0f;
            }
        }

        MeshUtils::computeMeshNormals(&mesh);
        return mesh;
    }
}

Mesh MeshGenerator::createCustomIcosphere(float radius, int subdivisions)
{
    PolySoup soup;
    float t = (1.0f + sqrtf(5.0f)) / 2.0f;
    Vector3 baseVertices[12] = {
        {-1,  t,  0}, { 1,  t,  0}, {-1, -t,  0}, { 1, -t,  0},
        { 0, -1,  t}, { 0,  1,  t}, { 0, -1, -t}, { 0,  1, -t},
        { t,  0, -1}, { t,  0,  1}, {-t,  0, -1}, {-t,  0,  1}
    };
    soup.verts.assign(baseVertices, baseVertices + 12);
    soup.indices = {
        0,11,5,  0,5,1,  0,1,7,  0,7,10, 0,10,11,
        1,5,9,   5,11,4, 11,10,2, 10,7,6, 7,1,8,
        3,9,4,   3,4,2,  3,2,6,  3,6,8,  3,8,9,
        4,9,5,   2,4,11, 6,2,10, 8,6,7,  9,8,1
    };

    int levels = clampSubdiv(subdivisions);
    PolySoup refined = subdivideSoup(soup, levels);
    return bakeSoupToSphere(refined, radius);
}
void MeshUtils::computeMeshNormals(Mesh *mesh)
{
    // Safety checks
    if (mesh == NULL || mesh->vertices == NULL || mesh->triangleCount <= 0)
        return;

    int vCount = mesh->triangleCount * 3;

    // Allocate normals if needed
    if (mesh->normals == NULL)
        mesh->normals = (float*)MemAlloc(vCount * 3 * sizeof(float));

    // Clear normals (Raylib expects clean memory)
    memset(mesh->normals, 0, vCount * 3 * sizeof(float));

    for (int tri = 0; tri < mesh->triangleCount; tri++)
    {
        int i = tri * 3;

        Vector3 v1 = { mesh->vertices[(i+0)*3+0], mesh->vertices[(i+0)*3+1], mesh->vertices[(i+0)*3+2] };
        Vector3 v2 = { mesh->vertices[(i+1)*3+0], mesh->vertices[(i+1)*3+1], mesh->vertices[(i+1)*3+2] };
        Vector3 v3 = { mesh->vertices[(i+2)*3+0], mesh->vertices[(i+2)*3+1], mesh->vertices[(i+2)*3+2] };

        Vector3 edge1 = { v2.x - v1.x, v2.y - v1.y, v2.z - v1.z };
        Vector3 edge2 = { v3.x - v1.x, v3.y - v1.y, v3.z - v1.z };

        Vector3 normal = {
            edge1.y * edge2.z - edge1.z * edge2.y,
            edge1.z * edge2.x - edge1.x * edge2.z,
            edge1.x * edge2.y - edge1.y * edge2.x
        };

        float len = sqrtf(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
        if (len != 0.0f) {
            normal.x /= len;
            normal.y /= len;
            normal.z /= len;
        }

        // Write same normal to all 3 vertices
        for (int j = 0; j < 3; j++) {
            mesh->normals[(i+j)*3+0] = normal.x;
            mesh->normals[(i+j)*3+1] = normal.y;
            mesh->normals[(i+j)*3+2] = normal.z;
        }
    }
}

// Unshare mesh vertices to create flat/faceted shading (hard edges per face)
void MeshUtils::unshareMeshVertices(Mesh *mesh) {
    // Only proceed if the mesh is indexed
    if (mesh->indices == NULL) return;

    int triangleCount = mesh->triangleCount;
    int vertexCount = triangleCount * 3;

    // Allocate memory for new unshared data
    float *newVertices = (float *)MemAlloc(vertexCount * 3 * sizeof(float));
    float *newNormals = (float *)MemAlloc(vertexCount * 3 * sizeof(float));

    for (int i = 0; i < triangleCount * 3; i++) {
        int index = mesh->indices[i];

        // Copy vertex position from the old index to the new linear array
        newVertices[i * 3 + 0] = mesh->vertices[index * 3 + 0];
        newVertices[i * 3 + 1] = mesh->vertices[index * 3 + 1];
        newVertices[i * 3 + 2] = mesh->vertices[index * 3 + 2];
    }

    // Free old data
    MemFree(mesh->vertices);
    if (mesh->normals) MemFree(mesh->normals);
    if (mesh->indices) MemFree(mesh->indices);

    // Assign new data
    mesh->vertices = newVertices;
    mesh->normals = newNormals;
    mesh->vertexCount = vertexCount;
    mesh->indices = NULL; // The mesh is no longer indexed
    
    MeshUtils::computeMeshNormals(mesh);
}

Mesh MeshGenerator::createCustomOctahedron(float radius, int subdivisions)
{
    PolySoup soup;
    Vector3 baseVertices[6] = {
        { 1,  0,  0}, {-1,  0,  0}, { 0,  1,  0}, { 0, -1,  0}, { 0,  0,  1}, { 0,  0, -1}
    };
    soup.verts.assign(baseVertices, baseVertices + 6);
    soup.indices = {
        0, 2, 4,   2, 1, 4,   1, 3, 4,   3, 0, 4,
        0, 3, 5,   3, 1, 5,   1, 2, 5,   2, 0, 5
    };
    PolySoup refined = subdivideSoup(soup, clampSubdiv(subdivisions));
    return bakeSoupToSphere(refined, radius);
}

Mesh MeshGenerator::createCustomTetrahedron(float radius, int subdivisions)
{
    PolySoup soup;
    const float s = 1.0f;
    Vector3 baseVertices[4] = {
        {  s,  s,  s}, { -s, -s,  s}, { -s,  s, -s}, {  s, -s, -s}
    };
    soup.verts.assign(baseVertices, baseVertices + 4);
    soup.indices = { 0,1,2, 0,3,1, 0,2,3, 1,3,2 };
    PolySoup refined = subdivideSoup(soup, clampSubdiv(subdivisions));
    return bakeSoupToSphere(refined, radius);
}

Mesh MeshGenerator::createCustomCubeSphere(float radius, int subdivisions)
{
    PolySoup soup;
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
    PolySoup refined = subdivideSoup(soup, clampSubdiv(subdivisions));
    return bakeSoupToSphere(refined, radius);
}

Mesh MeshGenerator::createCustomDodecahedron(float radius)
{
    PolySoup soup;
    const float phi = (1.0f + sqrtf(5.0f)) * 0.5f;
    const float a = 1.0f;
    const float b = 1.0f / phi;
    const float c = 2.0f - phi;

    soup.verts = {
        {  a,  a,  a}, {  a,  a, -a}, {  a, -a,  a}, {  a, -a, -a},
        { -a,  a,  a}, { -a,  a, -a}, { -a, -a,  a}, { -a, -a, -a},
        {  0,  b,  c}, {  0,  b, -c}, {  0, -b,  c}, {  0, -b, -c},
        {  b,  c,  0}, {  b, -c,  0}, { -b,  c,  0}, { -b, -c,  0},
        {  c,  0,  b}, { -c,  0,  b}, {  c,  0, -b}, { -c,  0, -b}
    };

    const int faces[12][5] = {
        {0,8,4,17,16}, {0,16,2,10,8}, {0,12,1,9,8}, {0,12,18,16,17},
        {1,12,18,3,19}, {1,9,5,15,19}, {1,12,0,8,9}, {2,13,3,18,16},
        {2,10,6,11,13}, {2,16,0,17,10}, {3,19,7,14,13}, {3,18,12,1,19}
    };

    soup.indices.clear();
    soup.indices.reserve(12 * 3 * 3);
    for (int f = 0; f < 12; f++) {
        int v0 = faces[f][0];
        for (int k = 1; k < 4; k++) {
            soup.indices.push_back(v0);
            soup.indices.push_back(faces[f][k]);
            soup.indices.push_back(faces[f][k + 1]);
        }
    }

    return bakeSoupToSphere(soup, radius);
}

Mesh MeshGenerator::createCustomPentagonalPrismSphere(float radius, int rings)
{
    PolySoup soup;
    if (rings < 0) rings = 0; if (rings > 4) rings = 4;

    const int sides = 5;
    const float h = 1.0f;
    const float step = 2.0f * PI / sides;

    // Top/bottom rings interleaved
    for (int i = 0; i < sides; i++) {
        float ang = i * step;
        soup.verts.push_back({ cosf(ang),  h, sinf(ang) });
        soup.verts.push_back({ cosf(ang), -h, sinf(ang) });
    }
    // Mid rings
    for (int r = 1; r <= rings; r++) {
        float t = (float)r / (float)(rings + 1);
        float y = h - 2.0f * h * t;
        for (int i = 0; i < sides; i++) {
            float ang = i * step;
            soup.verts.push_back({ cosf(ang), y, sinf(ang) });
        }
    }

    // Fans
    for (int i = 1; i < sides - 1; i++) {
        soup.indices.insert(soup.indices.end(), { 0, 2*i, 2*(i+1) });
    }
    int bottomStart = 1;
    for (int i = 1; i < sides - 1; i++) {
        soup.indices.insert(soup.indices.end(), { bottomStart, bottomStart + 2*(i+1), bottomStart + 2*i });
    }

    // Side quads split into tris
    int ringCount = rings + 2;
    auto ringIndex = [&](int ring, int side) {
        if (ring == 0) return side * 2;
        if (ring == ringCount - 1) return side * 2 + 1;
        return 2 * sides + (ring - 1) * sides + side;
    };

    for (int r = 0; r < ringCount - 1; r++) {
        for (int s = 0; s < sides; s++) {
            int sNext = (s + 1) % sides;
            int a = ringIndex(r, s);
            int b = ringIndex(r + 1, s);
            int c = ringIndex(r + 1, sNext);
            int d = ringIndex(r, sNext);
            soup.indices.insert(soup.indices.end(), { a, b, c, a, c, d });
        }
    }

    return bakeSoupToSphere(soup, radius);
}

Mesh MeshGenerator::createSquareTree(float radius, int cubeSubdivisions, int octaSubdivisions)
{
    PolySoup base;
    
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
    PolySoup baseSubdiv = subdivideSoup(base, clampSubdiv(cubeSubdivisions), false);
    const float trunkScaleX = radius * 0.37f;
    const float trunkScaleY = radius * 0.85f;
    const float trunkScaleZ = radius * 0.37f;
    for (auto& v : baseSubdiv.verts) {
        v.x *= trunkScaleX;
        v.y *= trunkScaleY;
        v.z *= trunkScaleZ;
    }

    // Build a rounded octahedron canopy perched on top of the cube
    PolySoup canopy;
    Vector3 octVerts[6] = {
        { 1,  0,  0}, {-1,  0,  0}, { 0,  1,  0}, { 0, -1,  0}, { 0,  0,  1}, { 0,  0, -1}
    };
    canopy.verts.assign(octVerts, octVerts + 6);
    canopy.indices = {
        0, 2, 4,   2, 1, 4,   1, 3, 4,   3, 0, 4,
        0, 3, 5,   3, 1, 5,   1, 2, 5,   2, 0, 5
    };

    PolySoup canopySubdiv = subdivideSoup(canopy, clampSubdiv(octaSubdivisions));
    float canopyRadius = radius * 1.05f;
    float canopyOffsetY = trunkScaleY + canopyRadius * 0.6f; // keep canopy dominant and nestled
    for (auto& v : canopySubdiv.verts) {
        float len = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
        if (len > 0.0001f) { v.x /= len; v.y /= len; v.z /= len; }
        v.x *= canopyRadius;
        v.y = v.y * canopyRadius + canopyOffsetY;
        v.z *= canopyRadius;
    }

    PolySoup merged = mergeSoups(baseSubdiv, canopySubdiv);

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

Mesh MeshGenerator::createCubicStar(float radius, int cubeSubdivisions)
{
    PolySoup base;
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
    PolySoup baseSubdiv = subdivideSoup(base, clampSubdiv(cubeSubdivisions), true);
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