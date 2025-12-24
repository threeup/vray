#include "raylib.h"
#include "mesh.h"
#include <cstdio>

struct MeshSummary {
    int vertexCount = 0;
    int triangleCount = 0;
    bool hasVertices = false;
    bool hasNormals = false;
    bool hasIndices = false;
};

static MeshSummary SummarizeMesh(const Mesh& mesh) {
    MeshSummary s;
    s.vertexCount = mesh.vertexCount;
    s.triangleCount = mesh.triangleCount;
    s.hasVertices = mesh.vertices != nullptr;
    s.hasNormals = mesh.normals != nullptr;
    s.hasIndices = mesh.indices != nullptr;
    return s;
}

static bool IsMeshReady(const MeshSummary& s) {
    return s.vertexCount > 0 && s.triangleCount > 0 && s.hasVertices && s.hasNormals;
}

static void PrintSummary(const char* label, const MeshSummary& s) {
    std::printf("%s: verts=%d tris=%d vertices=%s normals=%s indices=%s\n",
                label,
                s.vertexCount,
                s.triangleCount,
                s.hasVertices ? "yes" : "no",
                s.hasNormals ? "yes" : "no",
                s.hasIndices ? "yes" : "no");
}

static void PrintMeshBounds(const char* label, const Mesh& mesh) {
    if (!mesh.vertices || mesh.vertexCount == 0) {
        std::printf("%s: no vertices\n", label);
        return;
    }
    std::printf("%s: first 3 verts: (%.2f,%.2f,%.2f) (%.2f,%.2f,%.2f) (%.2f,%.2f,%.2f)\n",
                label,
                mesh.vertices[0], mesh.vertices[1], mesh.vertices[2],
                mesh.vertices[3], mesh.vertices[4], mesh.vertices[5],
                mesh.vertices[6], mesh.vertices[7], mesh.vertices[8]);
    float minX = mesh.vertices[0], maxX = mesh.vertices[0];
    float minY = mesh.vertices[1], maxY = mesh.vertices[1];
    float minZ = mesh.vertices[2], maxZ = mesh.vertices[2];
    
    for (int i = 0; i < mesh.vertexCount; i++) {
        minX = (mesh.vertices[i*3+0] < minX) ? mesh.vertices[i*3+0] : minX;
        maxX = (mesh.vertices[i*3+0] > maxX) ? mesh.vertices[i*3+0] : maxX;
        minY = (mesh.vertices[i*3+1] < minY) ? mesh.vertices[i*3+1] : minY;
        maxY = (mesh.vertices[i*3+1] > maxY) ? mesh.vertices[i*3+1] : maxY;
        minZ = (mesh.vertices[i*3+2] < minZ) ? mesh.vertices[i*3+2] : minZ;
        maxZ = (mesh.vertices[i*3+2] > maxZ) ? mesh.vertices[i*3+2] : maxZ;
    }
    std::printf("%s bounds: X[%.2f,%.2f] Y[%.2f,%.2f] Z[%.2f,%.2f]\n",
                label, minX, maxX, minY, maxY, minZ, maxZ);
}

static void CompareSummaries(const MeshSummary& a, const MeshSummary& b) {
    std::printf("Field comparison (lowPoly vs icosphere)\n");
    std::printf(" vertexCount : %d vs %d %s\n", a.vertexCount, b.vertexCount,
                (a.vertexCount > 0 && b.vertexCount > 0) ? "ok" : "mismatch");
    std::printf(" triangleCount: %d vs %d %s\n", a.triangleCount, b.triangleCount,
                (a.triangleCount > 0 && b.triangleCount > 0) ? "ok" : "mismatch");
    std::printf(" vertices ptr : %s vs %s %s\n", a.hasVertices ? "set" : "null",
                b.hasVertices ? "set" : "null",
                (a.hasVertices && b.hasVertices) ? "ok" : "mismatch");
    std::printf(" normals ptr  : %s vs %s %s\n", a.hasNormals ? "set" : "null",
                b.hasNormals ? "set" : "null",
                (a.hasNormals && b.hasNormals) ? "ok" : "mismatch");
    std::printf(" indices ptr  : %s vs %s (note: icosphere intentionally unindexed)\n",
                a.hasIndices ? "set" : "null",
                b.hasIndices ? "set" : "null");
}

int main() {
    // Initialize raylib window and graphics context before mesh operations
    InitWindow(800, 600, "mesh_tests");
    
    // Test 1: create low-poly sphere mesh
    Mesh lowPoly = MeshGenerator::createLowPolySphereMesh(1.5f, 10, 10, 0.2f);
    MeshSummary lowSummary = SummarizeMesh(lowPoly);
    bool lowOk = IsMeshReady(lowSummary);

    // // Test 2: create custom icosphere mesh
    Mesh ico = MeshGenerator::createCustomIcosphere(1.5f, 0);
    MeshSummary icoSummary = SummarizeMesh(ico);
    bool icoOk = IsMeshReady(icoSummary);

    // Test 3: create square tree (merged cube + octahedron)
    Mesh tree = MeshGenerator::createSquareTree(1.0f, 1, 1);
    MeshSummary treeSummary = SummarizeMesh(tree);
    bool treeOk = IsMeshReady(treeSummary);

    PrintSummary("lowPoly", lowSummary);
    PrintSummary("icosphere", icoSummary);
    PrintSummary("squareTree", treeSummary);
    PrintMeshBounds("tree", tree);
    CompareSummaries(lowSummary, icoSummary);

    bool allOk = lowOk && icoOk && treeOk;
    std::printf("Result: %s\n", allOk ? "PASS" : "FAIL");

    // Clean up allocated mesh buffers
    UnloadMesh(lowPoly);
    UnloadMesh(ico);
    UnloadMesh(tree);
    
    // Cleanup raylib
    CloseWindow();

    return allOk ? 0 : 1;
}
