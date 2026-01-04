#include <gtest/gtest.h>
#include <vector>
#include "raylib.h"
#include "app.h"

// Mock the GetFactionFromColor function from render.cpp
// Since it's static, we'll test it indirectly via the palette detection logic

class RenderTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize any common test data
    }

    void TearDown() override {
        // Cleanup
    }
};

// Helper: Simulate faction detection logic
static int DetectFactionFromColor(Color c) {
    float r = c.r / 255.0f;
    float g = c.g / 255.0f;
    float b = c.b / 255.0f;
    
    // Find the dominant color channel
    float maxVal = fmaxf(fmaxf(r, g), b);
    
    // Only classify if the dominant channel is strong enough
    if (maxVal < 0.4f) return (int)FactionType::Neutral;
    
    // Red faction: red is dominant
    if (r == maxVal) return (int)FactionType::RedFaction;
    // Blue faction: blue is dominant
    if (b == maxVal) return (int)FactionType::BlueFaction;
    // Green faction: green is dominant
    if (g == maxVal) return (int)FactionType::GreenFaction;
    
    // Default to neutral
    return (int)FactionType::Neutral;
}

// Helper: Simulate palette selection that prefers enemies, then heroes, skipping props
struct TestEntity {
    Color color;
    bool isEnemy;
    bool isActor;
};

static int DetectPaletteIndex(const std::vector<TestEntity>& entities) {
    auto detect = [](const std::vector<TestEntity>& ents, bool enemiesOnly) {
        for (const auto& e : ents) {
            if (!e.isActor) continue;
            if (enemiesOnly && !e.isEnemy) continue;

            float r = e.color.r / 255.0f;
            float g = e.color.g / 255.0f;
            float b = e.color.b / 255.0f;

            float maxC = fmaxf(fmaxf(r, g), b);
            float minC = fminf(fminf(r, g), b);
            float saturation = (maxC - minC);

            if (saturation > 0.3f && maxC > 0.3f) {
                return DetectFactionFromColor(e.color);
            }
        }
        return (int)FactionType::Neutral;
    };

    int idx = detect(entities, /*enemiesOnly=*/true);
    if (idx == (int)FactionType::Neutral) {
        idx = detect(entities, /*enemiesOnly=*/false);
    }
    return idx;
}

// Test: Red enemy mech detection
TEST(RenderTests, DetectRedFactionMech) {
    Color redEnemy = {200, 90, 90, 255};  // Enemy color from world.cpp
    int faction = DetectFactionFromColor(redEnemy);
    EXPECT_EQ(faction, (int)FactionType::RedFaction);
}

// Test: Light uniform packing (vec3 -> float[3])
TEST(RenderTests, LightUniformUpdate) {
    Vector3 lightPos = {3.5f, 2.1f, -1.2f};
    float lightArray[3] = {lightPos.x, lightPos.y, lightPos.z};

    EXPECT_FLOAT_EQ(lightArray[0], 3.5f);
    EXPECT_FLOAT_EQ(lightArray[1], 2.1f);
    EXPECT_FLOAT_EQ(lightArray[2], -1.2f);
}

// Test: Green hero mech detection
TEST(RenderTests, DetectGreenFactionMech) {
    Color greenHero = {80, 200, 120, 255};  // Hero color from world.cpp
    int faction = DetectFactionFromColor(greenHero);
    EXPECT_EQ(faction, (int)FactionType::GreenFaction);
}

// Test: Blue faction detection
TEST(RenderTests, DetectBlueFactionMech) {
    Color blueMech = {50, 100, 200, 255};
    int faction = DetectFactionFromColor(blueMech);
    EXPECT_EQ(faction, (int)FactionType::BlueFaction);
}

// Test: Neutral (gray) detection
TEST(RenderTests, DetectNeutralGrayMech) {
    Color grayMech = {100, 100, 100, 255};
    int faction = DetectFactionFromColor(grayMech);
    EXPECT_EQ(faction, (int)FactionType::Neutral);
}

// Test: Near-threshold brightness should stay neutral
TEST(RenderTests, NearThresholdBrightnessNeutral) {
    Color dim = {90, 90, 90, 255}; // max channel ~0.35 < 0.4 threshold
    int faction = DetectFactionFromColor(dim);
    EXPECT_EQ(faction, (int)FactionType::Neutral);
}

// Test: Tie on dominant channels should pick first branch (red when r==g==max)
TEST(RenderTests, TieBreakChoosesRedWhenRedEqualsMax) {
    Color tieRedGreen = {180, 180, 90, 255}; // r == g > b
    int faction = DetectFactionFromColor(tieRedGreen);
    EXPECT_EQ(faction, (int)FactionType::RedFaction);
}

// Test: Palette detection skips props and prefers enemy over hero
TEST(RenderTests, PalettePrefersEnemyOverHero) {
    std::vector<TestEntity> ents = {
        { Color{30, 160, 80, 255}, false, false }, // tree (prop)
        { Color{80, 200, 120, 255}, false, true }, // hero (green)
        { Color{200, 90, 90, 255}, true, true }    // enemy (red)
    };

    int paletteIdx = DetectPaletteIndex(ents);
    EXPECT_EQ(paletteIdx, (int)FactionType::RedFaction);
}

// Test: Palette falls back to hero when no enemies are present
TEST(RenderTests, PaletteFallsBackToHeroWhenNoEnemy) {
    std::vector<TestEntity> ents = {
        { Color{140, 140, 150, 255}, false, false }, // prop
        { Color{80, 200, 120, 255}, false, true }    // hero
    };

    int paletteIdx = DetectPaletteIndex(ents);
    EXPECT_EQ(paletteIdx, (int)FactionType::GreenFaction);
}

// Test: Dark color defaults to neutral
TEST(RenderTests, DarkColorNeutral) {
    Color darkColor = {30, 20, 25, 255};
    int faction = DetectFactionFromColor(darkColor);
    EXPECT_EQ(faction, (int)FactionType::Neutral);
}

// Test: Saturation detection (red with high saturation)
TEST(RenderTests, SaturationCalculation) {
    Color redSaturated = {200, 90, 90, 255};
    float r = redSaturated.r / 255.0f;
    float g = redSaturated.g / 255.0f;
    float b = redSaturated.b / 255.0f;
    
    float maxC = fmaxf(fmaxf(r, g), b);
    float minC = fminf(fminf(r, g), b);
    float saturation = (maxC - minC);
    
    EXPECT_GT(saturation, 0.3f);  // Should be well-saturated
    EXPECT_GT(maxC, 0.3f);        // Should be reasonably bright
}

// Test: Low saturation gray
TEST(RenderTests, LowSaturationGray) {
    Color gray = {128, 128, 128, 255};
    float r = gray.r / 255.0f;
    float g = gray.g / 255.0f;
    float b = gray.b / 255.0f;
    
    float maxC = fmaxf(fmaxf(r, g), b);
    float minC = fminf(fminf(r, g), b);
    float saturation = (maxC - minC);
    
    EXPECT_LT(saturation, 0.1f);  // Should be low saturation
}

// Test: Palette array sizes
TEST(RenderTests, PaletteStructures) {
    // Verify palette structures are properly defined
    EXPECT_EQ(sizeof(PALETTE_RED.highlight), sizeof(Vector3));
    EXPECT_EQ(sizeof(PALETTE_RED.base), sizeof(Vector3));
    EXPECT_EQ(sizeof(PALETTE_RED.shade), sizeof(Vector3));
    EXPECT_EQ(sizeof(PALETTE_RED.deepShadow), sizeof(Vector3));
}

// Test: Red palette values in valid range
TEST(RenderTests, RedPaletteValuesInRange) {
    EXPECT_GE(PALETTE_RED.highlight.x, 0.0f);
    EXPECT_LE(PALETTE_RED.highlight.x, 1.0f);
    EXPECT_GE(PALETTE_RED.base.x, 0.0f);
    EXPECT_LE(PALETTE_RED.base.x, 1.0f);
}

// Test: Blue palette values in valid range
TEST(RenderTests, BluePaletteValuesInRange) {
    EXPECT_GE(PALETTE_BLUE.highlight.z, 0.0f);
    EXPECT_LE(PALETTE_BLUE.highlight.z, 1.0f);
}

// Test: Green palette values in valid range
TEST(RenderTests, GreenPaletteValuesInRange) {
    EXPECT_GE(PALETTE_GREEN.base.y, 0.0f);
    EXPECT_LE(PALETTE_GREEN.base.y, 1.0f);
}

// Test: Neutral palette is grayscale
TEST(RenderTests, NeutralPaletteGrayscale) {
    // All channels should be equal for neutral (grayscale)
    EXPECT_FLOAT_EQ(PALETTE_NEUTRAL.highlight.x, PALETTE_NEUTRAL.highlight.y);
    EXPECT_FLOAT_EQ(PALETTE_NEUTRAL.highlight.y, PALETTE_NEUTRAL.highlight.z);
    
    EXPECT_FLOAT_EQ(PALETTE_NEUTRAL.base.x, PALETTE_NEUTRAL.base.y);
    EXPECT_FLOAT_EQ(PALETTE_NEUTRAL.base.y, PALETTE_NEUTRAL.base.z);
}

// Test: Palette darkening order (highlight > base > shade > deepShadow)
TEST(RenderTests, RedPaletteDarkeningOrder) {
    float highlightBright = PALETTE_RED.highlight.x;
    float baseBright = PALETTE_RED.base.x;
    float shadeBright = PALETTE_RED.shade.x;
    float deepBright = PALETTE_RED.deepShadow.x;
    
    EXPECT_GT(highlightBright, baseBright);
    EXPECT_GT(baseBright, shadeBright);
    EXPECT_GT(shadeBright, deepBright);
}

// Test: Faction enum values
TEST(RenderTests, FactionEnumValues) {
    EXPECT_EQ((int)FactionType::RedFaction, 0);
    EXPECT_EQ((int)FactionType::BlueFaction, 1);
    EXPECT_EQ((int)FactionType::GreenFaction, 2);
    EXPECT_EQ((int)FactionType::Neutral, 3);
}

// Test: Dominant channel detection
TEST(RenderTests, DominantChannelDetection) {
    Color red = {255, 50, 50, 255};
    Color blue = {50, 50, 255, 255};
    Color green = {50, 255, 50, 255};
    
    EXPECT_EQ(DetectFactionFromColor(red), (int)FactionType::RedFaction);
    EXPECT_EQ(DetectFactionFromColor(blue), (int)FactionType::BlueFaction);
    EXPECT_EQ(DetectFactionFromColor(green), (int)FactionType::GreenFaction);
}
