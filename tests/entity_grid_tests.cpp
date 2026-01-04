#include <gtest/gtest.h>
#include "../src/grid.h"
#include "../src/entity.h"
#include <vector>
#include <cmath>

// ==================== GRID TESTS ====================

class GridTests : public ::testing::Test {
protected:
    Grid grid;
};

TEST_F(GridTests, GridSizeIsCorrect) {
    EXPECT_EQ(Grid::SIZE, 12);
}

TEST_F(GridTests, GridInitializesCellsToEmpty) {
    for (int x = 0; x < Grid::SIZE; ++x) {
        for (int y = 0; y < Grid::SIZE; ++y) {
            EXPECT_EQ(grid.getCell(x, y), 0);
        }
    }
}

TEST_F(GridTests, IsValidPositionTrueForValidCoords) {
    EXPECT_TRUE(grid.isValidPosition(0, 0));
    EXPECT_TRUE(grid.isValidPosition(6, 6));
    EXPECT_TRUE(grid.isValidPosition(11, 11));
    EXPECT_TRUE(grid.isValidPosition(5, 5));
}

TEST_F(GridTests, IsValidPositionFalseForNegativeCoords) {
    EXPECT_FALSE(grid.isValidPosition(-1, 0));
    EXPECT_FALSE(grid.isValidPosition(0, -1));
    EXPECT_FALSE(grid.isValidPosition(-5, -5));
}

TEST_F(GridTests, IsValidPositionFalseForOutOfBoundsCoords) {
    EXPECT_FALSE(grid.isValidPosition(12, 0));
    EXPECT_FALSE(grid.isValidPosition(0, 12));
    EXPECT_FALSE(grid.isValidPosition(12, 12));
    EXPECT_FALSE(grid.isValidPosition(20, 20));
}

TEST_F(GridTests, SetCellUpdatesValue) {
    grid.setCell(5, 5, 1);
    EXPECT_EQ(grid.getCell(5, 5), 1);
}

TEST_F(GridTests, SetCellMultipleLocations) {
    grid.setCell(0, 0, 1);
    grid.setCell(5, 5, 2);
    grid.setCell(11, 11, 3);
    
    EXPECT_EQ(grid.getCell(0, 0), 1);
    EXPECT_EQ(grid.getCell(5, 5), 2);
    EXPECT_EQ(grid.getCell(11, 11), 3);
}

TEST_F(GridTests, SetCellIgnoresInvalidPositions) {
    // Should not crash, silently ignore
    grid.setCell(-1, 0, 1);
    grid.setCell(12, 12, 1);
    grid.setCell(100, 100, 1);
    
    // Grid should remain unchanged
    EXPECT_EQ(grid.getCell(0, 0), 0);
}

TEST_F(GridTests, GetCellReturnsNegativeOneForInvalidPosition) {
    EXPECT_EQ(grid.getCell(-1, 0), -1);
    EXPECT_EQ(grid.getCell(12, 5), -1);
    EXPECT_EQ(grid.getCell(5, 12), -1);
}

TEST_F(GridTests, CellsAreIndependent) {
    grid.setCell(2, 3, 7);
    grid.setCell(3, 2, 9);
    
    EXPECT_EQ(grid.getCell(2, 3), 7);
    EXPECT_EQ(grid.getCell(3, 2), 9);
    EXPECT_NE(grid.getCell(2, 3), grid.getCell(3, 2));
}

TEST_F(GridTests, SetCellCanUpdateExistingValue) {
    grid.setCell(4, 4, 5);
    EXPECT_EQ(grid.getCell(4, 4), 5);
    
    grid.setCell(4, 4, 10);
    EXPECT_EQ(grid.getCell(4, 4), 10);
}

TEST_F(GridTests, GridBoundaryConditions) {
    // Test all four corners
    grid.setCell(0, 0, 1);
    grid.setCell(11, 0, 2);
    grid.setCell(0, 11, 3);
    grid.setCell(11, 11, 4);
    
    EXPECT_EQ(grid.getCell(0, 0), 1);
    EXPECT_EQ(grid.getCell(11, 0), 2);
    EXPECT_EQ(grid.getCell(0, 11), 3);
    EXPECT_EQ(grid.getCell(11, 11), 4);
}

// ==================== ENTITY TESTS ====================

class EntityTests : public ::testing::Test {
protected:
    Entity createEntity(int id, EntityType type, Vector2 position, std::string name) {
        Entity e;
        e.id = id;
        e.type = type;
        e.position = position;
        e.name = name;
        e.health = 100;
        return e;
    }
};

TEST_F(EntityTests, EntityConstructorDefaults) {
    Entity e;
    EXPECT_EQ(e.health, 100);
}

TEST_F(EntityTests, EntityTypeEnumValid) {
    EXPECT_EQ(PLAYER, 0);
    EXPECT_EQ(ENEMY, 1);
    EXPECT_EQ(OBJECT, 2);
}

TEST_F(EntityTests, CreatePlayerEntity) {
    Entity player = createEntity(1, PLAYER, {3.0f, 4.0f}, "Hero");
    
    EXPECT_EQ(player.id, 1);
    EXPECT_EQ(player.type, PLAYER);
    EXPECT_EQ(player.position.x, 3.0f);
    EXPECT_EQ(player.position.y, 4.0f);
    EXPECT_EQ(player.name, "Hero");
    EXPECT_EQ(player.health, 100);
}

TEST_F(EntityTests, CreateEnemyEntity) {
    Entity enemy = createEntity(2, ENEMY, {7.0f, 8.0f}, "Goblin");
    
    EXPECT_EQ(enemy.id, 2);
    EXPECT_EQ(enemy.type, ENEMY);
    EXPECT_EQ(enemy.position.x, 7.0f);
    EXPECT_EQ(enemy.position.y, 8.0f);
    EXPECT_EQ(enemy.name, "Goblin");
}

TEST_F(EntityTests, CreateObjectEntity) {
    Entity obj = createEntity(3, OBJECT, {5.0f, 5.0f}, "Barrel");
    
    EXPECT_EQ(obj.id, 3);
    EXPECT_EQ(obj.type, OBJECT);
    EXPECT_EQ(obj.name, "Barrel");
}

TEST_F(EntityTests, EntityPositionVector2) {
    Entity e = createEntity(1, PLAYER, {0.5f, 11.5f}, "Edge");
    
    EXPECT_FLOAT_EQ(e.position.x, 0.5f);
    EXPECT_FLOAT_EQ(e.position.y, 11.5f);
}

TEST_F(EntityTests, EntityHealthTrackable) {
    Entity e = createEntity(1, PLAYER, {5.0f, 5.0f}, "Hero");
    EXPECT_EQ(e.health, 100);
    
    e.health -= 25;
    EXPECT_EQ(e.health, 75);
    
    e.health -= 75;
    EXPECT_EQ(e.health, 0);
}

TEST_F(EntityTests, MultipleEntitiesIndependent) {
    Entity player = createEntity(1, PLAYER, {2.0f, 3.0f}, "Player");
    Entity enemy = createEntity(2, ENEMY, {9.0f, 10.0f}, "Enemy");
    
    player.health = 50;
    enemy.health = 30;
    
    EXPECT_EQ(player.health, 50);
    EXPECT_EQ(enemy.health, 30);
    EXPECT_NE(player.id, enemy.id);
}

// ==================== ENTITY-GRID INTEGRATION TESTS ====================

class EntityGridIntegrationTests : public ::testing::Test {
protected:
    Grid grid;
    std::vector<Entity> entities;
    
    Entity createEntity(int id, EntityType type, Vector2 pos, std::string name) {
        Entity e;
        e.id = id;
        e.type = type;
        e.position = pos;
        e.name = name;
        e.health = 100;
        return e;
    }
    
    bool isEntityAtGridPosition(const Entity& entity, int gridX, int gridY) {
        return static_cast<int>(entity.position.x) == gridX &&
               static_cast<int>(entity.position.y) == gridY;
    }
    
    bool canPlaceEntity(const Entity& entity, const std::vector<Entity>& otherEntities) {
        int gridX = static_cast<int>(entity.position.x);
        int gridY = static_cast<int>(entity.position.y);
        
        if (!grid.isValidPosition(gridX, gridY)) {
            return false;
        }
        
        for (const auto& other : otherEntities) {
            if (isEntityAtGridPosition(other, gridX, gridY)) {
                return false;
            }
        }
        
        return true;
    }
};

TEST_F(EntityGridIntegrationTests, EntityOnValidGridPosition) {
    Entity hero = createEntity(1, PLAYER, {5.0f, 6.0f}, "Hero");
    
    EXPECT_TRUE(grid.isValidPosition(5, 6));
    EXPECT_TRUE(isEntityAtGridPosition(hero, 5, 6));
}

TEST_F(EntityGridIntegrationTests, EntityOnInvalidGridPosition) {
    Entity hero = createEntity(1, PLAYER, {15.0f, 15.0f}, "Hero");
    
    EXPECT_FALSE(grid.isValidPosition(15, 15));
}

TEST_F(EntityGridIntegrationTests, PlaceEntityAtGridCorners) {
    Entity e1 = createEntity(1, PLAYER, {0.0f, 0.0f}, "TopLeft");
    Entity e2 = createEntity(2, ENEMY, {11.0f, 0.0f}, "TopRight");
    Entity e3 = createEntity(3, PLAYER, {0.0f, 11.0f}, "BottomLeft");
    Entity e4 = createEntity(4, ENEMY, {11.0f, 11.0f}, "BottomRight");
    
    entities = {e1, e2, e3, e4};
    
    for (const auto& e : entities) {
        EXPECT_TRUE(grid.isValidPosition(
            static_cast<int>(e.position.x),
            static_cast<int>(e.position.y)
        ));
    }
}

TEST_F(EntityGridIntegrationTests, PreventEntityPlacementOutOfBounds) {
    Entity hero = createEntity(1, PLAYER, {-1.0f, 5.0f}, "OutOfBounds");
    
    EXPECT_FALSE(canPlaceEntity(hero, entities));
}

TEST_F(EntityGridIntegrationTests, PreventEntityCollision) {
    Entity hero = createEntity(1, PLAYER, {5.0f, 5.0f}, "Hero");
    Entity enemy = createEntity(2, ENEMY, {5.0f, 5.0f}, "Enemy");
    
    entities.push_back(hero);
    
    EXPECT_FALSE(canPlaceEntity(enemy, entities));
}

TEST_F(EntityGridIntegrationTests, AllowEntityPlacementAtAdjacentPositions) {
    Entity hero = createEntity(1, PLAYER, {5.0f, 5.0f}, "Hero");
    Entity enemy = createEntity(2, ENEMY, {5.0f, 6.0f}, "Enemy");
    
    entities.push_back(hero);
    
    EXPECT_TRUE(canPlaceEntity(enemy, entities));
}

TEST_F(EntityGridIntegrationTests, MarkGridCellAsOccupiedByEntity) {
    Entity hero = createEntity(1, PLAYER, {3.0f, 4.0f}, "Hero");
    
    int gridX = static_cast<int>(hero.position.x);
    int gridY = static_cast<int>(hero.position.y);
    
    grid.setCell(gridX, gridY, hero.id);
    
    EXPECT_EQ(grid.getCell(gridX, gridY), hero.id);
}

TEST_F(EntityGridIntegrationTests, MultipleEntitiesOnGridWithoutCollision) {
    Entity p1 = createEntity(1, PLAYER, {2.0f, 2.0f}, "Player1");
    Entity p2 = createEntity(2, PLAYER, {2.0f, 4.0f}, "Player2");
    Entity e1 = createEntity(3, ENEMY, {9.0f, 9.0f}, "Enemy1");
    
    entities = {p1, p2, e1};
    
    for (const auto& e : entities) {
        EXPECT_TRUE(canPlaceEntity(e, {entities.begin(), entities.end() - 1}));
    }
}

TEST_F(EntityGridIntegrationTests, ClearGridCellWhenEntityMoves) {
    Entity hero = createEntity(1, PLAYER, {5.0f, 5.0f}, "Hero");
    
    // Mark initial position
    grid.setCell(5, 5, hero.id);
    EXPECT_EQ(grid.getCell(5, 5), 1);
    
    // Move hero
    hero.position = {6.0f, 6.0f};
    grid.setCell(5, 5, 0); // Clear old position
    grid.setCell(6, 6, hero.id); // Mark new position
    
    EXPECT_EQ(grid.getCell(5, 5), 0);
    EXPECT_EQ(grid.getCell(6, 6), 1);
}

TEST_F(EntityGridIntegrationTests, EntityDistanceCalculation) {
    Entity p1 = createEntity(1, PLAYER, {0.0f, 0.0f}, "Start");
    Entity p2 = createEntity(2, PLAYER, {3.0f, 4.0f}, "End");
    
    float dx = p2.position.x - p1.position.x;
    float dy = p2.position.y - p1.position.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    EXPECT_FLOAT_EQ(distance, 5.0f);
}

TEST_F(EntityGridIntegrationTests, ManhattanDistanceBetweenEntities) {
    Entity p1 = createEntity(1, PLAYER, {2.0f, 3.0f}, "Start");
    Entity p2 = createEntity(2, PLAYER, {5.0f, 7.0f}, "End");
    
    int manhattan = static_cast<int>(std::abs(p2.position.x - p1.position.x) +
                                      std::abs(p2.position.y - p1.position.y));
    
    EXPECT_EQ(manhattan, 7);
}

TEST_F(EntityGridIntegrationTests, FindNeighboringGridCells) {
    int centerX = 6, centerY = 6;
    std::vector<std::pair<int, int>> neighbors;
    
    // Cardinal directions
    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};
    
    for (int i = 0; i < 4; ++i) {
        int nx = centerX + dx[i];
        int ny = centerY + dy[i];
        if (grid.isValidPosition(nx, ny)) {
            neighbors.push_back({nx, ny});
        }
    }
    
    EXPECT_EQ(neighbors.size(), 4);
}

TEST_F(EntityGridIntegrationTests, FindNeighboringGridCellsAtEdge) {
    int centerX = 0, centerY = 0;
    std::vector<std::pair<int, int>> neighbors;
    
    // Cardinal directions
    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};
    
    for (int i = 0; i < 4; ++i) {
        int nx = centerX + dx[i];
        int ny = centerY + dy[i];
        if (grid.isValidPosition(nx, ny)) {
            neighbors.push_back({nx, ny});
        }
    }
    
    EXPECT_EQ(neighbors.size(), 2); // Only right and down are valid
}

TEST_F(EntityGridIntegrationTests, EntityCountInGame) {
    entities.push_back(createEntity(1, PLAYER, {1.0f, 1.0f}, "Hero"));
    entities.push_back(createEntity(2, ENEMY, {10.0f, 10.0f}, "Goblin"));
    entities.push_back(createEntity(3, OBJECT, {5.0f, 5.0f}, "Barrel"));
    
    EXPECT_EQ(entities.size(), 3);
    
    int playerCount = 0, enemyCount = 0, objectCount = 0;
    for (const auto& e : entities) {
        if (e.type == PLAYER) playerCount++;
        else if (e.type == ENEMY) enemyCount++;
        else if (e.type == OBJECT) objectCount++;
    }
    
    EXPECT_EQ(playerCount, 1);
    EXPECT_EQ(enemyCount, 1);
    EXPECT_EQ(objectCount, 1);
}
