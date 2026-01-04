// Procedural mech generation and merging into a single mesh
#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <cstring>
#include "meshProcessUtils.h"

static float SmoothStep(float edge0, float edge1, float x) {
    x = (x - edge0) / (edge1 - edge0);
    x = fmaxf(0.0f, fminf(1.0f, x));
    return x * x * (3.0f - 2.0f * x);
}
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
        float frontBias = SmoothStep(0.0f, 0.7f, cosf(angle)); // soften transition on low-poly hex
        float zOffset = width * 0.25f * frontBias; 

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

    return mesh;
}

static Mesh CreateArmoredLegPart(float bottomRad, float topRad, float height) {
    std::vector<float> vertices;
    std::vector<unsigned short> indices;
    int sides = 6;
    float h2 = height / 2.0f;

    // Helper to add a flat quad for a side
    auto addFlatSide = [&](int i) {
        int next = (i + 1) % sides;
        float ang1 = (float)i / sides * 2.0f * PI;
        float ang2 = (float)next / sides * 2.0f * PI;

        unsigned short baseIdx = (unsigned short)(vertices.size() / 3);

        // Define 4 unique vertices for this specific panel
        // Bottom Right
        vertices.push_back(cosf(ang1) * bottomRad); vertices.push_back(-h2); vertices.push_back(sinf(ang1) * bottomRad);
        // Bottom Left
        vertices.push_back(cosf(ang2) * bottomRad); vertices.push_back(-h2); vertices.push_back(sinf(ang2) * bottomRad);
        // Top Left
        vertices.push_back(cosf(ang2) * topRad);    vertices.push_back(h2);  vertices.push_back(sinf(ang2) * topRad);
        // Top Right
        vertices.push_back(cosf(ang1) * topRad);    vertices.push_back(h2);  vertices.push_back(sinf(ang1) * topRad);

        indices.push_back(baseIdx + 0); indices.push_back(baseIdx + 2); indices.push_back(baseIdx + 1);
        indices.push_back(baseIdx + 0); indices.push_back(baseIdx + 3); indices.push_back(baseIdx + 2);
    };

    for (int i = 0; i < sides; i++) addFlatSide(i);

    // Finalize Mesh
    Mesh mesh = { 0 };
    mesh.vertexCount = (int)vertices.size() / 3;
    mesh.triangleCount = (int)indices.size() / 3;
    mesh.vertices = (float*)RL_MALLOC(vertices.size() * sizeof(float));
    mesh.indices = (unsigned short*)RL_MALLOC(indices.size() * sizeof(unsigned short));
    std::memcpy(mesh.vertices, vertices.data(), vertices.size() * sizeof(float));
    std::memcpy(mesh.indices, indices.data(), indices.size() * sizeof(unsigned short));
    
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
    return mesh;
}

static Mesh CreatePlasmaCannon(float radius, float length) {
    std::vector<float> vertices;
    std::vector<unsigned short> indices;
    int sides = 8;
    
    // Helper to add a flat-shaded cylinder segment
    auto addSegment = [&](float rBottom, float rTop, float h, float yOffset) {
        float h2 = h / 2.0f;
        for (int i = 0; i < sides; i++) {
            int next = (i + 1) % sides;
            float ang1 = (float)i / sides * 2.0f * PI;
            float ang2 = (float)next / sides * 2.0f * PI;

            unsigned short baseIdx = (unsigned short)(vertices.size() / 3);

            // 4 vertices per side panel for flat shading
            Vector3 v1 = {cosf(ang1) * rBottom, yOffset - h2, sinf(ang1) * rBottom};
            Vector3 v2 = {cosf(ang2) * rBottom, yOffset - h2, sinf(ang2) * rBottom};
            Vector3 v3 = {cosf(ang2) * rTop,    yOffset + h2, sinf(ang2) * rTop};
            Vector3 v4 = {cosf(ang1) * rTop,    yOffset + h2, sinf(ang1) * rTop};

            vertices.insert(vertices.end(), {v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, v3.x, v3.y, v3.z, v4.x, v4.y, v4.z});

            indices.push_back(baseIdx + 0); indices.push_back(baseIdx + 2); indices.push_back(baseIdx + 1);
            indices.push_back(baseIdx + 0); indices.push_back(baseIdx + 3); indices.push_back(baseIdx + 2);
        }
    };

    // Part A: Cooling Shroud (The thick heavy base)
    float shroudLen = length * 0.35f;
    float shroudRad = radius * 1.8f;
    addSegment(shroudRad, shroudRad * 0.9f, shroudLen, -length * 0.3f);

    // Part B: Main Barrel
    float barrelLen = length * 0.6f;
    addSegment(radius, radius, barrelLen, length * 0.15f);

    // Part C: Muzzle Brake (The tip)
    float muzzleLen = length * 0.1f;
    addSegment(radius * 1.2f, radius * 1.1f, muzzleLen, length * 0.45f);

    // Finalize Mesh
    Mesh mesh = { 0 };
    mesh.vertexCount = (int)vertices.size() / 3;
    mesh.triangleCount = (int)indices.size() / 3;
    mesh.vertices = (float*)RL_MALLOC(vertices.size() * sizeof(float));
    mesh.indices = (unsigned short*)RL_MALLOC(indices.size() * sizeof(unsigned short));
    std::memcpy(mesh.vertices, vertices.data(), vertices.size() * sizeof(float));
    std::memcpy(mesh.indices, indices.data(), indices.size() * sizeof(unsigned short));
    
    return mesh;
}

static Mesh CreateArmorPlate(float width, float height, float thickness) {
    // We can use a slightly scaled cube for the armor plate
    Mesh mesh = GenMeshCube(width, height, thickness);
    return mesh; 
}

ProceduralMech AssembleMech() {
    ProceduralMech mech;
    const float scale = 0.4f; 
    
    // --- LEGS (BattleTech Style) ---
    float stanceWidth = 0.6f * scale;

    for (int side = -1; side <= 1; side += 2) { // -1 for Left, 1 for Right
        float x = side * stanceWidth;

        // 1. Massive Foot (Using your custom Foot mesh)
        mech.AddPart(CreateMechFoot(0.4f * scale, 0.9f * scale), MatrixTranslate(x, 0.1f * scale, 0.1f * scale));

        // 2. Ankle Joint
        mech.AddPart(CreateSimpleSphere(0.15f * scale, 8, 8), MatrixTranslate(x, 0.25f * scale, 0));

        // 3. Lower Leg (Tapered Hex)
        mech.AddPart(CreateArmoredLegPart(0.15f * scale, 0.25f * scale, 1.2f * scale), 
                    MatrixTranslate(x, 0.85f * scale, 0));

        // 4. Knee Joint (Bulge)
        mech.AddPart(CreateSimpleSphere(0.28f * scale, 8, 8), MatrixTranslate(x, 1.45f * scale, 0.1f * scale));


        // 5. Upper Leg
        Matrix rotation = MatrixRotateX(-15 * DEG2RAD); // Predatory lean
        Matrix translation = MatrixTranslate(x, 2.2f * scale, 0);
        Matrix thighTransform = MatrixMultiply(rotation, translation);

        mech.AddPart(CreateArmoredLegPart(0.25f * scale, 0.35f * scale, 1.4f * scale), thighTransform);

        // 6. Hip Actuator (CORRECTED MATRIX ORDER)
        // Rotate the cylinder 90 degrees to make it a horizontal axle
        Matrix hipJointMatrix = MatrixMultiply(MatrixRotateZ(90 * DEG2RAD), MatrixTranslate(x * 0.7f, 2.9f * scale, 0));
        mech.AddPart(CreateSimpleCylinder(0.2f * scale, 0.4f * scale, 6), hipJointMatrix);
    }

    // --- UPPER BODY ---
    // Pelvis: Widened to match the new stanceWidth
    mech.AddPart(GenMeshCube(1.2f * scale, 0.4f * scale, 0.8f * scale), MatrixTranslate(0, 3.0f * scale, 0));
    
    // --- ARMS & SHOULDERS ---
    for (int side = -1; side <= 1; side += 2) {
        float xPos = side * 0.85f * scale; 
        float yPos = 4.8f * scale;

        // 1. Shoulder Joint (The Sphere)
        mech.AddPart(CreateSimpleSphere(0.3f * scale, 8, 8), MatrixTranslate(side * 0.8f * scale, yPos, 0));

        // 2. NEW: ARMOR SHIELD (The Pauldron)
        // RotateZ tilts it over the shoulder, RotateY angles it slightly forward
        float shieldAngle = side * -25.0f; // Tilt outward
        Matrix shieldRot = MatrixMultiply(MatrixRotateY(10 * DEG2RAD), MatrixRotateZ(shieldAngle * DEG2RAD));
        // Offset it slightly above and outside the sphere center
        Matrix shieldTrans = MatrixTranslate(xPos + (side * 0.1f * scale), yPos + (0.3f * scale), 0);
        
        mech.AddPart(CreateArmorPlate(0.6f * scale, 0.5f * scale, 0.1f * scale), 
                    MatrixMultiply(shieldRot, shieldTrans));

        // 3. Shoulder Actuator (The "Joint Cube")
        mech.AddPart(GenMeshCube(0.4f * scale, 0.4f * scale, 0.4f * scale), 
                    MatrixTranslate(xPos, yPos, 0));

        // 4. The Armature (Horizontal Connector)
        mech.AddPart(GenMeshCube(0.3f * scale, 0.2f * scale, 0.2f * scale), 
                    MatrixTranslate(xPos + (side * 0.2f * scale), yPos, 0));

        // 5. Weapon Systems (Plasma/Rockets)
        if (side == -1) {
            // Left Arm: Dual Plasma
            Matrix weaponRot = MatrixMultiply(MatrixRotateX(90 * DEG2RAD), MatrixTranslate(xPos + (side * 0.3f * scale), yPos - 0.2f * scale, 0.4f * scale));
            mech.AddPart(CreateSimpleCylinder(0.12f * scale, 1.2f * scale, 8), weaponRot);
            Matrix weaponRot2 = MatrixMultiply(MatrixRotateX(90 * DEG2RAD), MatrixTranslate(xPos + (side * 0.15f * scale), yPos - 0.2f * scale, 0.4f * scale));
            mech.AddPart(CreateSimpleCylinder(0.12f * scale, 1.2f * scale, 8), weaponRot2);
        } else {
            // Right Arm: Rocket Pod
            Matrix podTransform = MatrixMultiply(MatrixRotateY(0), MatrixTranslate(xPos + (side * 0.3f * scale), yPos, 0.2f * scale));
            mech.AddPart(CreateRocketPod(0.7f * scale, 0.8f * scale, 0.6f * scale), podTransform);
        }
    }
    // Torso: Increased radius to 0.75f to look "heavy" and armored
    mech.AddPart(CreateSimpleCylinder(0.75f * scale, 2.0f * scale, 6), MatrixTranslate(0, 4.0f * scale, 0));
    
    // Neck and Head
    mech.AddPart(CreateSimpleCylinder(0.2f * scale, 0.35f * scale, 8), MatrixTranslate(0, 5.2f * scale, 0));
    mech.AddPart(CreateMechHead(0.45f * scale, 0.5f * scale), MatrixTranslate(0, 5.45f * scale, 0.12f * scale));

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
    mesh.normals = (float*)RL_MALLOC(verts.size() * sizeof(float));

    std::memcpy(mesh.vertices, verts.data(), verts.size() * sizeof(float));
    std::memcpy(mesh.indices, indices.data(), indices.size() * sizeof(unsigned short));
    std::fill(mesh.normals, mesh.normals + verts.size(), 0.0f);

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