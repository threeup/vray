#pragma once

#include "raylib.h"
#include <vector>
#include <map>
#include <cmath>

namespace MeshUtils {

// Poly soup structure for mesh generation
struct PolySoup {
    std::vector<Vector3> verts;
    std::vector<int> indices; // multiples of 3
};

// Simple deterministic pseudo-random helper based on an integer index
inline float pseudoRandom01(int i)
{
    // LCG-style hash, keep it cheap and deterministic
    unsigned int x = static_cast<unsigned int>(i * 747796405u + 2891336453u);
    x ^= x >> 13;
    x *= 1597334677u;
    x ^= x >> 16;
    return (x & 0xFFFFFF) / static_cast<float>(0xFFFFFF); // [0,1]
}

// Clamp subdivision levels to valid range [0,2]
inline int clampSubdiv(int s) {
    return (s < 0) ? 0 : (s > 2) ? 2 : s;
}

// Helper for midpoint calculation during subdivision
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

// Subdivide a polygon soup mesh by midpoint interpolation
PolySoup subdivideSoup(const PolySoup& soup, int levels, bool normalizeMidpoints = true);

// Merge two polygon soups together
inline PolySoup mergeSoups(const PolySoup& a, const PolySoup& b) {
    PolySoup out = a;
    int offset = (int)out.verts.size();
    out.verts.insert(out.verts.end(), b.verts.begin(), b.verts.end());
    out.indices.reserve(out.indices.size() + b.indices.size());
    for (int idx : b.indices) out.indices.push_back(idx + offset);
    return out;
}

// Bake a polygon soup to a spherical mesh with given radius
Mesh bakeSoupToSphere(const PolySoup& soup, float radius);

} // namespace MeshUtils
