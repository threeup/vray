#include "world.h"
#include "mesh.h"
#include "utils/meshMech.h"
#include "rlights.h" // For CreateLight
#include "raymath.h" // For Vector3Zero
#include "app.h"     // For AppContext shaders
#include <algorithm>

// Private helper to wrap mesh processing and model creation
static void AddEntity(World& world, Mesh mesh, Vector3 pos, Color tint, Shader shader, bool isActor) {
    // Upload geometry once per mesh instance
    if (mesh.vaoId == 0) {
        MeshUtils::unshareMeshVertices(&mesh);
        UploadMesh(&mesh, false);
    }

    WorldEntity ent;
    ent.model = LoadModelFromMesh(mesh);
    ent.model.materials[0].shader = shader;
    ent.position = pos;
    ent.startPos = pos;
    ent.targetPos = pos;  // Start at current position
    ent.scale = { 1.0f, 1.0f, 1.0f };
    ent.color = tint;
    ent.id = static_cast<int>(world.entities.size());  // Simple ID assignment
    ent.moveProgress = 0.0f;
    ent.isActor = isActor;
    ent.isEnemy = false;

    world.entities.push_back(ent);
}

static Color TileColor(TileType type) {
    switch (type) {
    case TileType::Dirt: return Color{181, 140, 99, 255};
    case TileType::Forest: return Color{82, 120, 68, 255};
    case TileType::Skyscraper: return Color{120, 120, 130, 255};
    case TileType::Water: return Color{60, 120, 180, 255};
    case TileType::Mountain: return Color{110, 96, 80, 255};
    case TileType::SpawnHero: return Color{90, 170, 90, 255};
    case TileType::SpawnEnemy: return Color{170, 90, 90, 255};
    default: return LIGHTGRAY;
    }
}

static void BuildSampleLayout(World& world) {
    auto idx = [](int x, int y) { return y * World::kTilesWide + x; };

    std::fill(world.tiles.begin(), world.tiles.end(), TileType::Dirt);
    std::fill(world.occupants.begin(), world.occupants.end(), Occupant::None);

    // Alternating dirt/forest baseline
    for (int y = 0; y < World::kTilesHigh; ++y) {
        for (int x = 0; x < World::kTilesWide; ++x) {
            if ((x + y) % 2 == 1) {
                world.tiles[idx(x, y)] = TileType::Forest;
            }
        }
    }

    // Skyscraper footprints (5 tiles)
    const int skyscrapers[][2] = { {2,1}, {3,1}, {4,1}, {3,2}, {4,2} };
    for (auto& p : skyscrapers) {
        world.tiles[idx(p[0], p[1])] = TileType::Skyscraper;
    }

    // Water tiles (a few)
    const int water[][2] = { {0,3}, {1,3}, {0,4} };
    for (auto& p : water) {
        world.tiles[idx(p[0], p[1])] = TileType::Water;
    }

    // Mountain tiles (a few)
    const int mountains[][2] = { {5,2}, {6,2} };
    for (auto& p : mountains) {
        world.tiles[idx(p[0], p[1])] = TileType::Mountain;
    }

    // Hero spawns (3)
    const int heroes[][2] = { {1,6}, {2,6}, {1,5} };
    for (auto& p : heroes) {
        world.tiles[idx(p[0], p[1])] = TileType::SpawnHero;
        world.occupants[idx(p[0], p[1])] = Occupant::Hero;
    }

    // Enemy spawns (3)
    const int enemies[][2] = { {6,6}, {5,6}, {6,5} };
    for (auto& p : enemies) {
        world.tiles[idx(p[0], p[1])] = TileType::SpawnEnemy;
        world.occupants[idx(p[0], p[1])] = Occupant::Enemy;
    }
}

static void PlacePropsFromTiles(World& world, const AppContext& appCtx) {
    auto idx = [](int x, int y) { return y * World::kTilesWide + x; };

    // Reuse simple generated meshes for props
    Mesh treeMesh = MeshGenerator::createSquareTree(0.6f, 1, 1); // shorter trees to avoid blocking view
    Mesh mountainMesh = MeshGenerator::createCraggyMountain(0.8f, 1.5f, 8); // Craggy 3-ring mountain with validation
    Mesh skyscraperMesh = GenMeshCube(0.9f, 1.6f, 0.9f);

    for (int y = 0; y < World::kTilesHigh; ++y) {
        for (int x = 0; x < World::kTilesWide; ++x) {
            const TileType t = world.tiles[idx(x, y)];
            const float baseY = TileSurfaceHeight(t); // align to slab top
            Vector3 pos = { (x - World::kTilesWide * 0.5f + 0.5f) * World::kTileSize, baseY, (y - World::kTilesHigh * 0.5f + 0.5f) * World::kTileSize };

            switch (t) {
            case TileType::Forest:
                pos.y += 0.30f; // lift trees above slab
                AddEntity(world, treeMesh, pos, Color{30, 160, 80, 255}, appCtx.shaders.flat, false);
                break;
            case TileType::Mountain:
                pos.y += 0.50f; // taller mountain placement (150% tree height)
                AddEntity(world, mountainMesh, pos, Color{110, 96, 80, 255}, appCtx.shaders.flat, false);
                break;
            case TileType::Skyscraper:
                pos.y += 0.80f; // half the skyscraper height
                AddEntity(world, skyscraperMesh, pos, Color{140, 140, 150, 255}, appCtx.shaders.flat, false);
                break;
            default:
                break;
            }
        }
    }
}

static void PlaceActorsFromOccupants(World& world, const AppContext& appCtx) {
    auto idx = [](int x, int y) { return y * World::kTilesWide + x; };
    auto tileToWorldPos = [](int tx, int ty) -> Vector3 {
        return { (tx - World::kTilesWide * 0.5f + 0.5f) * World::kTileSize,
                 0.0f,  // Height will be set per tile
                 (ty - World::kTilesHigh * 0.5f + 0.5f) * World::kTileSize };
    };

    auto tileToEntityPos = [&](int tx, int ty) -> Vector3 {
        Vector3 p = tileToWorldPos(tx, ty);
        p.y = ActorBaseHeight(world.tiles[idx(tx, ty)]); // align mech feet to slab top with tiny clearance
        return p;
    };
    
    static Mesh mechVariants[3] = { 0 };
    static bool mechInit[3] = { false, false, false };
    const char* variantNames[3] = { "alpha", "bravo", "charlie" };

    auto getVariantMesh = [&](int variantIdx) -> Mesh {
        if (variantIdx < 0 || variantIdx >= 3) variantIdx = 1; // default to bravo
        if (!mechInit[variantIdx]) {
            mechVariants[variantIdx] = CreateMechMesh(variantNames[variantIdx]);
            mechInit[variantIdx] = true;
        }
        return mechVariants[variantIdx];
    };

    int heroCount = 0;
    int enemyCount = 0;

    for (int y = 0; y < World::kTilesHigh; ++y) {
        for (int x = 0; x < World::kTilesWide; ++x) {
            const Occupant occ = world.occupants[idx(x, y)];
            if (occ == Occupant::None) continue;

            Vector3 pos = tileToEntityPos(x, y);

            if (occ == Occupant::Hero) {
                int variantIdx = heroCount % 3; // alpha, bravo, charlie
                AddEntity(world, getVariantMesh(variantIdx), pos, Color{80, 200, 120, 255}, appCtx.shaders.flat, true);
                heroCount++;
            } else if (occ == Occupant::Enemy) {
                int variantIdx = enemyCount % 3; // alpha, bravo, charlie
                AddEntity(world, getVariantMesh(variantIdx), pos, Color{200, 90, 90, 255}, appCtx.shaders.flat, true);
                enemyCount++;

                WorldEntity& enemy = world.entities.back();
                enemy.isEnemy = true;
                enemy.startPos = pos;
                enemy.targetPos = pos; // No patrol; stays put unless commanded by turn system
                enemy.patrolPoints = {pos, pos, pos, pos};
                enemy.patrolIndex = 0;
            }
        }
    }
}

// Place four bright anchor tetrahedrons just outside each board corner for visibility
static void PlaceCornerAnchors(World& world, const AppContext& appCtx) {
    static Mesh anchorMesh = { 0 };
    if (anchorMesh.vertexCount == 0) {
        anchorMesh = MeshGenerator::createCustomTetrahedron(0.30f, 0); // larger than mech anchor
    }

    auto tileToWorldPos = [](int tx, int ty) -> Vector3 {
        return { (tx - World::kTilesWide * 0.5f + 0.5f) * World::kTileSize,
                 0.0f,
                 (ty - World::kTilesHigh * 0.5f + 0.5f) * World::kTileSize };
    };

    const float baseY = ActorBaseHeight(TileType::Dirt);
    const Color anchorColor{255, 60, 200, 255};

    const int corners[4][2] = {
        { -1, -1 },
        { World::kTilesWide, -1 },
        { -1, World::kTilesHigh },
        { World::kTilesWide, World::kTilesHigh }
    };

    for (auto& c : corners) {
        Vector3 pos = tileToWorldPos(c[0], c[1]);
        pos.y = baseY;
        AddEntity(world, anchorMesh, pos, anchorColor, appCtx.shaders.flat, false);
    }
}

static void AdvanceTurn(World& world) {
    for (auto& entity : world.entities) {
        entity.startPos = entity.targetPos; // assume it reached its target at turn boundary
        entity.moveProgress = 0.0f;
        entity.targetPos = entity.startPos; // No automatic patrol progression
    }

    world.currentTurn++;
}

void World_Init(World& world, const AppContext& appCtx) {
    world.entities.clear();

    BuildSampleLayout(world);

    // Place props based on tile types
    PlacePropsFromTiles(world, appCtx);

    // Place actors based on occupant map
    PlaceActorsFromOccupants(world, appCtx);

    // Corner anchors past the board extents for an obvious ground reference
    PlaceCornerAnchors(world, appCtx);

    // Lighting
    // Primary key light (directional)
    world.lights[0] = CreateLight(LIGHT_DIRECTIONAL, Vector3{-2.0f, 4.0f, -2.0f}, Vector3Zero(), Color{255, 240, 200, 255}, appCtx.shaders.flat);
    world.lightCount = 1;
    world.activeLight = 0;
}

void World_Update(World& world, float elapsedTime) {
    // Cycle light position in a 10-second loop
    const float cyclePeriod = 10.0f;
    float cycleT = fmodf(elapsedTime, cyclePeriod) / cyclePeriod;  // [0, 1)
    
    Vector3 lightPos;
    
    if (cycleT < 0.25f) {
        // 0-2.5s: Interpolate from noon (0) to east morning (0.25)
        float t = cycleT / 0.25f;  // [0, 1)
        Vector3 noon = {-2.0f, 4.0f, -2.0f};
        Vector3 eastMorning = {4.0f, 2.0f, -2.0f};
        lightPos = Vector3Lerp(noon, eastMorning, t);
    } else if (cycleT < 0.5f) {
        // 2.5-5s: Interpolate from east morning (0.25) to south morning (0.5)
        float t = (cycleT - 0.25f) / 0.25f;  // [0, 1)
        Vector3 eastMorning = {4.0f, 2.0f, -2.0f};
        Vector3 southMorning = {-2.0f, 2.0f, 4.0f};
        lightPos = Vector3Lerp(eastMorning, southMorning, t);
    } else if (cycleT < 0.75f) {
        // 5-7.5s: Interpolate from south morning (0.5) to late evening (0.75)
        float t = (cycleT - 0.5f) / 0.25f;  // [0, 1)
        Vector3 southMorning = {-2.0f, 2.0f, 4.0f};
        Vector3 lateEvening = {-4.0f, 1.5f, -4.0f};
        lightPos = Vector3Lerp(southMorning, lateEvening, t);
    } else {
        // 7.5-10s: Interpolate from late evening (0.75) back to noon (1.0)
        float t = (cycleT - 0.75f) / 0.25f;  // [0, 1)
        Vector3 lateEvening = {-4.0f, 1.5f, -4.0f};
        Vector3 noon = {-2.0f, 4.0f, -2.0f};
        lightPos = Vector3Lerp(lateEvening, noon, t);
    }
    
    world.lights[world.activeLight].position = lightPos;
    
    // ==================== TURN SYSTEM ====================
    // Turn progression is driven externally; keep entities fixed unless the turn system updates targetPos.
    world.turnElapsedTime = 0.0f;
    for (auto& entity : world.entities) {
        entity.moveProgress = 0.0f;
        entity.position = entity.targetPos;
    }
}
void World_DrawGround(const World& world, const AppContext& appCtx) {
    auto idx = [](int x, int y) { return y * World::kTilesWide + x; };

    // Build a reusable tile model once to ensure the flat shader is applied (matte, no specular)
    static Model tileModel = { 0 };
    static bool tileModelInit = false;
    if (!tileModelInit) {
        Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);
        tileModel = LoadModelFromMesh(cube);
        tileModel.materials[0].shader = appCtx.shaders.flat;
        tileModelInit = true;
    }

    for (int y = 0; y < World::kTilesHigh; ++y) {
        for (int x = 0; x < World::kTilesWide; ++x) {
            const TileType t = world.tiles[idx(x, y)];
            const Color c = TileColor(t);
            float h = TileBaseHeight(t);

            Vector3 size = { World::kTileSize, kTileSlabThickness, World::kTileSize };
            // Center the slab so its top is at h + kTileSlabThickness * 0.5f
            Vector3 pos = { (x - World::kTilesWide * 0.5f + 0.5f) * World::kTileSize,
                            h + kTileSlabThickness * 0.5f,
                            (y - World::kTilesHigh * 0.5f + 0.5f) * World::kTileSize };

            // Draw using the flat shader (matte) with non-uniform scale
            DrawModelEx(tileModel, pos, Vector3{0, 1, 0}, 0.0f, size, c);

            // Outline removed to avoid bright edges when using default wireframe shader
        }
    }
}