
#include "utils/meshMathUtils.h"
#include "utils/meshProcessUtils.h"

namespace MeshUtils {

// Subdivide a polygon soup mesh by midpoint interpolation
PolySoup subdivideSoup(const PolySoup& soup, int levels, bool normalizeMidpoints) {
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


// Bake a polygon soup to a spherical mesh with given radius
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

} // namespace MeshUtils
