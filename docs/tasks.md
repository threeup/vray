# VRay Development Roadmap

**Date:** 2026-01-05 | **Target:** 60 FPS / Production Stability

---

## Legend

| Symbol | Meaning                         |
| ------ | ------------------------------- |
| 🔴     | **Immediate** – do next         |
| 🟡     | **Soon** – following batch      |
| 🔵     | **Backlog** – later priority    |
| ❄️     | **Icebox** – research/long-term |

**Effort:** XS (0.5d) | S (1-2d) | M (3-5d) | L (>5d)

---

## 🔴 Immediate (Stability & Core Infrastructure)

### T_070: Graphics Handles RAII

**Effort:** S | **Status:** ✅ Implemented

Wrap `Shader` and `Model` handles in move-only RAII structs to ensure automatic cleanup on destruction.

```cpp
struct ShaderHandle {
    Shader shader{0};
    ShaderHandle() = default;
    explicit ShaderHandle(Shader s) : shader(s) {}
    ~ShaderHandle() { if (shader.id != 0) UnloadShader(shader); }

    ShaderHandle(ShaderHandle&& o) noexcept : shader(o.shader) { o.shader.id = 0; }
    ShaderHandle& operator=(ShaderHandle&& o) noexcept {
        if (this != &o) {
            if (shader.id != 0) UnloadShader(shader);
            shader = o.shader; o.shader.id = 0;
        }
        return *this;
    }
    bool valid() const { return shader.id != 0; }
};
```

**SubTasks:**

- Create `src/utils/raii_handles.h`
- Refactor `RenderShaders` and `RenderModels` in `src/app.h`
- Add unit tests for scope cleanup and move semantics

**DoD:** No raw `Shader`/`Model` ownership in app context; cleanup verified in tests.

### T_072: Game State Persistence

**Effort:** M | **Status:** Planned

Implement JSON round-trip for entities, plans, and game phase.

```cpp
struct GameSaveData {
    std::vector<EntityData> entities;
    int turnCount;
    std::string currentPhase;
};

bool SaveGame(const Game& game, const std::string& filePath);
bool LoadGame(Game& game, const std::string& filePath);
```

**SubTasks:**

- Create `src/persistence.h` and `.cpp`
- Integrate Save/Load buttons into UI/Menu
- Add JSON serialization for all relevant game state

**DoD:** Save persists between sessions; regression tests verify state equality after round-trip.

### T_075: RenderBackend Interface

**Effort:** M | **Status:** ✅ Implemented

Abstract Raylib calls behind a `RenderBackend` interface to enable headless testing.

```cpp
class RenderBackend {
public:
    virtual ~RenderBackend() = default;
    virtual void drawModel(Model model, Vector3 pos, float scale, Color tint) = 0;
    virtual void beginMode3D(Camera3D camera) = 0;
    virtual void endMode3D() = 0;
    // ... other render operations
};
```

**SubTasks:**

- Define `src/platform/interface/render_backend.h` (abstract)
- Implement `RaylibRenderBackend` (concrete) and `MockRenderBackend` (test)
- Refactor `render.cpp` and `Render_DrawFrame()` to use the interface

**DoD:** Smoke tests run without GPU; runtime path unchanged.

### T_071: CI Pipeline

**Effort:** S | **Status:** ✅ Implemented

GitHub Actions workflow for Windows build and automated testing.

```yaml
name: CI
on: [push, pull_request]
jobs:
  build-and-test:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4
      - name: Configure
        run: cmake -B build -G "Visual Studio 17 2022"
      - name: Build
        run: cmake --build build --config Release
      - name: Test
        run: ctest --test-dir build -C Release --output-on-failure
```

**SubTasks:**

- Create `.github/workflows/ci.yml`
- Add status badge to README
- Ensure all tests pass on every push

**DoD:** CI status badge green; all branches gated on passing build and tests.

### T_009: Crash-safe Main Loop

**Effort:** M | **Status:** Planned

Wrap main loop in try/catch to detect and gracefully handle asset/device failures.

**SubTasks:**

- Implement error modal display on critical failures
- Add graceful shutdown with log capture
- Test missing asset and device failure scenarios

**DoD:** Error modal displays cleanly; app exits safely with logs captured.

---

## 🟡 Soon (UX & Polish)

### T_060: Play Button Animations

**Effort:** S | **Status:** Planned

Add subtle animations to "PLAY TURN" button for visual feedback.

```cpp
struct UIAnimationState {
    float pulseTime = 0.0f;      // accumulates for pulse oscillation
    float pressTime = 0.0f;      // countdown for press animation
    bool isPulseEnabled = false; // only pulse when button enabled
};
```

**Animations:**

- **Pulse:** `scale = 1.0 + 0.02 * sin(pulseTime * 2π)` (smooth breathing effect)
- **Press:** Scale to 0.95 for 0.1s on click

**DoD:** Animations smooth and non-distracting; button remains clickable.

### T_061: Mirror Toggle Animation

**Effort:** S | **Status:** Planned

Animate mirror button toggle with horizontal flip and color flash.

```cpp
struct MirrorAnimationState {
    int animatingMechId = -1;  // which slot is animating
    float flipTime = 0.0f;     // 0.0 = idle, >0 = animating
    float flashTime = 0.0f;    // color flash overlay time
};
```

**Animations:**

- **Flip:** `scaleX = 1.0 - 2.0 * min(flipTime / 0.2, 1.0)` (goes 1→-1→1 over 0.2s)
- **Flash:** Yellow highlight fading to normal over 0.15s

**DoD:** Animation visually indicates state change; no disruption to card placement.

### T_062: Drop Feedback Animations

**Effort:** M | **Status:** Planned

Provide visual feedback on card drop: green flash on success, bounce-back on rejection.

```cpp
struct DragState {
    Vector2 cardOriginPos = {0, 0};  // starting position for bounce-back
    float dropFeedbackTime = 0.0f;   // animation duration
    bool dropWasValid = false;       // last drop succeeded?
};
```

**Animations:**

- **Valid Drop:** Highlight target slot green (0.15s fade)
- **Invalid Drop:** Ease-out cubic bounce-back: `newPos = mix(cardOriginPos, currentPos, easeOutCubic(1 - t/0.2))`
- **Easing:** `easeOutCubic(t) = 1 - (1-t)³`

**DoD:** Players clearly see drop outcome; bounce-back smooth and satisfying.

### T_073: Onboarding & Help Overlay

**Effort:** M | **Status:** Planned

First-run tutorial and in-game help screen.

```cpp
struct HelpOverlay {
    bool visible = false;
    int currentPage = 0;
    void draw(int winW, int winH);
    void handleInput();
};
```

**Pages:**

1. **Controls** – WASD camera, drag-drop cards, confirm/cancel keys
2. **Gameplay** – Phase flow, mech assignment, mirror toggle mechanics
3. **Tips** – Card synergies, turn planning strategies

**SubTasks:**

- Check for `first_run.flag` file; show tutorial if missing
- Implement page navigation with next/prev buttons
- Add F1/H key binding for help access

**DoD:** New users guided through first turn; help accessible in-game.

### T_066: Layout System Tests

**Effort:** S | **Status:** Planned

Parameterized GTest for `GameUIPanel` layout validation across resolutions.

```cpp
TEST_P(GameUIPanelLayoutTest, ComputesValidLayout) {
    GameUIPanel panel;
    panel.metrics = PanelMetrics{10.0f, 12.0f, 4.0f};
    panel.computeLayout(GetParam().width, GetParam().height);

    // Verify no overlaps
    EXPECT_FALSE(CheckCollisionRecs(panel.phaseRect, panel.deckRect));

    // Verify within bounds
    EXPECT_GE(panel.phaseRect.y, panel.metrics.margin);
}
```

**Test Resolutions:**

- 800×600, 1024×768, 1280×720, 1920×1080, 2560×1440

**Assertions:**

- No negative dimensions; no overlapping regions
- All regions fit within window bounds
- Phase bar at top; hand bar at bottom; game board centered

**DoD:** All layout tests passing for specified resolutions.

### T_076: Structured Logging

**Effort:** S | **Status:** Planned

Implement log levels (DEBUG, INFO, WARN, ERROR) with size-based rotation.

```cpp
enum class LogLevel { Debug, Info, Warn, Error };
void LogMessage(LogLevel level, const char* format, ...);
void SetLogFile(const std::string& path, size_t maxSize = 10*1024*1024);
```

**SubTasks:**

- Create `src/utils/logger.h`
- Replace `TraceLog()` calls with `LogMessage()`
- Implement 10MB rotation cap
- Control level via `vars.lua` config

**DoD:** Logs rotate at 10MB; output filtered by configured level.

### T_074: Input Remapping

**Effort:** M | **Status:** Planned

Configurable key bindings persisted in config.

```cpp
struct InputBindings {
    int cameraForward = KEY_W;
    int cameraBack = KEY_S;
    int cameraLeft = KEY_A;
    int cameraRight = KEY_D;
    int confirm = KEY_ENTER;
    int cancel = KEY_ESCAPE;
    int help = KEY_F1;
};
```

**Lua Config:**

```lua
bindings = {
    camera_forward = "W",
    camera_back = "S",
    -- ...
}
```

**SubTasks:**

- Add bindings struct to AppConfig
- Implement conflict detection in UI
- Persist bindings between sessions

**DoD:** UI panel for remapping works; bindings persist and validate.

---

## 🔵 Backlog (Content & Optimization)

### T_063: Drag Preview Stat Delta

**Effort:** M | **Status:** Planned

Show real-time stat delta preview when dragging card over mech slot.

```cpp
struct DragStateDelta {
    int deltaHealth = 0;
    int deltaAttack = 0;
    int deltaDefense = 0;
};
```

**Implementation:**

- Call `calculateMechStats()` with hypothetical assignment during drag hover
- Display "+5 ATK" (green) or "-3 HP" (red) at slot center
- Fade out when dragging outside any slot

**DoD:** Preview calculates correctly; appears/disappears smoothly with drag.

### T_064: Text Styling Polish

**Effort:** XS | **Status:** Planned

Improve readability of tooltips with visual differentiation for mirrored effects.

**Changes:**

- **Mirrored Effects:** Use italic font or slant transform (scale x = 0.8) + lighter gray color
- **Card Name:** Draw with bold/larger font (size 16 vs 14)

**DoD:** Mirrored effects visually distinct; readability improved.

### T_065: Visual Depth

**Effort:** S | **Status:** Planned

Add stacked rectangle visual depth effect to card slots when occupied.

**Implementation:**

- Draw 3 offset rectangles (2-3px staggered) behind main card rect
- Progressive darkening: (80,80,80) → (60,60,60) → (40,40,40)
- Layers at (0,0), (2,2), (4,4) offsets

**DoD:** Depth effect visible with occupied cards; visual polish without gameplay impact.

### Infrastructure & Rendering

| Task                               | Effort | Description                                                   |
| ---------------------------------- | ------ | ------------------------------------------------------------- |
| **T_071: CI Pipeline**             | S      | GitHub Actions workflow; cmake build + ctest on push/PR       |
| **T_021: Performance Hooks**       | M      | Lightweight timers, frame budget reporting, frustum culling   |
| **T_074: Input Remapping**         | M      | Configurable bindings with conflict detection                 |
| **T_031: Procedural Mesh Cleanup** | M      | Consolidate tree/pyramid/cube generators into reusable module |
| **T_075: RenderBackend Interface** | M      | Abstract render behind interface for headless testing         |
| **T_076: Structured Logging**      | S      | Log levels (DEBUG/INFO/WARN/ERROR) with 10MB rotation         |
| **T_032: Instancing**              | M      | Implement InstancedRenderer to reduce draw calls              |
| **T_030: Advanced Rendering**      | M      | Distance-field lines and post-FX pipeline (Bloom/FXAA)        |

---

## ❄️ Icebox (Long-term / Research)

| Task                              | Description                                                    |
| --------------------------------- | -------------------------------------------------------------- |
| **T_028: Replay System**          | Deterministic turn logs, move history, debug playback controls |
| **T_027: AI Players**             | Heuristic-based sequence generation and headless simulation    |
| **T_033: Architectural Refactor** | Full ECS (Entity Component System) migration                   |
| **T_025: Distribution**           | Build automation, installers, and auto-changelog generation    |

---

## Notes

- **Immediate** focuses on safety and stability (RAII, persistence, core infrastructure)
- **Soon** builds user experience (onboarding, UI polish, input customization)
- **Backlog** contains optimization and polish work; backlog items move to Soon as capacity allows
- **Icebox** holds large architectural changes and long-term features; revisit after Immediate/Soon complete
