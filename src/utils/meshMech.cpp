// Procedural mech generation and merging into a single mesh
#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <cstring>
#include "meshProcessUtils.h"
struct MechPart {
    Mesh mesh;
    Matrix transform;
};

struct ProceduralMech {
    std::vector<MechPart> parts;

    void AddPart(Mesh mesh, Matrix localTransform) {
        parts.push_back({ mesh, localTransform });
    }
};

// Helper: Create a simple cylinder mesh
static Mesh CreateSimpleCylinder(float radius, float height, int slices) {
    std::vector<float> vertices;
    std::vector<unsigned short> indices;
    slices = (slices < 3) ? 3 : slices;
    float halfHeight = height / 2.0f;

    // Bottom ring
    for (int i = 0; i <= slices; i++) {
        float angle = 2.0f * PI * i / slices;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);
        vertices.push_back(x);
        vertices.push_back(-halfHeight);
        vertices.push_back(z);
    }

    // Top ring
    for (int i = 0; i <= slices; i++) {
        float angle = 2.0f * PI * i / slices;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);
        vertices.push_back(x);
        vertices.push_back(halfHeight);
        vertices.push_back(z);
    }

    // Side indices (reversed winding for outward-facing normals)
    for (int i = 0; i < slices; i++) {
        int b1 = i;
        int b2 = i + 1;
        int t1 = slices + 1 + i;
        int t2 = slices + 1 + i + 1;
        indices.push_back(b1);
        indices.push_back(t1);
        indices.push_back(b2);
        indices.push_back(b2);
        indices.push_back(t1);
        indices.push_back(t2);
    }

    // Bottom cap center
    int bottomCenterIdx = vertices.size() / 3;
    vertices.push_back(0.0f);
    vertices.push_back(-halfHeight);
    vertices.push_back(0.0f);

    // Top cap center
    int topCenterIdx = vertices.size() / 3;
    vertices.push_back(0.0f);
    vertices.push_back(halfHeight);
    vertices.push_back(0.0f);

    // Bottom cap triangles
    for (int i = 0; i < slices; i++) {
        indices.push_back(bottomCenterIdx);
        indices.push_back(i + 1);
        indices.push_back(i);
    }

    // Top cap triangles
    for (int i = 0; i < slices; i++) {
        int t1 = slices + 1 + i;
        int t2 = slices + 1 + i + 1;
        indices.push_back(topCenterIdx);
        indices.push_back(t2);
        indices.push_back(t1);
    }

    Mesh mesh = { 0 };
    mesh.vertexCount = (int)vertices.size() / 3;
    mesh.triangleCount = (int)indices.size() / 3;
    mesh.vertices = (float*)RL_MALLOC(vertices.size() * sizeof(float));
    mesh.indices = (unsigned short*)RL_MALLOC(indices.size() * sizeof(unsigned short));
    mesh.texcoords = (float*)RL_MALLOC((size_t)mesh.vertexCount * 2 * sizeof(float));
    
    std::memcpy(mesh.vertices, vertices.data(), vertices.size() * sizeof(float));
    std::memcpy(mesh.indices, indices.data(), indices.size() * sizeof(unsigned short));
    std::fill(mesh.texcoords, mesh.texcoords + mesh.vertexCount * 2, 0.0f);

    return mesh;
}

// Helper: Create a simple sphere mesh
static Mesh CreateSimpleSphere(float radius, int rings, int slices) {
    std::vector<float> vertices;
    std::vector<unsigned short> indices;
    slices = (slices < 3) ? 3 : slices;
    rings = (rings < 2) ? 2 : rings;

    for (int r = 0; r <= rings; r++) {
        float phi = PI * r / rings;
        for (int s = 0; s <= slices; s++) {
            float theta = 2.0f * PI * s / slices;
            float x = radius * sinf(phi) * cosf(theta);
            float y = radius * cosf(phi);
            float z = radius * sinf(phi) * sinf(theta);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    for (int r = 0; r < rings; r++) {
        for (int s = 0; s < slices; s++) {
            int v1 = r * (slices + 1) + s;
            int v2 = v1 + 1;
            int v3 = (r + 1) * (slices + 1) + s;
            int v4 = v3 + 1;
            indices.push_back(v1);
            indices.push_back(v3);
            indices.push_back(v2);
            indices.push_back(v2);
            indices.push_back(v3);
            indices.push_back(v4);
        }
    }

    Mesh mesh = { 0 };
    mesh.vertexCount = (int)vertices.size() / 3;
    mesh.triangleCount = (int)indices.size() / 3;
    mesh.vertices = (float*)RL_MALLOC(vertices.size() * sizeof(float));
    mesh.indices = (unsigned short*)RL_MALLOC(indices.size() * sizeof(unsigned short));
    mesh.texcoords = (float*)RL_MALLOC((size_t)mesh.vertexCount * 2 * sizeof(float));
    
    std::memcpy(mesh.vertices, vertices.data(), vertices.size() * sizeof(float));
    std::memcpy(mesh.indices, indices.data(), indices.size() * sizeof(unsigned short));
    std::fill(mesh.texcoords, mesh.texcoords + mesh.vertexCount * 2, 0.0f);

    return mesh;
}

Mesh CreateMechHead(float width, float height) {
    std::vector<float> vertices;
    std::vector<unsigned short> indices;
    int slices = 6; 
    int rings = 3;

    // 1. Generate Vertices
    auto getHeadPos = [&](int ring, int slice) {
        float hPerc = (float)ring / 2.0f;
        // Ring 1 (middle) is the widest for that "helmet" look
        float radius = (ring == 1) ? width : width * 0.7f;
        float angle = slice * (2.0f * PI / slices);
        
        // Flatten/Push the "face" forward (assuming Z+ is forward)
        // cosf(angle) > 0.5 picks the front-most slices of the hexagon
        float zOffset = (cosf(angle) > 0.5f) ? (width * 0.3f) : 0.0f; 

        return Vector3{ 
            cosf(angle) * radius, 
            hPerc * height, 
            sinf(angle) * radius + zOffset 
        };
    };

    for (int r = 0; r < rings; r++) {
        for (int s = 0; s < slices; s++) {
            Vector3 p = getHeadPos(r, s);
            vertices.push_back(p.x); vertices.push_back(p.y); vertices.push_back(p.z);
        }
    }

    // Add center points for caps
    int bottomCenterIdx = (int)vertices.size() / 3;
    vertices.push_back(0); vertices.push_back(0); vertices.push_back(0);
    
    int topCenterIdx = (int)vertices.size() / 3;
    vertices.push_back(0); vertices.push_back(height); vertices.push_back(0.1f); // Slightly forward

    // 2. Generate Indices (Side Shell)
    for (int r = 0; r < rings - 1; r++) {
        for (int s = 0; s < slices; s++) {
            int next = (s + 1) % slices;
            int currentRing = r * slices;
            int nextRing = (r + 1) * slices;

            indices.push_back(currentRing + s);
            indices.push_back(nextRing + s);
            indices.push_back(nextRing + next);

            indices.push_back(currentRing + s);
            indices.push_back(nextRing + next);
            indices.push_back(currentRing + next);
        }
    }

    // 3. Bottom Cap (Neck area)
    for (int s = 0; s < slices; s++) {
        indices.push_back(bottomCenterIdx);
        indices.push_back((s + 1) % slices);
        indices.push_back(s);
    }

    // 4. Top Cap (Cranium)
    int topRingStart = (rings - 1) * slices;
    for (int s = 0; s < slices; s++) {
        int next = (s + 1) % slices;
        // Flip winding so normals point outward/up (prevents culling hole)
        indices.push_back(topCenterIdx);
        indices.push_back(topRingStart + next);
        indices.push_back(topRingStart + s);
    }

    // Finalize Mesh
    Mesh mesh = { 0 };
    mesh.vertexCount = (int)vertices.size() / 3;
    mesh.triangleCount = (int)indices.size() / 3;
    mesh.vertices = (float*)RL_MALLOC(vertices.size() * sizeof(float));
    mesh.indices = (unsigned short*)RL_MALLOC(indices.size() * sizeof(unsigned short));

    memcpy(mesh.vertices, vertices.data(), vertices.size() * sizeof(float));
    memcpy(mesh.indices, indices.data(), indices.size() * sizeof(unsigned short));

    // Upload and calculate normals for lighting
    MeshUtils::computeMeshNormals(&mesh);
    UploadMesh(&mesh, false);

    return mesh;
}
Mesh CreateRocketPod(float width, float height, float depth) {
    std::vector<float> vertices;
    std::vector<unsigned short> indices;

    float inset = 0.1f * width; // How thick the armor casing is
    float rackDepth = 0.15f * depth; // How deep the missiles are recessed

    // --- 1. DEFINE VERTICES ---
    // Back Face (0-3)
    float b = -depth/2;
    vertices.insert(vertices.end(), {-width/2, -height/2, b,  width/2, -height/2, b,  width/2, height/2, b,  -width/2, height/2, b});

    // Front Rim Face (4-7)
    float f = depth/2;
    vertices.insert(vertices.end(), {-width/2, -height/2, f,  width/2, -height/2, f,  width/2, height/2, f,  -width/2, height/2, f});

    // Inset Launcher Panel (8-11)
    float i = f - rackDepth;
    vertices.insert(vertices.end(), {
        -width/2 + inset, -height/2 + inset, i,
         width/2 - inset, -height/2 + inset, i,
         width/2 - inset,  height/2 - inset, i,
        -width/2 + inset,  height/2 - inset, i
    });

    // --- 2. DEFINE INDICES (Triangles) ---
    auto addQuad = [&](int a, int b, int c, int d) {
        indices.push_back(a); indices.push_back(b); indices.push_back(c);
        indices.push_back(a); indices.push_back(c); indices.push_back(d);
    };

    // External Casing (Back, Left, Right, Top, Bottom)
    addQuad(0, 3, 2, 1); // Back
    addQuad(0, 4, 7, 3); // Left
    addQuad(1, 2, 6, 5); // Right
    addQuad(3, 7, 6, 2); // Top
    addQuad(0, 1, 5, 4); // Bottom

    // Front Bezel (The "Rim" faces)
    addQuad(4, 5, 9, 8);   // Bottom rim
    addQuad(7, 11, 10, 6); // Top rim
    addQuad(4, 8, 11, 7);  // Left rim
    addQuad(5, 6, 10, 9);  // Right rim

    // The actual Launcher Plate (The recessed surface)
    addQuad(8, 9, 10, 11);

    // --- 3. FINALIZE MESH ---
    Mesh mesh = { 0 };
    mesh.vertexCount = (int)vertices.size() / 3;
    mesh.triangleCount = (int)indices.size() / 3;
    mesh.vertices = (float*)RL_MALLOC(vertices.size() * sizeof(float));
    mesh.indices = (unsigned short*)RL_MALLOC(indices.size() * sizeof(unsigned short));

    memcpy(mesh.vertices, vertices.data(), vertices.size() * sizeof(float));
    memcpy(mesh.indices, indices.data(), indices.size() * sizeof(unsigned short));

    // Allocate UVs so we can map a "Circle/Missile" texture to the inset panel
    mesh.texcoords = (float*)RL_MALLOC(mesh.vertexCount * 2 * sizeof(float));
    for (int n = 0; n < mesh.vertexCount; n++) {
        // Simple planar mapping for the front face (vertices 8-11)
        if (n >= 8 && n <= 11) {
            if (n == 8) { mesh.texcoords[n*2] = 0; mesh.texcoords[n*2+1] = 0; }
            if (n == 9) { mesh.texcoords[n*2] = 1; mesh.texcoords[n*2+1] = 0; }
            if (n == 10) { mesh.texcoords[n*2] = 1; mesh.texcoords[n*2+1] = 1; }
            if (n == 11) { mesh.texcoords[n*2] = 0; mesh.texcoords[n*2+1] = 1; }
        } else {
            mesh.texcoords[n*2] = 0; mesh.texcoords[n*2+1] = 0; 
        }
    }

    MeshUtils::computeMeshNormals(&mesh);
    UploadMesh(&mesh, false);
    return mesh;
}
Mesh CreateMechFoot(float width, float length) {
    std::vector<float> vertices;
    std::vector<unsigned short> indices;
    float h = 0.3f; // Height of the foot

    // 1. Define the 8 vertices as requested
    Vector3 pts[8] = {
        {-width, 0, -length * 0.5f},      // 0: Back Bottom Left
        {width, 0, -length * 0.5f},       // 1: Back Bottom Right
        {width, 0,  length * 0.8f},       // 2: Front Bottom Right (Extended)
        {-width, 0,  length * 0.8f},      // 3: Front Bottom Left (Extended)
        {-width * 0.6f, h, -length * 0.4f}, // 4: Back Top Left
        {width * 0.6f, h, -length * 0.4f},  // 5: Back Top Right
        {width * 0.6f, h,  length * 0.2f},  // 6: Front Top Right (Recessed)
        {-width * 0.6f, h,  length * 0.2f}  // 7: Front Top Left (Recessed)
    };

    for (int i = 0; i < 8; i++) {
        vertices.push_back(pts[i].x);
        vertices.push_back(pts[i].y);
        vertices.push_back(pts[i].z);
    }

    // 2. Define the Triangles (Clockwise or Counter-Clockwise depending on culling)
    auto addQuad = [&](int a, int b, int c, int d) {
        // Triangle 1
        indices.push_back(a); indices.push_back(b); indices.push_back(c);
        // Triangle 2
        indices.push_back(a); indices.push_back(c); indices.push_back(d);
    };

    // Front face (The "Toe" slant)
    addQuad(3, 2, 6, 7);
    // Back face (The Heel)
    addQuad(1, 0, 4, 5);
    // Top face (Ankle attachment)
    addQuad(7, 6, 5, 4);
    // Bottom face (Sole)
    addQuad(0, 1, 2, 3);
    // Left face
    addQuad(0, 3, 7, 4);
    // Right face
    addQuad(2, 1, 5, 6);

    // 3. Create the Raylib Mesh
    Mesh mesh = { 0 };
    mesh.vertexCount = (int)vertices.size() / 3;
    mesh.triangleCount = (int)indices.size() / 3;
    mesh.vertices = (float*)RL_MALLOC(vertices.size() * sizeof(float));
    mesh.indices = (unsigned short*)RL_MALLOC(indices.size() * sizeof(unsigned short));

    memcpy(mesh.vertices, vertices.data(), vertices.size() * sizeof(float));
    memcpy(mesh.indices, indices.data(), indices.size() * sizeof(unsigned short));

    // Allocate basic texcoords (planar mapping for now)
    mesh.texcoords = (float*)RL_MALLOC(mesh.vertexCount * 2 * sizeof(float));
    for (int i = 0; i < mesh.vertexCount; i++) {
        mesh.texcoords[i * 2] = vertices[i * 3] / width;
        mesh.texcoords[i * 2 + 1] = vertices[i * 3 + 2] / length;
    }

    // Generate normals so the foot looks 3D with lighting
    MeshUtils::computeMeshNormals(&mesh);
    UploadMesh(&mesh, false);

    return mesh;
}
ProceduralMech AssembleMech() {
    ProceduralMech mech;
    
    // Lankier proportions with thinner limbs, then scale to 40% overall size
    const float scale = 0.4f;
    
    // Bottom-up stack with proper vertical alignment
    // Feet: smaller, lankier proportions
    mech.AddPart(CreateMechFoot(0.35f * scale, 0.7f * scale), MatrixTranslate(-0.4f * scale, 0.1f * scale, 0.08f * scale));
    mech.AddPart(CreateMechFoot(0.35f * scale, 0.7f * scale), MatrixTranslate(0.4f * scale, 0.1f * scale, 0.08f * scale));
    
    // Shins: thinner, taller
    mech.AddPart(CreateSimpleCylinder(0.18f * scale, 1.2f * scale, 6), MatrixTranslate(-0.35f * scale, 0.65f * scale, 0));
    mech.AddPart(CreateSimpleCylinder(0.18f * scale, 1.2f * scale, 6), MatrixTranslate(0.35f * scale, 0.65f * scale, 0));
    
    // Thighs: thinner, taller
    mech.AddPart(CreateSimpleCylinder(0.22f * scale, 1.4f * scale, 6), MatrixTranslate(-0.35f * scale, 1.95f * scale, 0));
    mech.AddPart(CreateSimpleCylinder(0.22f * scale, 1.4f * scale, 6), MatrixTranslate(0.35f * scale, 1.95f * scale, 0));
    
    // Pelvis: compact, narrow
    mech.AddPart(GenMeshCube(0.7f * scale, 0.4f * scale, 0.7f * scale), MatrixTranslate(0, 3.0f * scale, 0));
    
    // Torso: much thinner, taller for lankiness
    mech.AddPart(CreateSimpleCylinder(0.55f * scale, 2.0f * scale, 6), MatrixTranslate(0, 4.0f * scale, 0));
    
    // Shoulders: smaller, positioned tighter to torso
    mech.AddPart(CreateSimpleSphere(0.25f * scale, 8, 8), MatrixTranslate(-0.65f * scale, 4.8f * scale, 0));
    mech.AddPart(CreateSimpleSphere(0.25f * scale, 8, 8), MatrixTranslate(0.65f * scale, 4.8f * scale, 0));
    
    // Weapons on shoulders: thinner, smaller
    // Left weapon (cannon, rotated)
    mech.AddPart(CreateSimpleCylinder(0.14f * scale, 1.0f * scale, 8),
        MatrixMultiply(MatrixRotateX(75 * DEG2RAD), MatrixTranslate(-0.65f * scale, 4.6f * scale, 0.6f * scale)));
    
    // Right weapon (pod): smaller
    mech.AddPart(GenMeshCube(0.4f * scale, 0.4f * scale, 0.7f * scale), MatrixTranslate(0.65f * scale, 4.5f * scale, 0.45f * scale));
    
    // Neck: thinner bridge
    mech.AddPart(CreateSimpleCylinder(0.18f * scale, 0.35f * scale, 8), MatrixTranslate(0, 5.2f * scale, 0));
    
    // Head: smaller proportions (positioned so bottom touches neck top)
    mech.AddPart(CreateMechHead(0.38f * scale, 0.45f * scale), MatrixTranslate(0, 5.375f * scale, 0.08f * scale));

    return mech;
}

// Merge all mech parts into a single mesh with transforms applied and fresh normals.
static Mesh MergeMechParts(const ProceduralMech& mech) {
    std::vector<float> verts;
    std::vector<unsigned short> indices;

    float minY = FLT_MAX;

    for (size_t p = 0; p < mech.parts.size(); ++p) {
        const auto& part = mech.parts[p];
        if (part.mesh.vertexCount <= 0 || part.mesh.vertices == nullptr || part.mesh.indices == nullptr) {
            continue;
        }
        
        unsigned short base = static_cast<unsigned short>(verts.size() / 3);

        // Transform and append vertices
        for (int i = 0; i < part.mesh.vertexCount; ++i) {
            Vector3 p_in{ part.mesh.vertices[i * 3 + 0], part.mesh.vertices[i * 3 + 1], part.mesh.vertices[i * 3 + 2] };
            Vector3 p_out = Vector3Transform(p_in, part.transform);
            minY = fminf(minY, p_out.y);
            verts.push_back(p_out.x);
            verts.push_back(p_out.y);
            verts.push_back(p_out.z);
        }

        // Append indices with offset
        for (int i = 0; i < part.mesh.triangleCount * 3; ++i) {
            indices.push_back(base + part.mesh.indices[i]);
        }

        // Free the part mesh now that we've copied data
        UnloadMesh(part.mesh);
    }

    // Rebase so feet sit at y=0
    if (minY == FLT_MAX) minY = 0.0f;
    for (size_t i = 0; i < verts.size(); i += 3) {
        verts[i + 1] -= minY;
    }

    Mesh mesh = { 0 };
    mesh.vertexCount = static_cast<int>(verts.size() / 3);
    mesh.triangleCount = static_cast<int>(indices.size() / 3);
    mesh.vertices = (float*)RL_MALLOC(verts.size() * sizeof(float));
    mesh.indices = (unsigned short*)RL_MALLOC(indices.size() * sizeof(unsigned short));

    std::memcpy(mesh.vertices, verts.data(), verts.size() * sizeof(float));
    std::memcpy(mesh.indices, indices.data(), indices.size() * sizeof(unsigned short));

    // Allocate dummy texcoords
    mesh.texcoords = (float*)RL_MALLOC((size_t)mesh.vertexCount * 2 * sizeof(float));
    std::fill(mesh.texcoords, mesh.texcoords + mesh.vertexCount * 2, 0.0f);

    MeshUtils::computeMeshNormals(&mesh);
    MeshUtils::checkIsValid(mesh);
    UploadMesh(&mesh, false);
    return mesh;
}

Mesh CreateMechMesh() {
    ProceduralMech mech = AssembleMech();
    return MergeMechParts(mech);
}