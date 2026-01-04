
#include "utils/meshProcessUtils.h"
#include "raylib.h"
#include <cstring>
#include <cmath>

namespace MeshUtils {

void computeMeshNormals(Mesh *mesh)
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

// Unshare mesh vertices to create hard edges per face (flat/faceted shading)
void unshareMeshVertices(Mesh *mesh) {
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
    
    computeMeshNormals(mesh);
}

} // namespace MeshUtils

