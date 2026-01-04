#include "world.h"
#include "mesh.h"
#include "rlights.h" // For CreateLight
#include "raymath.h" // For Vector3Zero
#include "app.h"     // For AppContext shaders
#include <algorithm>

// Private helper to wrap mesh processing and model creation
static void AddEntity(World& world, Mesh mesh, Vector3 pos, Color tint, Shader shader) {
    // Upload geometry once per mesh instance
    if (mesh.vaoId == 0) {
        MeshUtils::unshareMeshVertices(&mesh);
        UploadMesh(&mesh, false);
    }

    WorldEntity ent;
    ent.model = LoadModelFromMesh(mesh);
    ent.model.materials[0].shader = shader;
    ent.position = pos;
    ent.scale = { 1.0f, 1.0f, 1.0f };
    ent.color = tint; // Ensure your WorldEntity struct has a Color field!

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

static float TileHeight(TileType type) {
    switch (type) {
    case TileType::Water: return -0.05f;
    case TileType::Mountain: return 0.6f;
    case TileType::Skyscraper: return 0.12f; // footprint pad; building added as prop
    default: return 0.0f;
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
            const float baseY = TileHeight(t) + 0.05f; // rest on top of tile slab
            Vector3 pos = { (x - World::kTilesWide * 0.5f + 0.5f) * World::kTileSize, baseY, (y - World::kTilesHigh * 0.5f + 0.5f) * World::kTileSize };

            switch (t) {
            case TileType::Forest:
                pos.y += 0.30f; // lift trees above slab
                AddEntity(world, treeMesh, pos, Color{30, 160, 80, 255}, appCtx.shaders.flat);
                break;
            case TileType::Mountain:
                pos.y += 0.50f; // taller mountain placement (150% tree height)
                AddEntity(world, mountainMesh, pos, Color{110, 96, 80, 255}, appCtx.shaders.flat);
                break;
            case TileType::Skyscraper:
                pos.y += 0.80f; // half the skyscraper height
                AddEntity(world, skyscraperMesh, pos, Color{140, 140, 150, 255}, appCtx.shaders.flat);
                break;
            default:
                break;
            }
        }
    }
}

static void PlaceActorsFromOccupants(World& world, const AppContext& appCtx) {
    auto idx = [](int x, int y) { return y * World::kTilesWide + x; };

    Mesh heroMesh = GenMeshCube(0.7f, 0.9f, 0.7f);
    Mesh enemyMesh = GenMeshCube(0.7f, 0.7f, 0.7f);

    for (int y = 0; y < World::kTilesHigh; ++y) {
        for (int x = 0; x < World::kTilesWide; ++x) {
            const Occupant occ = world.occupants[idx(x, y)];
            if (occ == Occupant::None) continue;

            float baseY = TileHeight(world.tiles[idx(x, y)]) + 0.05f; // top of tile slab
            Vector3 pos = { (x - World::kTilesWide * 0.5f + 0.5f) * World::kTileSize,
                            baseY + 0.45f,
                            (y - World::kTilesHigh * 0.5f + 0.5f) * World::kTileSize };

            if (occ == Occupant::Hero) {
                AddEntity(world, heroMesh, pos, Color{80, 200, 120, 255}, appCtx.shaders.flat);
            } else if (occ == Occupant::Enemy) {
                AddEntity(world, enemyMesh, pos, Color{200, 90, 90, 255}, appCtx.shaders.flat);
            }
        }
    }
}

void World_Init(World& world, const AppContext& appCtx) {
    world.entities.clear();

    BuildSampleLayout(world);

    // Place props based on tile types
    PlacePropsFromTiles(world, appCtx);

    // Place actors based on occupant map
    PlaceActorsFromOccupants(world, appCtx);

    // Lighting
    // Primary key light (directional)
    world.lights[0] = CreateLight(LIGHT_DIRECTIONAL, Vector3{-2.0f, 4.0f, -2.0f}, Vector3Zero(), Color{255, 240, 200, 255}, appCtx.shaders.flat);
    world.lightCount = 1;
    world.activeLight = 0;
}
void World_DrawGround(const World& world) {
    auto idx = [](int x, int y) { return y * World::kTilesWide + x; };

    constexpr float slabThickness = 0.80f;

    for (int y = 0; y < World::kTilesHigh; ++y) {
        for (int x = 0; x < World::kTilesWide; ++x) {
            const TileType t = world.tiles[idx(x, y)];
            const Color c = TileColor(t);
            float h = TileHeight(t);

            Vector3 size = { World::kTileSize, slabThickness, World::kTileSize };
            // Center the slab so its top is at h + slabThickness * 0.5f
            Vector3 pos = { (x - World::kTilesWide * 0.5f + 0.5f) * World::kTileSize,
                            h + slabThickness * 0.5f,
                            (y - World::kTilesHigh * 0.5f + 0.5f) * World::kTileSize };

            DrawCube(pos, size.x, size.y, size.z, c);

            // Outline
            DrawCubeWires(pos, size.x, size.y + 0.001f, size.z, Color{60, 60, 60, 255});
        }
    }
}