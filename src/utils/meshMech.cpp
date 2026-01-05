// Procedural mech generation and merging into a single mesh
#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <algorithm>
#include "meshProcessUtils.h"
#include "meshGenerateUtils.h"
#include "luaUtils.h"

struct MechConfig {
    float scale;
    float stance_width;
    float foot_width;
    float foot_length;
    float foot_height;
    float foot_bottom_back_frac;
    float foot_bottom_front_frac;
    float foot_top_back_frac;
    float foot_top_front_frac;
    float foot_top_width_scale;
    float foot_y_offset_frac;
    float foot_z_offset_frac;
    float ankle_radius;
    float lower_leg_bottom;
    float lower_leg_top;
    float lower_leg_height;
    float knee_radius;
    float knee_z_offset;
    float upper_leg_bottom;
    float upper_leg_top;
    float upper_leg_height;
    float thigh_angle_deg;
    float upper_leg_extra_y;
    float hip_radius;
    float hip_length;
    float hip_x_offset;
    float pelvis_w;
    float pelvis_h;
    float pelvis_d;
    float pelvis_y;
    float shoulder_sphere_r;
    float shoulder_y;
    float shoulder_x;
    float shoulder_sphere_inset;
    float shield_angle_deg;
    float shield_rot_y_deg;
    float shield_dx;
    float shield_dy;
    float shield_w;
    float shield_h;
    float shield_t;
    float shoulder_cube;
    float armature_w;
    float armature_h;
    float armature_d;
    float armature_offset;
    float plasma_radius;
    float plasma_length;
    float plasma_y;
    float plasma_z;
    float plasma_x;
    float plasma_x2;
    float plasma_shroud_len_frac;
    float plasma_shroud_radius_scale;
    float plasma_shroud_taper;
    float plasma_shroud_offset_frac;
    float plasma_barrel_len_frac;
    float plasma_barrel_offset_frac;
    float plasma_muzzle_len_frac;
    float plasma_muzzle_radius_scale;
    float plasma_muzzle_tip_scale;
    float plasma_muzzle_offset_frac;
    float rocket_w;
    float rocket_h;
    float rocket_d;
    float rocket_z;
    float rocket_x;
    float torso_r;
    float torso_h;
    float torso_y;
    float neck_r;
    float neck_h;
    float neck_y;
    float head_w;
    float head_h;
    float head_z;
    int left_weapon;   // 0=plasma, 1=rocket
    int right_weapon;  // 0=plasma, 1=rocket

    MechConfig();
};

MechConfig::MechConfig()
    : scale(0.4f)
    , stance_width(0.6f)
    , foot_width(0.4f)
    , foot_length(0.9f)
    , foot_height(0.3f)
    , foot_bottom_back_frac(0.5f)
    , foot_bottom_front_frac(0.8f)
    , foot_top_back_frac(0.4f)
    , foot_top_front_frac(0.2f)
    , foot_top_width_scale(0.6f)
    , foot_y_offset_frac(0.3333333f) // 0.1 / 0.3 original offset
    , foot_z_offset_frac(0.1111111f) // 0.1 / 0.9 original offset
    , ankle_radius(0.15f)
    , lower_leg_bottom(0.15f)
    , lower_leg_top(0.25f)
    , lower_leg_height(1.2f)
    , knee_radius(0.28f)
    , knee_z_offset(0.1f)
    , upper_leg_bottom(0.25f)
    , upper_leg_top(0.35f)
    , upper_leg_height(1.4f)
    , thigh_angle_deg(-15.0f)
    , upper_leg_extra_y(0.05f)
    , hip_radius(0.2f)
    , hip_length(0.4f)
    , hip_x_offset(0.7f)
    , pelvis_w(1.2f)
    , pelvis_h(0.4f)
    , pelvis_d(0.8f)
    , pelvis_y(3.0f)
    , shoulder_sphere_r(0.3f)
    , shoulder_y(4.8f)
    , shoulder_x(0.85f)
    , shoulder_sphere_inset(0.05f)
    , shield_angle_deg(-25.0f)
    , shield_rot_y_deg(10.0f)
    , shield_dx(0.1f)
    , shield_dy(0.3f)
    , shield_w(0.6f)
    , shield_h(0.5f)
    , shield_t(0.1f)
    , shoulder_cube(0.4f)
    , armature_w(0.3f)
    , armature_h(0.2f)
    , armature_d(0.2f)
    , armature_offset(0.2f)
    , plasma_radius(0.12f)
    , plasma_length(1.2f)
    , plasma_y(-0.2f)
    , plasma_z(0.4f)
    , plasma_x(0.3f)
    , plasma_x2(0.15f)
    , plasma_shroud_len_frac(0.35f)
    , plasma_shroud_radius_scale(1.8f)
    , plasma_shroud_taper(0.9f)
    , plasma_shroud_offset_frac(-0.3f)
    , plasma_barrel_len_frac(0.6f)
    , plasma_barrel_offset_frac(0.15f)
    , plasma_muzzle_len_frac(0.1f)
    , plasma_muzzle_radius_scale(1.2f)
    , plasma_muzzle_tip_scale(1.1f)
    , plasma_muzzle_offset_frac(0.45f)
    , rocket_w(0.7f)
    , rocket_h(0.8f)
    , rocket_d(0.6f)
    , rocket_z(0.2f)
    , rocket_x(0.3f)
    , torso_r(0.75f)
    , torso_h(2.0f)
    , torso_y(4.0f)
    , neck_r(0.2f)
    , neck_h(0.35f)
    , neck_y(5.2f)
    , head_w(0.45f)
    , head_h(0.5f)
    , head_z(0.12f)
    , left_weapon(0)   // default: plasma
    , right_weapon(1) {} // default: rocket

static MechConfig LoadMechConfig(const std::string& path) {
    MechConfig cfg;
    std::ifstream f(path);
    if (!f.is_open()) return cfg;
    std::stringstream buf; buf << f.rdbuf();
    SimpleLuaParser p;
    try { p.Parse(buf.str()); } catch (...) { return cfg; }

    auto get = [&](const char* key, float def) {
        return ParseFloat(p.GetTableValue("mech", key, SimpleLuaParser::NumberToString(def)), def);
    };

    cfg.scale = get("scale", cfg.scale);
    cfg.stance_width = get("stance_width", cfg.stance_width);
    cfg.foot_width = get("foot_width", cfg.foot_width);
    cfg.foot_length = get("foot_length", cfg.foot_length);
    cfg.foot_height = get("foot_height", cfg.foot_height);
    cfg.foot_bottom_back_frac = get("foot_bottom_back_frac", cfg.foot_bottom_back_frac);
    cfg.foot_bottom_front_frac = get("foot_bottom_front_frac", cfg.foot_bottom_front_frac);
    cfg.foot_top_back_frac = get("foot_top_back_frac", cfg.foot_top_back_frac);
    cfg.foot_top_front_frac = get("foot_top_front_frac", cfg.foot_top_front_frac);
    cfg.foot_top_width_scale = get("foot_top_width_scale", cfg.foot_top_width_scale);
    cfg.foot_y_offset_frac = get("foot_y_offset_frac", cfg.foot_y_offset_frac);
    cfg.foot_z_offset_frac = get("foot_z_offset_frac", cfg.foot_z_offset_frac);
    cfg.ankle_radius = get("ankle_radius", cfg.ankle_radius);
    cfg.lower_leg_bottom = get("lower_leg_bottom", cfg.lower_leg_bottom);
    cfg.lower_leg_top = get("lower_leg_top", cfg.lower_leg_top);
    cfg.lower_leg_height = get("lower_leg_height", cfg.lower_leg_height);
    cfg.knee_radius = get("knee_radius", cfg.knee_radius);
    cfg.knee_z_offset = get("knee_z_offset", cfg.knee_z_offset);
    cfg.upper_leg_bottom = get("upper_leg_bottom", cfg.upper_leg_bottom);
    cfg.upper_leg_top = get("upper_leg_top", cfg.upper_leg_top);
    cfg.upper_leg_height = get("upper_leg_height", cfg.upper_leg_height);
    cfg.thigh_angle_deg = get("thigh_angle_deg", cfg.thigh_angle_deg);
    cfg.upper_leg_extra_y = get("upper_leg_extra_y", cfg.upper_leg_extra_y);
    cfg.hip_radius = get("hip_radius", cfg.hip_radius);
    cfg.hip_length = get("hip_length", cfg.hip_length);
    cfg.hip_x_offset = get("hip_x_offset", cfg.hip_x_offset);
    cfg.pelvis_w = get("pelvis_w", cfg.pelvis_w);
    cfg.pelvis_h = get("pelvis_h", cfg.pelvis_h);
    cfg.pelvis_d = get("pelvis_d", cfg.pelvis_d);
    cfg.pelvis_y = get("pelvis_y", cfg.pelvis_y);
    cfg.shoulder_sphere_r = get("shoulder_sphere_r", cfg.shoulder_sphere_r);
    cfg.shoulder_y = get("shoulder_y", cfg.shoulder_y);
    cfg.shoulder_x = get("shoulder_x", cfg.shoulder_x);
    cfg.shoulder_sphere_inset = get("shoulder_sphere_inset", cfg.shoulder_sphere_inset);
    cfg.shield_angle_deg = get("shield_angle_deg", cfg.shield_angle_deg);
    cfg.shield_rot_y_deg = get("shield_rot_y_deg", cfg.shield_rot_y_deg);
    cfg.shield_dx = get("shield_dx", cfg.shield_dx);
    cfg.shield_dy = get("shield_dy", cfg.shield_dy);
    cfg.shield_w = get("shield_w", cfg.shield_w);
    cfg.shield_h = get("shield_h", cfg.shield_h);
    cfg.shield_t = get("shield_t", cfg.shield_t);
    cfg.shoulder_cube = get("shoulder_cube", cfg.shoulder_cube);
    cfg.armature_w = get("armature_w", cfg.armature_w);
    cfg.armature_h = get("armature_h", cfg.armature_h);
    cfg.armature_d = get("armature_d", cfg.armature_d);
    cfg.armature_offset = get("armature_offset", cfg.armature_offset);
    cfg.plasma_radius = get("plasma_radius", cfg.plasma_radius);
    cfg.plasma_length = get("plasma_length", cfg.plasma_length);
    cfg.plasma_y = get("plasma_y", cfg.plasma_y);
    cfg.plasma_z = get("plasma_z", cfg.plasma_z);
    cfg.plasma_x = get("plasma_x", cfg.plasma_x);
    cfg.plasma_x2 = get("plasma_x2", cfg.plasma_x2);
    cfg.plasma_shroud_len_frac = get("plasma_shroud_len_frac", cfg.plasma_shroud_len_frac);
    cfg.plasma_shroud_radius_scale = get("plasma_shroud_radius_scale", cfg.plasma_shroud_radius_scale);
    cfg.plasma_shroud_taper = get("plasma_shroud_taper", cfg.plasma_shroud_taper);
    cfg.plasma_shroud_offset_frac = get("plasma_shroud_offset_frac", cfg.plasma_shroud_offset_frac);
    cfg.plasma_barrel_len_frac = get("plasma_barrel_len_frac", cfg.plasma_barrel_len_frac);
    cfg.plasma_barrel_offset_frac = get("plasma_barrel_offset_frac", cfg.plasma_barrel_offset_frac);
    cfg.plasma_muzzle_len_frac = get("plasma_muzzle_len_frac", cfg.plasma_muzzle_len_frac);
    cfg.plasma_muzzle_radius_scale = get("plasma_muzzle_radius_scale", cfg.plasma_muzzle_radius_scale);
    cfg.plasma_muzzle_tip_scale = get("plasma_muzzle_tip_scale", cfg.plasma_muzzle_tip_scale);
    cfg.plasma_muzzle_offset_frac = get("plasma_muzzle_offset_frac", cfg.plasma_muzzle_offset_frac);
    cfg.rocket_w = get("rocket_w", cfg.rocket_w);
    cfg.rocket_h = get("rocket_h", cfg.rocket_h);
    cfg.rocket_d = get("rocket_d", cfg.rocket_d);
    cfg.rocket_z = get("rocket_z", cfg.rocket_z);
    cfg.rocket_x = get("rocket_x", cfg.rocket_x);
    cfg.torso_r = get("torso_r", cfg.torso_r);
    cfg.torso_h = get("torso_h", cfg.torso_h);
    cfg.torso_y = get("torso_y", cfg.torso_y);
    cfg.neck_r = get("neck_r", cfg.neck_r);
    cfg.neck_h = get("neck_h", cfg.neck_h);
    cfg.neck_y = get("neck_y", cfg.neck_y);
    cfg.head_w = get("head_w", cfg.head_w);
    cfg.head_h = get("head_h", cfg.head_h);
    cfg.head_z = get("head_z", cfg.head_z);
    cfg.left_weapon = (int)get("left_weapon", (float)cfg.left_weapon);
    cfg.right_weapon = (int)get("right_weapon", (float)cfg.right_weapon);
    return cfg;
}

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
    int bottomCenterIdx = (int)(vertices.size() / 3);
    vertices.push_back(0.0f);
    vertices.push_back(-halfHeight);
    vertices.push_back(0.0f);

    // Top cap center
    int topCenterIdx = (int)(vertices.size() / 3);
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

    // Chamfer ring near the front (12-15)
    float noseZ = f + rackDepth * 0.15f;
    float noseW = width * 0.85f;
    float noseH = height * 0.85f;
    vertices.insert(vertices.end(), {
        -noseW/2, -noseH/2, noseZ,
         noseW/2, -noseH/2, noseZ,
         noseW/2,  noseH/2, noseZ,
        -noseW/2,  noseH/2, noseZ
    });

    // Nose cap ring (front) (16-19) slightly forward to avoid z-fighting
    float noseCapZ = noseZ + rackDepth * 0.08f;
    float capW = noseW * 0.9f;
    float capH = noseH * 0.9f;
    vertices.insert(vertices.end(), {
        -capW/2, -capH/2, noseCapZ,
         capW/2, -capH/2, noseCapZ,
         capW/2,  capH/2, noseCapZ,
        -capW/2,  capH/2, noseCapZ
    });

    // Nose center (20) pushed forward a hair
    vertices.insert(vertices.end(), {0.0f, 0.0f, noseCapZ + rackDepth * 0.04f});

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

    // The actual Launcher Plate (The recessed surface) + backface for visibility
    addQuad(8, 9, 10, 11);
    addQuad(11, 10, 9, 8);

    // Chamfer to nose ring
    addQuad(4, 5, 13, 12); // bottom
    addQuad(7, 15, 14, 6); // top
    addQuad(4, 12, 15, 7); // left
    addQuad(5, 6, 14, 13); // right

    // Nose front cap (double-sided) using front ring verts
    addQuad(16, 17, 18, 19);   // front CCW facing +Z
    addQuad(19, 18, 17, 16);   // back face
    // Small fan from center to give facet detail
    indices.push_back(20); indices.push_back(16); indices.push_back(17);
    indices.push_back(20); indices.push_back(17); indices.push_back(18);
    indices.push_back(20); indices.push_back(18); indices.push_back(19);
    indices.push_back(20); indices.push_back(19); indices.push_back(16);

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
Mesh CreateMechFoot(float width, float length, float height, const MechConfig& cfg) {
    std::vector<float> vertices;
    std::vector<unsigned short> indices;
    float h = height; // Height of the foot

    float bottomBackZ = -length * cfg.foot_bottom_back_frac;
    float bottomFrontZ = length * cfg.foot_bottom_front_frac;
    float topBackZ = -length * cfg.foot_top_back_frac;
    float topFrontZ = length * cfg.foot_top_front_frac;
    float topWidthScale = cfg.foot_top_width_scale;

    // 1. Define the 8 vertices as requested
    Vector3 pts[8] = {
        {-width, 0, bottomBackZ},               // 0: Back Bottom Left
        {width, 0, bottomBackZ},                // 1: Back Bottom Right
        {width, 0,  bottomFrontZ},              // 2: Front Bottom Right (Extended)
        {-width, 0,  bottomFrontZ},             // 3: Front Bottom Left (Extended)
        {-width * topWidthScale, h, topBackZ},  // 4: Back Top Left
        {width * topWidthScale, h, topBackZ},   // 5: Back Top Right
        {width * topWidthScale, h,  topFrontZ}, // 6: Front Top Right (Recessed)
        {-width * topWidthScale, h,  topFrontZ} // 7: Front Top Left (Recessed)
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

static Mesh CreatePlasmaCannon(const MechConfig& cfg, float scale) {
    std::vector<float> vertices;
    std::vector<unsigned short> indices;
    int sides = 8;

    float radius = cfg.plasma_radius * scale;
    float length = cfg.plasma_length * scale;
    
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
    float shroudLen = length * cfg.plasma_shroud_len_frac;
    float shroudRad = radius * cfg.plasma_shroud_radius_scale;
    float barrelLen = length * cfg.plasma_barrel_len_frac;
    float muzzleLen = length * cfg.plasma_muzzle_len_frac;
    addSegment(shroudRad, shroudRad * cfg.plasma_shroud_taper, shroudLen, length * cfg.plasma_shroud_offset_frac);

    // Cap the rear (shoulder joint side) of the cannon so the base isn't open
    float shroudStart = length * cfg.plasma_shroud_offset_frac - shroudLen * 0.5f;
    float barrelStart = length * cfg.plasma_barrel_offset_frac - barrelLen * 0.5f;
    float muzzleStart = length * cfg.plasma_muzzle_offset_frac - muzzleLen * 0.5f;
    float backCapY = std::min(shroudStart, std::min(barrelStart, muzzleStart));
    float backCapR = std::max(shroudRad, std::max(radius, radius * cfg.plasma_muzzle_radius_scale));
    unsigned short backCenter = static_cast<unsigned short>(vertices.size() / 3);
    vertices.insert(vertices.end(), {0.0f, backCapY, 0.0f});
    unsigned short backRingStart = backCenter + 1;
    for (int i = 0; i < sides; ++i) {
        float ang = 2.0f * PI * i / sides;
        float x = backCapR * cosf(ang);
        float z = backCapR * sinf(ang);
        vertices.push_back(x);
        vertices.push_back(backCapY);
        vertices.push_back(z);
    }
    for (int i = 0; i < sides; ++i) {
        unsigned short a = backRingStart + i;
        unsigned short b = backRingStart + ((i + 1) % sides);
        indices.push_back(backCenter); // facing -Y
        indices.push_back(b);
        indices.push_back(a);
    }
    for (int i = 0; i < sides; ++i) {
        unsigned short a = backRingStart + i;
        unsigned short b = backRingStart + ((i + 1) % sides);
        indices.push_back(backCenter); // backside to ensure visibility
        indices.push_back(a);
        indices.push_back(b);
    }

    // Part B: Main Barrel
    addSegment(radius, radius, barrelLen, length * cfg.plasma_barrel_offset_frac);

    // Part C: Muzzle Brake (The tip)
    addSegment(radius * cfg.plasma_muzzle_radius_scale, radius * cfg.plasma_muzzle_tip_scale, muzzleLen, length * cfg.plasma_muzzle_offset_frac);

    auto addBox = [&](float cx, float cy, float cz, float sx, float sy, float sz) {
        unsigned short base = static_cast<unsigned short>(vertices.size() / 3);
        // 8 verts
        float hx = sx * 0.5f, hy = sy * 0.5f, hz = sz * 0.5f;
        float pts[8][3] = {
            {cx - hx, cy - hy, cz - hz}, {cx + hx, cy - hy, cz - hz},
            {cx + hx, cy + hy, cz - hz}, {cx - hx, cy + hy, cz - hz},
            {cx - hx, cy - hy, cz + hz}, {cx + hx, cy - hy, cz + hz},
            {cx + hx, cy + hy, cz + hz}, {cx - hx, cy + hy, cz + hz}
        };
        for (auto& p : pts) { vertices.insert(vertices.end(), {p[0], p[1], p[2]}); }
        auto quad = [&](unsigned short a, unsigned short b, unsigned short c, unsigned short d) {
            indices.push_back(base + a); indices.push_back(base + b); indices.push_back(base + c);
            indices.push_back(base + a); indices.push_back(base + c); indices.push_back(base + d);
        };
        quad(0,1,2,3); // back
        quad(4,5,6,7); // front
        quad(0,4,7,3); // left
        quad(1,5,6,2); // right
        quad(3,2,6,7); // top
        quad(0,1,5,4); // bottom
    };

    // Add small fins for visual interest
    float finLen = length * 0.35f;
    float finThick = radius * 0.18f;
    float finWide = radius * 0.9f;
    float finZ = length * 0.1f;
    addBox(0.0f, 0.0f, finZ, finWide, finThick, finLen); // top/bottom thin box (will look like a slab)
    addBox(0.0f, 0.0f, finZ, finThick, finWide, finLen); // side slab rotated (still axis-aligned adds cross-fin)

    // Cap the muzzle front to avoid seeing through the barrel
    float capY = length * cfg.plasma_muzzle_offset_frac + muzzleLen * 0.5f;
    float capR = radius * cfg.plasma_muzzle_tip_scale;
    unsigned short centerIdx = static_cast<unsigned short>(vertices.size() / 3);
    vertices.insert(vertices.end(), {0.0f, capY, 0.0f});
    // Ring vertices
    unsigned short ringStart = centerIdx + 1;
    for (int i = 0; i < sides; ++i) {
        float ang = 2.0f * PI * i / sides;
        float x = capR * cosf(ang);
        float z = capR * sinf(ang);
        vertices.push_back(x);
        vertices.push_back(capY);
        vertices.push_back(z);
    }
    // Front cap (facing +Y)
    for (int i = 0; i < sides; ++i) {
        unsigned short a = ringStart + i;
        unsigned short b = ringStart + ((i + 1) % sides);
        indices.push_back(centerIdx);
        indices.push_back(a);
        indices.push_back(b);
    }
    // Back face to ensure visibility from inside
    for (int i = 0; i < sides; ++i) {
        unsigned short a = ringStart + i;
        unsigned short b = ringStart + ((i + 1) % sides);
        indices.push_back(centerIdx);
        indices.push_back(b);
        indices.push_back(a);
    }

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

ProceduralMech AssembleMech(const MechConfig& cfg) {
    ProceduralMech mech;
    const float scale = cfg.scale; 
    
    // --- LEGS (BattleTech Style) ---
    float stanceWidth = cfg.stance_width * scale;
    float footYOffset = cfg.foot_height * cfg.foot_y_offset_frac * scale;
    float footZOffset = cfg.foot_length * cfg.foot_z_offset_frac * scale;

    for (int side = -1; side <= 1; side += 2) { // -1 for Left, 1 for Right
        float x = side * stanceWidth;

        // 1. Massive Foot (Using your custom Foot mesh)
        mech.AddPart(CreateMechFoot(cfg.foot_width * scale, cfg.foot_length * scale, cfg.foot_height * scale, cfg), MatrixTranslate(x, footYOffset, footZOffset));

        // 2. Ankle Joint
        float ankleY = cfg.ankle_radius * (0.25f / 0.15f) * scale;
        mech.AddPart(CreateSimpleSphere(cfg.ankle_radius * scale, 8, 8), MatrixTranslate(x, ankleY, 0));

        // 3. Lower Leg (Tapered Hex)
        float lowerLegCenterY = cfg.lower_leg_height * (0.85f / 1.2f) * scale;
        mech.AddPart(CreateArmoredLegPart(cfg.lower_leg_bottom * scale, cfg.lower_leg_top * scale, cfg.lower_leg_height * scale), 
                    MatrixTranslate(x, lowerLegCenterY, 0));

        // 4. Knee Joint (Bulge)
        float kneeY = lowerLegCenterY + (cfg.lower_leg_height * 0.5f * scale);
        mech.AddPart(CreateSimpleSphere(cfg.knee_radius * scale, 8, 8), MatrixTranslate(x, kneeY, cfg.knee_z_offset * scale));

        // 5. Upper Leg
        Matrix rotation = MatrixRotateX(cfg.thigh_angle_deg * DEG2RAD); // Predatory lean
        float upperLegCenterY = kneeY + (cfg.upper_leg_height * 0.5f * scale) + (cfg.upper_leg_extra_y * scale);
        Matrix translation = MatrixTranslate(x, upperLegCenterY, 0);
        Matrix thighTransform = MatrixMultiply(rotation, translation);

        mech.AddPart(CreateArmoredLegPart(cfg.upper_leg_bottom * scale, cfg.upper_leg_top * scale, cfg.upper_leg_height * scale), thighTransform);

        // 6. Hip Actuator (CORRECTED MATRIX ORDER)
        // Rotate the cylinder 90 degrees to make it a horizontal axle
        float hipY = upperLegCenterY + (cfg.upper_leg_height * 0.5f * scale);
        Matrix hipJointMatrix = MatrixMultiply(MatrixRotateZ(90 * DEG2RAD), MatrixTranslate(x * (cfg.hip_x_offset / 0.7f), hipY, 0));
        mech.AddPart(CreateSimpleCylinder(cfg.hip_radius * scale, cfg.hip_length * scale, 6), hipJointMatrix);
    }

    // --- UPPER BODY ---
    // Pelvis: Widened to match the new stanceWidth
    mech.AddPart(GenMeshCube(cfg.pelvis_w * scale, cfg.pelvis_h * scale, cfg.pelvis_d * scale), MatrixTranslate(0, cfg.pelvis_y * scale, 0));
    
    // --- ARMS & SHOULDERS ---
    for (int side = -1; side <= 1; side += 2) {
        float xPos = side * cfg.shoulder_x * scale; 
        float yPos = cfg.shoulder_y * scale;

        // 1. Shoulder Joint (The Sphere)
        mech.AddPart(CreateSimpleSphere(cfg.shoulder_sphere_r * scale, 8, 8), MatrixTranslate(side * (cfg.shoulder_x - cfg.shoulder_sphere_inset) * scale, yPos, 0));

        // 2. NEW: ARMOR SHIELD (The Pauldron)
        // RotateZ tilts it over the shoulder, RotateY angles it slightly forward
        float shieldAngle = side * cfg.shield_angle_deg; // Tilt outward
        Matrix shieldRot = MatrixMultiply(MatrixRotateY(cfg.shield_rot_y_deg * DEG2RAD), MatrixRotateZ(shieldAngle * DEG2RAD));
        // Offset it slightly above and outside the sphere center
        Matrix shieldTrans = MatrixTranslate(xPos + (side * cfg.shield_dx * scale), yPos + (cfg.shield_dy * scale), 0);
        
        mech.AddPart(CreateArmorPlate(cfg.shield_w * scale, cfg.shield_h * scale, cfg.shield_t * scale), 
                    MatrixMultiply(shieldRot, shieldTrans));

        // 3. Shoulder Actuator (The "Joint Cube")
        mech.AddPart(GenMeshCube(cfg.shoulder_cube * scale, cfg.shoulder_cube * scale, cfg.shoulder_cube * scale), 
                    MatrixTranslate(xPos, yPos, 0));

        // 4. The Armature (Horizontal Connector)
        mech.AddPart(GenMeshCube(cfg.armature_w * scale, cfg.armature_h * scale, cfg.armature_d * scale), 
                MatrixTranslate(xPos + (side * cfg.armature_offset * scale), yPos, 0));

        // 5. Weapon Systems (Plasma/Rockets)
        if (side == -1) {
            // Left Arm
            if (cfg.left_weapon == 0) {
                // Dual Plasma
                Matrix weaponRot = MatrixMultiply(MatrixRotateX(90 * DEG2RAD), MatrixTranslate(xPos + (side * cfg.plasma_x * scale), yPos + (cfg.plasma_y * scale), cfg.plasma_z * scale));
                mech.AddPart(CreatePlasmaCannon(cfg, scale), weaponRot);
                Matrix weaponRot2 = MatrixMultiply(MatrixRotateX(90 * DEG2RAD), MatrixTranslate(xPos + (side * cfg.plasma_x2 * scale), yPos + (cfg.plasma_y * scale), cfg.plasma_z * scale));
                mech.AddPart(CreatePlasmaCannon(cfg, scale), weaponRot2);
            } else {
                // Dual Rockets
                Matrix podTransform1 = MatrixMultiply(MatrixRotateY(0), MatrixTranslate(xPos + (side * cfg.rocket_x * scale), yPos + (cfg.rocket_h * 0.3f * scale), cfg.rocket_z * scale));
                mech.AddPart(CreateRocketPod(cfg.rocket_w * scale, cfg.rocket_h * scale, cfg.rocket_d * scale), podTransform1);
                Matrix podTransform2 = MatrixMultiply(MatrixRotateY(0), MatrixTranslate(xPos + (side * cfg.rocket_x * scale), yPos - (cfg.rocket_h * 0.3f * scale), cfg.rocket_z * scale));
                mech.AddPart(CreateRocketPod(cfg.rocket_w * scale, cfg.rocket_h * scale, cfg.rocket_d * scale), podTransform2);
            }
        } else {
            // Right Arm
            if (cfg.right_weapon == 0) {
                // Dual Plasma
                Matrix weaponRot = MatrixMultiply(MatrixRotateX(90 * DEG2RAD), MatrixTranslate(xPos + (side * cfg.plasma_x * scale), yPos + (cfg.plasma_y * scale), cfg.plasma_z * scale));
                mech.AddPart(CreatePlasmaCannon(cfg, scale), weaponRot);
                Matrix weaponRot2 = MatrixMultiply(MatrixRotateX(90 * DEG2RAD), MatrixTranslate(xPos + (side * cfg.plasma_x2 * scale), yPos + (cfg.plasma_y * scale), cfg.plasma_z * scale));
                mech.AddPart(CreatePlasmaCannon(cfg, scale), weaponRot2);
            } else {
                // Rocket Pod
                Matrix podTransform = MatrixMultiply(MatrixRotateY(0), MatrixTranslate(xPos + (side * cfg.rocket_x * scale), yPos, cfg.rocket_z * scale));
                mech.AddPart(CreateRocketPod(cfg.rocket_w * scale, cfg.rocket_h * scale, cfg.rocket_d * scale), podTransform);
            }
        }
    }
    // Torso: Increased radius to 0.75f to look "heavy" and armored
    mech.AddPart(CreateSimpleCylinder(cfg.torso_r * scale, cfg.torso_h * scale, 6), MatrixTranslate(0, cfg.torso_y * scale, 0));
    
    // Neck and Head
    mech.AddPart(CreateSimpleCylinder(cfg.neck_r * scale, cfg.neck_h * scale, 8), MatrixTranslate(0, cfg.neck_y * scale, 0));
    mech.AddPart(CreateMechHead(cfg.head_w * scale, cfg.head_h * scale), MatrixTranslate(0, (cfg.neck_y + cfg.neck_h) * scale, cfg.head_z * scale));

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

    // Note: mech anchor removed—complex mesh merging doesn't render it properly
    // Consider adding as separate world entity instead if needed
    if (minY == FLT_MAX) minY = 0.0f;

    // Rebase so feet sit at y=0
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

static std::string SelectMechVariantPath(const std::string& variant) {
    std::string v = variant;
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (v == "alpha") return "assets/mech_alpha.lua";
    if (v == "bravo") return "assets/mech_bravo.lua";
    if (v == "charlie") return "assets/mech_charlie.lua";
    return "assets/mech_bravo.lua"; // bravo/default
}

Mesh CreateMechMesh(const std::string& variant) {
    MechConfig cfg = LoadMechConfig(SelectMechVariantPath(variant));
    ProceduralMech mech = AssembleMech(cfg);
    return MergeMechParts(mech);
}