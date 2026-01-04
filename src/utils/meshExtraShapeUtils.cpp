#include "utils/meshGenerateUtils.h"
#include "utils/meshMathUtils.h"
#include "utils/meshProcessUtils.h"
#include <cmath>
#include <vector>
#include <map>

namespace MeshGenerator {

Mesh createSpikyBlobMesh(float radius, int rings, int slices, float spikeAmount) {
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
            float r = MeshUtils::pseudoRandom01(i * 17 + 3); // scramble a bit differently
            float offset = (r - 0.5f) * spikeAmount; // [-spike/2, spike/2]
            float scale = (len + offset) / len;
            verts[idx + 0] = x * scale;
            verts[idx + 1] = y * scale;
            verts[idx + 2] = z * scale;
        }
    }
    return mesh;
}

Mesh createTwistedColumnMesh(float radius, float height, int slices, int twistTurns) {
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


Mesh createCustomIcosphere(float radius, int subdivisions)
{
    MeshUtils::PolySoup soup;
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

    int levels = MeshUtils::clampSubdiv(subdivisions);
    MeshUtils::PolySoup refined = MeshUtils::subdivideSoup(soup, levels, true);
    return MeshUtils::bakeSoupToSphere(refined, radius);
}

Mesh createTorusMesh(float radius, float size, int radSeg, int sides) {
    Mesh mesh = { 0 };
    
    // Safety checks
    if (radius <= 0.0f) radius = 0.1f;
    if (size <= 0.0f) size = 0.1f;
    if (radSeg < 3) radSeg = 3;
    if (sides < 3) sides = 3;

    int numVertices = (radSeg + 1) * (sides + 1);
    int numTriangles = radSeg * sides * 2;
    
    mesh.vertexCount = numVertices;
    mesh.triangleCount = numTriangles;
    
    mesh.vertices = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.normals = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float*)MemAlloc(mesh.vertexCount * 2 * sizeof(float));
    mesh.indices = (unsigned short*)MemAlloc(mesh.triangleCount * 3 * sizeof(unsigned short));

    // Generate vertices
    int vIndex = 0;
    for (int i = 0; i <= radSeg; i++) {
        float theta = (float)i * 2.0f * PI / radSeg;
        float cosTheta = cosf(theta);
        float sinTheta = sinf(theta);

        for (int j = 0; j <= sides; j++) {
            float phi = (float)j * 2.0f * PI / sides;
            float cosPhi = cosf(phi);
            float sinPhi = sinf(phi);

            // Torus math
            float x = (radius + size * cosPhi) * cosTheta;
            float y = size * sinPhi;
            float z = (radius + size * cosPhi) * sinTheta;

            // Normal (points outward from the tube center)
            float nx = cosPhi * cosTheta;
            float ny = sinPhi;
            float nz = cosPhi * sinTheta;

            mesh.vertices[vIndex * 3 + 0] = x;
            mesh.vertices[vIndex * 3 + 1] = y;
            mesh.vertices[vIndex * 3 + 2] = z;

            mesh.normals[vIndex * 3 + 0] = nx;
            mesh.normals[vIndex * 3 + 1] = ny;
            mesh.normals[vIndex * 3 + 2] = nz;

            mesh.texcoords[vIndex * 2 + 0] = (float)i / radSeg;
            mesh.texcoords[vIndex * 2 + 1] = (float)j / sides;
            
            vIndex++;
        }
    }

    // Generate indices
    int iIndex = 0;
    for (int i = 0; i < radSeg; i++) {
        for (int j = 0; j < sides; j++) {
            int nextI = i + 1;
            int nextJ = j + 1;

            int a = i * (sides + 1) + j;
            int b = nextI * (sides + 1) + j;
            int c = nextI * (sides + 1) + nextJ;
            int d = i * (sides + 1) + nextJ;

            // Triangle 1
            mesh.indices[iIndex++] = (unsigned short)a;
            mesh.indices[iIndex++] = (unsigned short)d;
            mesh.indices[iIndex++] = (unsigned short)b;

            // Triangle 2
            mesh.indices[iIndex++] = (unsigned short)b;
            mesh.indices[iIndex++] = (unsigned short)d;
            mesh.indices[iIndex++] = (unsigned short)c;
        }
    }

    mesh.vboId = NULL;
    return mesh;
}

// ----------------------------------------------------------------------------
// 2. CAPSULE (Pill) - Essential for colliders and sci-fi props
//    Constructed by generating a cylinder and two hemispherical caps
// ----------------------------------------------------------------------------
Mesh createCapsuleMesh(float radius, float height, int rings, int slices) {
    // Height is the cylinder part only. Total height = height + 2*radius
    Mesh mesh = { 0 };
    
    if (rings < 1) rings = 1;
    if (slices < 3) slices = 3;

    // We simulate a sphere but split the rings in the middle to insert the cylinder body
    int capRings = rings; 
    int totalRings = capRings * 2 + 1; // +1 for the cylinder segment
    int numVertices = (totalRings + 1) * (slices + 1);
    int numTriangles = totalRings * slices * 2;

    mesh.vertexCount = numVertices;
    mesh.triangleCount = numTriangles;

    mesh.vertices = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.normals = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float*)MemAlloc(mesh.vertexCount * 2 * sizeof(float));
    mesh.indices = (unsigned short*)MemAlloc(mesh.triangleCount * 3 * sizeof(unsigned short));

    float halfHeight = height * 0.5f;
    int vIndex = 0;

    for (int r = 0; r <= totalRings; r++) {
        float v = (float)r / totalRings;
        
        // Determine vertical position (y) and ring radius factor (sinPhi)
        float y, ringRad;
        
        if (r <= capRings) {
            // Top Hemisphere
            float phi = (float)r / (float)capRings * (PI / 2.0f);
            y = halfHeight + radius * cosf(phi);
            ringRad = radius * sinf(phi);
        } else {
            // Bottom Hemisphere
            float phi = (float)(r - capRings) / (float)capRings * (PI / 2.0f) + (PI / 2.0f);
            y = -halfHeight + radius * cosf(phi);
            ringRad = radius * sinf(phi);
        }

        // Fix the "cylinder" seam:
        // If we are at the exact transition rings, ensure strictly equal radius
        if (r == capRings) { y = halfHeight; ringRad = radius; }
        if (r == capRings + 1) { y = -halfHeight; ringRad = radius; }

        for (int s = 0; s <= slices; s++) {
            float u = (float)s / slices;
            float theta = u * 2.0f * PI;

            float x = ringRad * cosf(theta);
            float z = ringRad * sinf(theta);

            mesh.vertices[vIndex * 3 + 0] = x;
            mesh.vertices[vIndex * 3 + 1] = y;
            mesh.vertices[vIndex * 3 + 2] = z;

            // Normal calculation
            Vector3 n = { x, 0, z };
            if (y > halfHeight) n.y = y - halfHeight;
            else if (y < -halfHeight) n.y = y + halfHeight;
            else n.y = 0; // Cylinder body
            
            float len = sqrtf(n.x*n.x + n.y*n.y + n.z*n.z);
            if (len > 0) { n.x/=len; n.y/=len; n.z/=len; }

            mesh.normals[vIndex * 3 + 0] = n.x;
            mesh.normals[vIndex * 3 + 1] = n.y;
            mesh.normals[vIndex * 3 + 2] = n.z;
            
            mesh.texcoords[vIndex * 2 + 0] = u;
            mesh.texcoords[vIndex * 2 + 1] = v;

            vIndex++;
        }
    }

    // Indices (standard grid connectivity)
    int iIndex = 0;
    for (int r = 0; r < totalRings; r++) {
        for (int s = 0; s < slices; s++) {
            int nextR = r + 1;
            int nextS = s + 1;
            
            int a = r * (slices + 1) + s;
            int b = nextR * (slices + 1) + s;
            int c = nextR * (slices + 1) + nextS;
            int d = r * (slices + 1) + nextS;

            mesh.indices[iIndex++] = a;
            mesh.indices[iIndex++] = d;
            mesh.indices[iIndex++] = b;

            mesh.indices[iIndex++] = b;
            mesh.indices[iIndex++] = d;
            mesh.indices[iIndex++] = c;
        }
    }
    
    mesh.vboId = NULL;
    return mesh;
}

Mesh createCustomPentagonalPrismSphere(float radius, int rings)
{
    MeshUtils::PolySoup soup;
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

    return  MeshUtils::bakeSoupToSphere(soup, radius);
}



Mesh createCustomDodecahedron(float radius)
{
    MeshUtils::PolySoup soup;
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

    return MeshUtils::bakeSoupToSphere(soup, radius);
}

} // namespace MeshGenerator