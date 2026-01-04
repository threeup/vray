#include <string>
#include "utils/meshProcessUtils.h"
#include "raylib.h"
#include <cstring>
#include <cmath>
#include <stdexcept>

namespace MeshUtils {
void computeMeshNormals(Mesh *mesh)
{
    // Safety checks
    if (mesh == NULL || mesh->vertices == NULL || mesh->triangleCount <= 0)
        return;

    // Allocate normals if needed (3 floats per vertex)
    if (mesh->normals == NULL)
        mesh->normals = (float*)MemAlloc(mesh->vertexCount * 3 * sizeof(float));

    // Clear normals
    memset(mesh->normals, 0, mesh->vertexCount * 3 * sizeof(float));

    for (int tri = 0; tri < mesh->triangleCount; tri++)
    {
        // Get the three vertex indices for this triangle
        int idx0, idx1, idx2;
        if (mesh->indices != NULL) {
            idx0 = mesh->indices[tri * 3 + 0];
            idx1 = mesh->indices[tri * 3 + 1];
            idx2 = mesh->indices[tri * 3 + 2];
        } else {
            idx0 = tri * 3 + 0;
            idx1 = tri * 3 + 1;
            idx2 = tri * 3 + 2;
        }

        Vector3 v1 = { mesh->vertices[idx0*3+0], mesh->vertices[idx0*3+1], mesh->vertices[idx0*3+2] };
        Vector3 v2 = { mesh->vertices[idx1*3+0], mesh->vertices[idx1*3+1], mesh->vertices[idx1*3+2] };
        Vector3 v3 = { mesh->vertices[idx2*3+0], mesh->vertices[idx2*3+1], mesh->vertices[idx2*3+2] };

        Vector3 edge1 = { v2.x - v1.x, v2.y - v1.y, v2.z - v1.z };
        Vector3 edge2 = { v3.x - v1.x, v3.y - v1.y, v3.z - v1.z };

        // Cross product
        Vector3 normal = {
            edge1.y * edge2.z - edge1.z * edge2.y,
            edge1.z * edge2.x - edge1.x * edge2.z,
            edge1.x * edge2.y - edge1.y * edge2.x
        };

        float len = sqrtf(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
        
        // FIX: Handle degenerate triangles (magnitude too small)
        if (len > 0.0001f) {
            normal.x /= len;
            normal.y /= len;
            normal.z /= len;
        } else {
            // Fallback to a default vector to satisfy validation and avoid black artifacts
            normal.x = 0.0f;
            normal.y = 1.0f;
            normal.z = 0.0f;
        }

        // Apply normal to vertices
        // Note: For flat shading (like your current code), we just overwrite.
        // For smooth shading, you would add these to a running sum and normalize at the end.
        int indices[3] = { idx0, idx1, idx2 };
        for (int j = 0; j < 3; j++) {
            mesh->normals[indices[j]*3+0] = normal.x;
            mesh->normals[indices[j]*3+1] = normal.y;
            mesh->normals[indices[j]*3+2] = normal.z;
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

bool checkIsValid(const Mesh& mesh) {
    // Check vertex count
    if (mesh.vertexCount <= 0) {
        throw std::runtime_error("Mesh validation failed: vertexCount <= 0");
    }
    
    // Check triangle count
    if (mesh.triangleCount <= 0) {
        throw std::runtime_error("Mesh validation failed: triangleCount <= 0");
    }
    
    // Check vertices pointer
    if (mesh.vertices == NULL) {
        throw std::runtime_error("Mesh validation failed: vertices pointer is NULL");
    }
    
    // Check normals pointer
    if (mesh.normals == NULL) {
        throw std::runtime_error("Mesh validation failed: normals pointer is NULL");
    }
    
    // Check texcoords pointer
    if (mesh.texcoords == NULL) {
        throw std::runtime_error("Mesh validation failed: texcoords pointer is NULL");
    }

    // 1. Check Index Bounds (The most common crash cause)
    if (mesh.indices != nullptr) {
        for (int i = 0; i < mesh.triangleCount * 3; i++) {
            if (mesh.indices[i] >= mesh.vertexCount) {
                // This means your index points to a vertex that doesn't exist
                throw std::runtime_error("Index out of bounds at index " + std::to_string(i));
            }
        }
    }

    // 2. Check for NaN/Inf (Causes mesh to be invisible)
    for (int i = 0; i < mesh.vertexCount * 3; i++) {
        if (std::isnan(mesh.vertices[i]) || std::isinf(mesh.vertices[i])) {
            throw std::runtime_error("Mesh contains NaN or Inf vertex data");
        }
    }

    // 3. Check Normal normalization
    if (mesh.normals == nullptr) throw std::runtime_error("Mesh normals are missing");
    // Validate vertex data for NaN or inf
    for (int i = 0; i < mesh.vertexCount; ++i) {
        float x = mesh.vertices[i * 3 + 0];
        float y = mesh.vertices[i * 3 + 1];
        float z = mesh.vertices[i * 3 + 2];
        if (std::isnan(x) || std::isnan(y) || std::isnan(z)) {
            throw std::runtime_error("Mesh validation failed: NaN in vertex data");
        }
        if (std::isinf(x) || std::isinf(y) || std::isinf(z)) {
            throw std::runtime_error("Mesh validation failed: Infinity in vertex data");
        }
    }
    
    // Validate normal data for NaN or inf
    for (int i = 0; i < mesh.vertexCount; ++i) {
        float nx = mesh.normals[i * 3 + 0];
        float ny = mesh.normals[i * 3 + 1];
        float nz = mesh.normals[i * 3 + 2];
        if (std::isnan(nx) || std::isnan(ny) || std::isnan(nz)) {
            throw std::runtime_error("Mesh validation failed: NaN in normal data");
        }
        if (std::isinf(nx) || std::isinf(ny) || std::isinf(nz)) {
            throw std::runtime_error("Mesh validation failed: Infinity in normal data");
        }
    }
    
    // Check that normals are reasonably normalized (magnitude should be close to 1)
    for (int i = 0; i < mesh.vertexCount; ++i) {
        float nx = mesh.normals[i * 3 + 0];
        float ny = mesh.normals[i * 3 + 1];
        float nz = mesh.normals[i * 3 + 2];
        float len = std::sqrt(nx*nx + ny*ny + nz*nz);
        if (len < 0.1f) {
            throw std::runtime_error("Mesh validation failed: degenerate normal (magnitude too small)");
        }
    }
    
    // Note: indices pointer can be NULL for non-indexed meshes, which is valid
    // (e.g., after unshareMeshVertices converts to non-indexed rendering)
    
    return true;
}

} // namespace MeshUtils

