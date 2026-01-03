# VRay Development Tasks (Tech Lead)

Implementation plan derived from [goals.md](goals.md) and [vray_eval.md](vray_eval.md) evaluation.

Maps tech work to creative goals. Reference goals.md for "why"; refer here for "what" and "how".

---

## Phase 0: Stabilization (Sprints 1-2)

Support goals G_001 (Crash-free) and G_002 (Quality standards).

### T_001: Wrap Render/Input Loop with Error Detection

**Goal**: G_001 | **Effort**: M (3-5 days)

**Work**:

- Wrap main render/input loop (main.cpp) in try-catch.
- Detect segfaults, null-derefs, missing assets, and device failures.
- Graceful shutdown instead of hang/zombie process.

**Definition of Done**:

- [ ] Error modal tested and displays on crash.
- [ ] Crash log capture working.
- [ ] App exits cleanly; no orphaned processes.

---

### T_002: Set Up Test Framework (Google Test)

**Goal**: G_002 | **Effort**: M (2-3 days)

**Work**:

- Add Google Test to CMakeLists.txt.
- Create `tests/` directory structure.
- Write 5+ smoke tests:
  - `RenderLoopStability`: Run 100 frames without crash.
  - `InputPolling`: Rapid keyboard/mouse input; no buffer overflow.
  - `WindowResize`: Create window, resize, verify frame output.
  - `AssetLoadFailure`: Missing asset; app recovers gracefully.
  - `PlatformInit`: Platform layer initializes and shuts down cleanly.
- Integrate into CI/CD so tests run on every commit (fail build if any fail).

**Definition of Done**:

- [ ] Tests build and pass locally.
- [ ] Tests run in <10 seconds.
- [ ] CI/CD (GitHub Actions or similar) runs tests automatically.
- [ ] Build blocks if tests fail.

---

### T_003: Replace AppContext::game Raw Pointer with unique_ptr

**Goal**: G_001 | **Effort**: S (1 day)

**Work**:

- Change `Game* game;` → `std::unique_ptr<Game> game;` in AppContext struct (app.h).
- Update main.cpp: `ctx.game = std::make_unique<Game>(...)`.
- Remove all manual `delete game;` calls.
- Verify no double-free or use-after-free via sanitizers.

**Definition of Done**:

- [ ] Code compiles.
- [ ] No manual delete calls remain.
- [ ] Smoke tests (T_002) pass.
- [ ] AddressSanitizer clean.

---

### T_004: Centralize Hardcoded Constants into Config System

**Goal**: G_004 | **Effort**: S (1-2 days)

**Work**:

- Create `src/config.h` with struct:
  ```cpp
  struct AppConfig {
    int window_width = 800;
    int window_height = 600;
    int target_fps = 60;
    float camera_fov = 45.0f;
    float camera_pitch = 35.0f;
    float camera_yaw = 23.0f;
    float camera_distance = 22.0f;
  };
  ```
- Create `config/default.json` template with those defaults.
- Add load function in main.cpp that reads from `config/default.json`.
- Replace all magic numbers in main.cpp, constants.h, render.cpp with config fields.
- Write test to verify config loads and defaults apply.

**Definition of Done**:

- [ ] config.h with AppConfig struct.
- [ ] default.json template created.
- [ ] All magic numbers (800, 600, 60, 45.0, 35.0, 23.0, 22.0) replaced.
- [ ] Config loading tested.

---

### T_005: Wrap RenderShaders & Models in RAII

**Goal**: G_001 | **Effort**: S (1-2 days) | **Depends on**: T_003

**Work**:

- Create `ShaderResource` RAII wrapper in new file `src/resource.h`:
  - Constructor: loads shader, validates.
  - Destructor: unloads shader.
  - Move-only (no copy).
- Similarly create `ModelResource`.
- Update AppContext to use `std::vector<ShaderResource>` instead of raw array.
- Add error handling: if LoadShader fails, throw or return error code.
- Document shader/model ownership in comments.

**Definition of Done**:

- [ ] ShaderResource and ModelResource RAII wrappers compile.
- [ ] All shaders/models wrapped; no raw resources.
- [ ] Missing/corrupt asset handling tested (T_002).
- [ ] No memory leaks on exit (AddressSanitizer clean).

---

## Phase 1: Hardening (Sprints 3-4)

Support goals G_003 (Raylib decoupling) and G_004 (Persistence).

### T_006: Introduce RenderBackend Interface & Raylib Adapter

**Goal**: G_003 | **Effort**: M (4-6 days) | **Depends on**: T_002

**Work**:

- Define `RenderBackend` pure abstract class in `src/render_backend.h`:
  ```cpp
  class RenderBackend {
  public:
    virtual ~RenderBackend() = default;
    virtual void SetRenderTarget(RenderTexture rt) = 0;
    virtual void DrawModel(Model m, Vector3 pos, ...) = 0;
    virtual void Clear(Color c) = 0;
    virtual void BeginMode3D(Camera3D c) = 0;
    virtual void EndMode3D() = 0;
    // ... other render calls
  };
  ```
- Implement `RaylibRenderBackend : RenderBackend` with current raylib calls.
- Create `MockRenderBackend : RenderBackend` that tracks calls (no actual drawing).
- Refactor render.cpp to accept `RenderBackend*` instead of calling raylib directly.
- Update Game and all render functions to use interface.
- Write tests: smoke tests use RaylibRenderBackend; unit tests use MockRenderBackend.

**Definition of Done**:

- [ ] RenderBackend interface defined and compiles.
- [ ] RaylibRenderBackend implements all methods.
- [ ] MockRenderBackend tracks calls for unit tests.
- [ ] Smoke tests pass with both implementations.
- [ ] Zero direct raylib calls in render.cpp (except behind adapter).

---

### T_007: Introduce InputHandler Interface & Test Doubles

**Goal**: G_003 | **Effort**: M (4-6 days) | **Depends on**: T_002

**Work**:

- Define `InputHandler` pure abstract class in `src/input_handler.h`:
  ```cpp
  class InputHandler {
  public:
    virtual ~InputHandler() = default;
    virtual bool IsKeyDown(int key) = 0;
    virtual bool IsKeyPressed(int key) = 0;
    virtual Vector2 GetMouseDelta() = 0;
    // ... other input queries
  };
  ```
- Implement `RaylibInputHandler : InputHandler` wrapping platform input.
- Implement `TestInputHandler : InputHandler` with scriptable input (replay, injection).
- Refactor input.cpp and camControl.cpp to use interface (no direct IsKeyDown calls).
- Write unit tests for camera control using TestInputHandler (e.g., "input WASD, verify camera moved").

**Definition of Done**:

- [ ] InputHandler interface defined.
- [ ] RaylibInputHandler and TestInputHandler implemented.
- [ ] All direct input calls removed (use interface).
- [ ] Unit tests for camera control pass.
- [ ] Smoke tests confirm input still works.

---

### T_008: Implement Save/Config Persistence

**Goal**: G_004 | **Effort**: M (3-4 days) | **Depends on**: T_004

**Work**:

- Create `SaveManager` class in `src/save_manager.h`:
  - Load() method: read save file (JSON or binary); apply to AppConfig.
  - Save() method: write current AppConfig to file.
  - GetSaveDir() method: return platform-specific save location (~/.vray_saves or %AppData%).
- Define save file format (recommend JSON):
  ```json
  {
    "window_width": 1024,
    "window_height": 768,
    "camera_pitch": 40.0,
    "control_bindings": { "forward": "W", "back": "S" }
  }
  ```
- On startup: LoadManager::Load() → AppConfig. Fall back to default.json if missing.
- On exit: SaveManager::Save() → write current state to save file.
- Write tests: verify save→load round-trip preserves values.

**Definition of Done**:

- [ ] SaveManager class compiles and links.
- [ ] Save file created on first exit.
- [ ] Load on startup restores previous config.
- [ ] Round-trip test passes (save, load, verify values match).

---

### T_009: Add First-Run Onboarding & Help Flow

**Goal**: G_004 | **Effort**: M (3-4 days) | **Depends on**: T_008

**Work**:

- Create onboarding modal (UI screen) displayed on first run:
  - "Welcome to VRay" header.
  - Control legend: WASD = camera, mouse pan = look, ESC = menu.
  - Brief gameplay overview.
  - "Skip" button (go to game immediately) + "Read Help" button.
- Add persistent `first_run_complete` flag in save file; skip onboarding after first time.
- Create Help screen in main menu:
  - Full control reference.
  - Performance tips.
  - Troubleshooting FAQs.
- Acceptance: user can skip, read, or go straight to gameplay.

**Definition of Done**:

- [ ] Onboarding modal displays on first run.
- [ ] Onboarding content is clear and <30 seconds to read.
- [ ] Help screen accessible from main menu.
- [ ] first_run_complete flag blocks repeat onboarding.
- [ ] Skip button works; gameplay starts immediately.

---

### T_010: Unit Test Suite for Utility & Math Functions

**Goal**: G_002 | **Effort**: M (2-3 days) | **Depends on**: T_002

**Work**:

- Identify all utility functions (math ops, collision checks, etc.).
- Write tests in `tests/` for:
  - Vector operations (add, subtract, dot, cross).
  - Matrix multiply.
  - AABB/sphere collision.
  - Angle/distance calculations.
- Target 80%+ coverage on utility/math modules.
- Integrate into test suite from T_002.

**Definition of Done**:

- [ ] Tests for all math/utility functions.
- [ ] 80%+ line coverage on utility modules.
- [ ] Tests run in <2 seconds.
- [ ] All tests pass.

---

## Phase 2: Evolution (Sprints 5-6)

Support goals G_005 (Accessibility) and G_006 (Performance).

### T_011: Input Remapping UI

**Goal**: G_005 | **Effort**: M (2-3 days) | **Depends on**: T_007, T_008

**Work**:

- Add "Controls" menu in settings.
- Display current bindings (WASD, mouse, ESC, etc.).
- Allow player to rebind:
  - Click binding → prompt for new key.
  - Conflict detection: warn if two actions → same key.
- Save custom bindings to save file.
- Load on startup; override defaults if present.

**Definition of Done**:

- [ ] Controls menu displays all actions.
- [ ] Player can rebind each action.
- [ ] Custom bindings persist to save file.
- [ ] Conflict detection implemented.
- [ ] Game respects custom bindings.

---

### T_012: Accessibility Toggles (Contrast, Hold/Toggle, Haptics)

**Goal**: G_005 | **Effort**: M (2-3 days) | **Depends on**: T_007, T_008

**Work**:

- Add Accessibility menu in settings with toggles:
  - **High Contrast**: Increase color saturation/brightness; improve text readability.
  - **Hold vs. Toggle**: Camera pan can be hold (manual) or toggle (lock and pan).
  - **Haptic Feedback**: Vibration on success/error events (if controller supported).
- Save prefs to save file.
- Apply on startup.

**Definition of Done**:

- [ ] Accessibility menu with toggles.
- [ ] High Contrast mode visibly improves readability.
- [ ] Hold/Toggle mode works for camera.
- [ ] Haptic feedback triggers appropriately.
- [ ] Preferences persist.

---

### T_013: Performance Profiling & Optimization (Culling, Async Load)

**Goal**: G_006 | **Effort**: M (4-6 days) | **Depends on**: T_006

**Work**:

- Profile render loop: frame time, draw calls, memory (use profiler: Tracy, Optick, etc.).
- Implement frustum culling: only draw models in camera view.
- Implement async asset loading: load non-critical assets in background thread.
- Add loading progress bar.
- Benchmark before/after; target 60 FPS on mid-range hardware.

**Definition of Done**:

- [ ] Profiler shows frame time breakdown.
- [ ] Frustum culling reduces draw calls.
- [ ] Async loading with progress bar working.
- [ ] Benchmark: 60 FPS on target hardware.
- [ ] No stutter during loads.

---

## Phase 3: Polish (Sprints 7+)

Support goal G_007 (Complete product).

### T_014: Logging & Telemetry Framework

**Goal**: G_001 | **Effort**: S (1-2 days) | **Depends on**: T_001

**Work**:

- Integrate spdlog or similar lightweight logging library.
- Define log levels: DEBUG, INFO, WARN, ERROR.
- Log events:
  - App start/stop.
  - Asset loads/failures.
  - Render/perf metrics (FPS, frame time).
- Write logs to `app.log` with rotation (max 10 MB, 3 files).
- Add optional telemetry opt-in (no forced data collection).

**Definition of Done**:

- [ ] Logging library integrated.
- [ ] Critical events logged.
- [ ] Log files rotate; no unbounded disk.
- [ ] Telemetry opt-in (not forced).
- [ ] Logs useful for field debugging.

---

### T_015: Full Test Suite & CI/CD Automation

**Goal**: G_002 | **Effort**: L (5-7 days) | **Depends on**: T_002, T_007, T_010

**Work**:

- Expand test suite: cover all major code paths (render, input, game logic, save/load).
- Implement integration tests: render + input + game loop together.
- Set up CI/CD pipeline (GitHub Actions):
  - Compile on every commit.
  - Run tests; fail build if any fail.
  - Generate and track coverage reports (target 60%+ overall, 80%+ critical paths).
  - Auto-deploy to staging on main branch.

**Definition of Done**:

- [ ] 60%+ code coverage overall.
- [ ] 80%+ coverage on render/input/game modules.
- [ ] CI/CD runs on every push.
- [ ] Build fails if tests don't pass.
- [ ] Coverage reports tracked over time.

---

### T_016: User Documentation & Support

**Goal**: G_007 | **Effort**: S (1-2 days) | **Depends on**: T_009

**Work**:

- Write user manual (online + in-app):
  - Getting started guide.
  - Controls and camera.
  - Troubleshooting FAQs.
  - Graphics/performance tips.
- Create support channel (email, Discord, GitHub Issues).
- Set up feedback form in app.

**Definition of Done**:

- [ ] User manual covers key topics.
- [ ] Support channel active and monitored.
- [ ] In-app help complete.
- [ ] FAQ reflects common questions.

---

### T_017: Build/Deploy Automation & Distribution

**Goal**: G_012 | **Effort**: S (1-2 days) | **Depends on**: T_015

**Work**:

- Automate release builds (Windows, macOS, Linux).
- Create installers (.msi, .dmg, .deb).
- Publish to distribution platform (itch.io, GitHub Releases, Steam).
- Automate version bumping and changelog generation.
- Set up staged rollout (beta → production).

**Definition of Done**:

- [ ] Release builds automated and reproducible.
- [ ] Installers created for target platforms.
- [ ] Distribution pipeline tested end-to-end.
- [ ] Version/changelog auto-managed.

---

## Phase 4: Advanced Features (Future)

Support goals G_008 (Platform abstraction), G_009 (Gameplay), G_010 (Advanced rendering), G_011 (Architecture), G_012 (Headless testing).

### T_018: Abstract Raylib Types into Platform-Agnostic Equivalents

**Goal**: G_008 | **Effort**: M (4-5 days) | **Depends on**: T_006

**Work**:

- Create `src/math_types.h` with platform-agnostic types:
  ```cpp
  struct Vec3f { float x, y, z; };
  struct ColorRGBA { uint8_t r, g, b, a; };
  struct Matrix4f { /* ... */ };
  using RenderableHandle = uint32_t;
  ```
- Create adapters in `src/platform/raylib_bridge.h` to convert between raylib and game types.
- Remove all direct raylib type usage from game.cpp, camControl.h, character_renderer.cpp, etc.
- Update render callbacks to use RenderableHandle instead of Model/Texture/Shader directly.
- Write tests verifying conversions preserve data.

**Definition of Done**:

- [ ] Platform-agnostic types defined in math_types.h.
- [ ] Raylib adapters created and tested.
- [ ] Zero raylib types in game modules (only platform layer).
- [ ] Conversion tests pass.

---

### T_019: Implement Simple AI Players with Sequence Generation

**Goal**: G_009 | **Effort**: M (5-7 days)

**Work**:

- Create `AIPlayer` class in `src/ai_player.h`:
  - Generates card sequences based on game state.
  - Simple heuristics: maximize score, block opponent, etc.
  - Configurable difficulty (easy/medium/hard).
- Implement sequence generation: choose 3-5 cards from available pool.
- Add deterministic randomness (seed-based for replay).
- Write tests: AI generates valid sequences; behavior changes with difficulty.

**Definition of Done**:

- [ ] AIPlayer class compiles.
- [ ] AI generates valid sequences.
- [ ] Difficulty levels produce different strategies.
- [ ] Deterministic with seeded randomness (replay-able).

---

### T_020: Implement Deterministic Turn Execution & Replay

**Goal**: G_009 | **Effort**: M (3-4 days) | **Depends on**: T_019

**Work**:

- Refactor turn execution to be deterministic (no floating-point order-of-operations drift).
- Store move history: player, sequence, timestamp.
- Implement replay: load history, re-execute moves, watch game unfold.
- Add `GameState` snapshot for save/restore.
- Write tests: two games with same input produce identical outcome.

**Definition of Done**:

- [ ] Turn execution deterministic (verified via replay).
- [ ] Move history captured and serializable.
- [ ] Replay mechanism working.
- [ ] Determinism tests pass.

---

### T_021: Build Card UI Sequence Builder (Hold Area, Target Area, Drag-Drop)

**Goal**: G_009 | **Effort**: M (4-5 days) | **Depends on**: T_007

**Work**:

- Create UI panels:
  - "Hold Area": available cards (hand) displayed as grid.
  - "Target Area": cards selected for sequence (ordered list).
  - "Sequence Preview": shows card effects/outcome.
- Implement drag-drop: hold card from Hold Area → drop in Target Area.
- Implement remove: right-click card in Target Area to remove.
- Implement reorder: drag card within Target Area to reorder.
- Write tests: drag→drop updates target; remove clears slot; preview updates.

**Definition of Done**:

- [ ] Hold and Target areas display cards.
- [ ] Drag-drop moves card from Hold to Target.
- [ ] Cards can be removed and reordered.
- [ ] Preview updates when Target changes.

---

### T_022: Create Distance-Field Line Rendering Library

**Goal**: G_010 | **Effort**: M (3-4 days) | **Depends on**: T_006

**Work**:

- Create `LineRenderer` class in `src/render/line_renderer.h`:
  - Renders anti-aliased lines via distance-field shader.
  - Supports variable thickness, colors, dash patterns.
- Implement shader in `assets/line_distance_field.fs`:
  - Reads distance field texture.
  - Outputs antialiased line with smooth edges.
- Add methods:
  - `DrawLine(Vec3f start, Vec3f end, ColorRGBA color, float thickness)`
  - `DrawPolyline(vector<Vec3f>, ...)`
- Write tests: line appears correct on screen; thickness varies.

**Definition of Done**:

- [ ] LineRenderer class compiles.
- [ ] Distance-field shader implemented.
- [ ] Lines rendered with anti-aliasing.
- [ ] Thickness and color configurable.

---

### T_023: Consolidate Procedural Mesh Generators (Trees, Pyramids, etc.)

**Goal**: G_010 | **Effort**: M (3-4 days)

**Work**:

- Create `ProceduralMesh` module in `src/render/procedural_mesh.h`:
  - Refactor existing generators (tree, pyramid, cube, etc.) into reusable functions.
  - Document parameters (height, base_width, iterations, etc.).
  - Generate mesh vertices/indices; return in neutral format (not raylib-specific).
- Implement generators:
  - `GenerateTree(height, base_width, branches) → Mesh`
  - `GeneratePyramid(base_size, height) → Mesh`
  - `GenerateCube(size) → Mesh`
- Write tests: mesh generation doesn't crash; vertex counts match expectations.

**Definition of Done**:

- [ ] ProceduralMesh module compiles.
- [ ] All generator functions documented with parameters.
- [ ] Generated meshes are platform-agnostic (not raylib types).
- [ ] Tests verify mesh generation.

---

### T_024: Add Instancing & Placement System for Scattered Objects

**Goal**: G_010 | **Effort**: M (2-3 days) | **Depends on**: T_006

**Work**:

- Create `InstancedRenderer` in `src/render/instanced_renderer.h`:
  - Renders multiple copies of same mesh efficiently (GPU instancing).
  - Accepts list of transforms (position, rotation, scale).
- Create `ScatterPlacement` helper:
  - Places N objects on grid with minimum spacing (no overlaps).
  - Configurable density, randomization, boundaries.
- Refactor rendering to use instancing instead of N separate DrawModel calls.
- Write tests: 1000 objects render faster than N individual calls.

**Definition of Done**:

- [ ] InstancedRenderer compiles and renders.
- [ ] ScatterPlacement generates non-overlapping positions.
- [ ] Performance improvement verified (profiler shows fewer draw calls).

---

### T_025: Implement Entity-Component-System (ECS) Skeleton

**Goal**: G_011 | **Effort**: L (6-8 days) | **Depends on**: T_020

**Work**:

- Create ECS framework in `src/ecs/`:
  - `Entity` struct: ID + component mask.
  - `Component` base class (POD data only).
  - `System` base class (pure logic, reads/writes components).
  - `World` container: holds entities, components, runs systems.
- Implement core components:
  - `TransformComponent` (position, rotation, scale).
  - `RenderComponent` (mesh, material handles).
  - `GameStateComponent` (health, status, etc.).
- Implement core systems:
  - `RenderSystem` (culls and draws).
  - `UpdateSystem` (moves, animates).
  - `CollisionSystem` (detects overlaps).
- Migrate monolithic Game struct to ECS.
- Write tests: entity creation/destruction works; systems run in order.

**Definition of Done**:

- [ ] ECS framework compiles.
- [ ] Core components and systems implemented.
- [ ] Game logic migrated to ECS.
- [ ] Tests verify entity lifecycle and system order.

---

### T_026: Refactor Game Logic to Functional Core + Imperative Shell

**Goal**: G_011 | **Effort**: M (4-5 days) | **Depends on**: T_025

**Work**:

- Separate pure logic (no I/O, deterministic):
  - Card effects, damage calculations, sequence resolution.
  - Moves into `src/game/game_logic.h` (pure functions).
- Separate imperative shell (I/O, side effects):
  - Rendering, input handling, file I/O.
  - Stays in main game loop; calls game_logic functions.
- Write tests: logic functions are pure (same input → same output).
- Document which modules are pure vs. impure.

**Definition of Done**:

- [ ] Pure logic extracted to separate module.
- [ ] Game loop calls pure logic functions.
- [ ] Tests verify purity (deterministic outcomes).
- [ ] Architecture documented.

---

### T_027: Create Layered Rendering Pipeline (World → Entities → UI → Post-FX)

**Goal**: G_010 | **Effort**: M (3-4 days) | **Depends on**: T_006, T_022

**Work**:

- Define render layers:
  - Layer 0: World (grid, terrain, static props).
  - Layer 1: Entities (game pieces, characters).
  - Layer 2: UI (menus, overlays, debug).
  - Layer 3: Post-FX (bloom, FXAA, color grading).
- Create `RenderPipeline` that orchestrates layers in order.
- Implement layer scheduling (sort by depth, material, etc.).
- Add post-FX shaders: bloom, FXAA, color grading (optional).
- Write tests: layers render in correct order; depth sorting works.

**Definition of Done**:

- [ ] Render pipeline orchestrates layers.
- [ ] Layers render in correct order.
- [ ] Depth sorting works within layers.
- [ ] Post-FX effects optional and configurable.

---

### T_028: Build Headless Simulation Harness for AI vs. AI

**Goal**: G_012 | **Effort**: M (3-4 days) | **Depends on**: T_020, T_025

**Work**:

- Create `HeadlessSimulation` class in `src/test/headless_sim.h`:
  - Runs game loop without rendering (no window, no raylib).
  - Uses MockRenderBackend and MockInputHandler.
  - Executes full game: AI vs. AI 100 turns.
  - Returns outcome (winner, final state, move history).
- Integrate into test suite:
  - Run 100 AI vs. AI matches automatically.
  - Log move history for every match (debugging).
  - Detect crashes, hangs, invalid states.
- Write tests: matches complete without error; outcomes are deterministic (replay matches outcome).

**Definition of Done**:

- [ ] HeadlessSimulation runs without graphics.
- [ ] AI vs. AI matches complete successfully.
- [ ] Determinism verified (replay matches original).
- [ ] Integrated into CI/CD.

---

### T_029: Add Deterministic Replay & Debug Playback

**Goal**: G_012 | **Effort**: M (2-3 days) | **Depends on**: T_020, T_028

**Work**:

- Create `ReplayPlayer` class:
  - Loads move history from file.
  - Re-executes moves in game; renders output.
  - Pauses/steps through turns.
- Create `ReplayRecorder`:
  - Saves move history during normal play.
  - Captures initial state and every move.
- Add debug UI:
  - "Load Replay" menu item.
  - Play/pause/step controls.
  - Frame counter and move inspector.
- Write tests: record→replay produces identical game state.

**Definition of Done**:

- [ ] Replay system records and loads move history.
- [ ] Replay playback produces identical outcome.
- [ ] Debug UI controls replay.
- [ ] Tests verify determinism.

---

## Quick Legend

| Symbol           | Meaning                                                                |
| ---------------- | ---------------------------------------------------------------------- |
| **T_NNN**        | Task identifier (tech work item)                                       |
| **G_NNN**        | Goal identifier (creative objective from goals.md)                     |
| **S**            | Small: 1-2 days                                                        |
| **M**            | Medium: 3-5 days                                                       |
| **L**            | Large: >5 days                                                         |
| **Blocker**      | Must complete before starting this task                                |
| **Dependencies** | Needed for context; should complete before, but doesn't strictly block |

---

## Summary & Critical Path

- **Total Tasks**: 29 (17 Phase 0-3, 12 Phase 4 advanced)
- **Total Estimated Effort**: ~180-220 days (8-10 months for team of 3-4)
- **Critical Path (MVP)**: T_001 → T_002 → T_006 → T_007 → T_011 (~12-15 weeks)
- **Critical Path (Advanced)**: + T_025 → T_026 → T_028 (~16-20 weeks for full-featured)

**Next Steps**:

1. Assign owners to each task.
2. Create sprints in project management tool.
3. Begin Phase 0 immediately.
4. Phase 4 tasks scheduled after Phase 3 stabilization.

Reference [goals.md](goals.md) for creative context and [vray_eval.md](vray_eval.md) for evaluation details.

**Description**:

- Create `ShaderResource` RAII wrapper (constructor loads, destructor unloads).
- Create `ModelResource` RAII wrapper similarly.
- Update AppContext to use `std::vector<ShaderResource>` instead of raw array.
- Add checks: if LoadShader fails, throw exception or return error code.
- Document shader/model lifetime in comments.

**Acceptance Criteria**:

- [ ] ShaderResource and ModelResource RAII wrappers created.
- [ ] All shader/model loads wrapped in RAII.
- [ ] Error handling for missing/corrupt assets.
- [ ] Smoke tests confirm cleanup on exit.

**Notes**: Reduces memory/resource leaks. Enables proper error handling. See [vray_implement.md#1.3](vray_implement.md#1.3) and [vray_strategy.md#7.1](vray_strategy.md#7.1).

---

## Phase 1: Hardening (Sprints 3-4)

### P1.1 Introduce RenderBackend Interface & Adapter

**Priority**: HIGH  
**Component**: Arch/Render  
**Status**: Not Started  
**Effort**: M (4-6 days)  
**Owner**: (Assign Arch)  
**Blocker**: P0.2  
**Dependencies**: P0.2 (smoke tests must pass)

**Description**:

- Define `RenderBackend` interface (abstract base class) with methods:
  - `SetRenderTarget(RenderTexture rt);`
  - `DrawModel(Model m, Vector3 pos, ...);`
  - `Clear(Color c);`
  - etc.
- Implement `RaylibRenderBackend : RenderBackend` with current raylib calls.
- Update render.cpp to use RenderBackend instead of direct raylib.
- Pass RenderBackend\* to Game and rendering functions.
- Add test double: `MockRenderBackend` for unit tests (no actual drawing).

**Acceptance Criteria**:

- [ ] RenderBackend interface defined.
- [ ] RaylibRenderBackend adapter implemented.
- [ ] All raylib rendering calls moved behind interface.
- [ ] MockRenderBackend created for tests.
- [ ] Smoke tests pass with both implementations.

**Notes**: Unblocks testing, alternate backends, headless rendering. Critical for testability. See [vray_strategy.md#4.1](vray_strategy.md#4.1).

---

### P1.2 Introduce InputHandler Interface & Test Doubles

**Priority**: HIGH  
**Component**: Arch/Input  
**Status**: Not Started  
**Effort**: M (4-6 days)  
**Owner**: (Assign Arch)  
**Blocker**: P0.2  
**Dependencies**: P0.2 (smoke tests must pass)

**Description**:

- Define `InputHandler` interface with methods:
  - `IsKeyDown(int key);`
  - `IsKeyPressed(int key);`
  - `GetMouseDelta();`
  - etc.
- Implement `RaylibInputHandler : InputHandler` wrapping platform input.
- Implement `TestInputHandler : InputHandler` with scripted inputs for testing.
- Update input.cpp and game logic to use InputHandler.
- Write unit tests for camera control using TestInputHandler.

**Acceptance Criteria**:

- [ ] InputHandler interface defined.
- [ ] RaylibInputHandler and TestInputHandler implemented.
- [ ] All direct input calls replaced with interface.
- [ ] Unit tests for camera control pass.
- [ ] Smoke tests confirm input still works.

**Notes**: Enables input replay, testing, and custom input devices. See [vray_strategy.md#4.1](vray_strategy.md#4.1).

---

### P1.3 Implement Save/Config System

**Priority**: HIGH  
**Component**: Feature/UX  
**Status**: Not Started  
**Effort**: M (4-6 days)  
**Owner**: (Assign)  
**Blocker**: P0.4  
**Dependencies**: P0.4 (config centralization)

**Description**:

- Define save file format (JSON or binary).
- Save player prefs:
  - Window size, fullscreen toggle.
  - Camera angle/distance defaults.
  - Control bindings (if remap exists).
  - Graphics settings (brightness, etc.).
- Create SaveManager class with Load/Save methods.
- On startup, load from save file; fall back to defaults.
- On exit or menu Save, persist to file.
- Add UI menu item: "Settings" → window, camera, bindings.

**Acceptance Criteria**:

- [ ] Save file format defined and documented.
- [ ] SaveManager Load/Save implemented.
- [ ] Save file created and restored on app restart.
- [ ] Settings UI menu created (optional if P1.4 not done).
- [ ] Smoke test verifies save/load cycle.

**Notes**: Improves user retention and customization. See [vray_vision.md#1.1](vray_vision.md#1.1) persistence gaps.

---

### P1.4 Add First-Run Onboarding & Help Screen

**Priority**: HIGH  
**Component**: UX/Content  
**Status**: Not Started  
**Effort**: M (4-6 days)  
**Owner**: (Assign Design + Dev)  
**Blocker**: None  
**Dependencies**: None

**Description**:

- On first run, display onboarding modal:
  - Welcome message.
  - Basic controls (WASD for camera, mouse for pan, ESC to menu).
  - Brief mechanic overview.
  - "Skip" button to go to gameplay.
- Add persistent "first-run" flag in save file.
- Create Help screen in menu:
  - Full control list.
  - Graphics/performance tips.
  - Troubleshooting FAQs.
- Make help accessible from main menu.

**Acceptance Criteria**:

- [ ] Onboarding modal displays on first run.
- [ ] Onboarding content is clear and <30 seconds to read.
- [ ] Help screen accessible from menu.
- [ ] First-run flag prevents repeat onboarding.
- [ ] User can skip and go straight to game.

**Notes**: Improves new user experience and adoption. See [vray_vision.md#2.3](vray_vision.md#2.3) adoption blockers.

---

### P1.5 Unit Tests for Utility & Math Functions

**Priority**: MEDIUM  
**Component**: Test/Quality  
**Status**: Not Started  
**Effort**: M (3-4 days)  
**Owner**: (Assign QA)  
**Blocker**: P0.2  
**Dependencies**: P0.2 (test framework set up)

**Description**:

- Identify utility/math functions (vector ops, matrix transforms, collision checks, etc.).
- Write unit tests for:
  - Vector add/subtract/dot/cross.
  - Matrix multiply.
  - Collision detection (AABB, sphere).
  - Angle/distance calculations.
- Aim for 80%+ coverage on math module.
- Integrate into smoke test suite.

**Acceptance Criteria**:

- [ ] Unit tests for all math functions.
- [ ] 80%+ line coverage on utility/math modules.
- [ ] Tests run in <2 seconds.
- [ ] CI/CD runs tests on every commit.

**Notes**: Reduces bugs and improves confidence in refactoring. Foundation for P2 optimization.

---

## Phase 2: Evolution (Sprints 5-6)

### P2.1 Input Remapping UI

**Priority**: MEDIUM  
**Component**: UX/Feature  
**Status**: Not Started  
**Effort**: M (3-5 days)  
**Owner**: (Assign)  
**Blocker**: P1.2  
**Dependencies**: P1.2 (InputHandler interface), P1.3 (Save/Config)

**Description**:

- Add "Controls" menu to settings.
- Display current bindings (WASD, mouse, ESC).
- Allow player to rebind keys:
  - Click on binding → press new key.
  - Conflict detection (warn if two actions bound to same key).
- Save custom bindings to save file.
- Load custom bindings on startup.

**Acceptance Criteria**:

- [ ] Controls menu displays all bindable actions.
- [ ] Player can rebind each action.
- [ ] Custom bindings persisted to save file.
- [ ] Conflict detection and warning.
- [ ] Game respects custom bindings.

**Notes**: Improves accessibility and user satisfaction. Depends on InputHandler abstraction.

---

### P2.2 Accessibility Toggles (Contrast, Hold/Toggle, Haptics)

**Priority**: MEDIUM  
**Component**: UX/A11y  
**Status**: Not Started  
**Effort**: M (3-5 days)  
**Owner**: (Assign)  
**Blocker**: P1.2  
**Dependencies**: P1.2 (InputHandler), P1.3 (Config/Save)

**Description**:

- Add accessibility section to settings.
- Implement toggles:
  - **High Contrast**: Double color saturation/brightness differences.
  - **Hold vs. Toggle**: Camera pan can be hold (manual) or toggle (lock).
  - **Haptic Feedback**: Vibration on success/error (if supported).
- Save preferences to config file.
- Apply at startup.

**Acceptance Criteria**:

- [ ] Accessibility menu with toggles.
- [ ] High Contrast mode visibly improves text/UI clarity.
- [ ] Hold/Toggle mode works for camera movement.
- [ ] Haptic feedback triggers on appropriate events.
- [ ] Preferences persist across sessions.

**Notes**: Broadens player base; improves inclusivity. Low engineering complexity.

---

### P2.3 Performance Optimization: Culling & Async Loading

**Priority**: MEDIUM  
**Component**: Perf/Render  
**Status**: Not Started  
**Effort**: M (4-6 days)  
**Owner**: (Assign Perf)  
**Blocker**: P1.1  
**Dependencies**: P1.1 (RenderBackend), profiling tools

**Description**:

- Profile render loop (frame time, draw calls, memory).
- Implement frustum culling: only draw models in camera view.
- Implement async asset loading: load non-critical assets in background thread.
- Add loading progress bar for long-running loads.
- Benchmark before/after; target 60 FPS on mid-range hardware.

**Acceptance Criteria**:

- [ ] Profiler output shows frame time breakdown.
- [ ] Frustum culling implemented; draw calls reduced.
- [ ] Async loading integrated with loading bar.
- [ ] Benchmarks confirm 60 FPS on target hardware.
- [ ] No perceptible stuttering during loads.

**Notes**: Improves user experience on lower-end hardware. Requires profiling infrastructure.

---

### P2.4 Logging & Telemetry Framework

**Priority**: LOW  
**Component**: Infra/Observability  
**Status**: Not Started  
**Effort**: S-M (2-4 days)  
**Owner**: (Assign Infra)  
**Blocker**: None  
**Dependencies**: P0.1 (error detection) recommended

**Description**:

- Integrate lightweight logging library (spdlog, serilog, or similar).
- Define log levels: DEBUG, INFO, WARN, ERROR.
- Log critical events:
  - App start/stop.
  - Asset loads/failures.
  - Input events (optional, at DEBUG level).
  - Render/perf metrics (FPS, frame time).
- Write logs to file (app.log) with rotation.
- Add optional telemetry hooks (opt-in) for crash reporting.

**Acceptance Criteria**:

- [ ] Logging library integrated.
- [ ] All critical events logged.
- [ ] Log files rotate; no unbounded disk use.
- [ ] Telemetry opt-in available (not forced).
- [ ] Log output is useful for debugging field issues.

**Notes**: Improves observability and user support. Deferred if not blocking shipping.

---

## Phase 3: Polish (Sprints 7+)

### P3.1 Full Automated Test Suite & CI/CD

**Priority**: LOW  
**Component**: Test/Infra  
**Status**: Not Started  
**Effort**: L (5-7 days)  
**Owner**: (Assign QA + DevOps)  
**Blocker**: P1.2 (InputHandler)  
**Dependencies**: All earlier phases

**Description**:

- Expand test suite to cover all major code paths.
- Implement integration tests: render + input + game loop.
- Set up CI/CD pipeline (GitHub Actions, Jenkins, etc.):
  - Compile on every commit.
  - Run tests; fail build if any fail.
  - Generate coverage reports.
  - Auto-deploy to staging on main branch.
- Target 60%+ overall coverage; 80%+ on critical paths.

**Acceptance Criteria**:

- [ ] 60%+ overall code coverage.
- [ ] 80%+ coverage on render/input/game modules.
- [ ] CI/CD pipeline runs on every push.
- [ ] Build fails if tests don't pass.
- [ ] Coverage reports generated and tracked.

**Notes**: Foundation for confident shipping and iteration. Reduces regression risk.

---

### P3.2 User Documentation & Support

**Priority**: LOW  
**Component**: Content/Support  
**Status**: Not Started  
**Effort**: S (2-3 days)  
**Owner**: (Assign Doc + PM)  
**Blocker**: None  
**Dependencies**: P1.4 (help content)

**Description**:

- Write user manual (online or in-app):
  - Getting started guide.
  - Controls and camera.
  - Troubleshooting common issues.
  - Graphics/performance tips.
  - FAQ.
- Create support channel (email, Discord, GitHub Issues).
- Set up feedback form in app.

**Acceptance Criteria**:

- [ ] User manual covers key topics.
- [ ] Support channel active and monitored.
- [ ] In-app help/about pages complete.
- [ ] FAQ reflects common user questions.

**Notes**: Improves user satisfaction and reduces support load.

---

### P3.3 Build/Deploy Automation & Distribution

**Priority**: LOW  
**Component**: Infra/DevOps  
**Status**: Not Started  
**Effort**: S (2-3 days)  
**Owner**: (Assign DevOps)  
**Blocker**: P3.1 (CI/CD)  
**Dependencies**: P3.1 (automated tests)

**Description**:

- Automate release builds (Windows, macOS, Linux if applicable).
- Create installers (.msi, .dmg, .deb).
- Publish to distribution platform (itch.io, GitHub Releases, Steam, etc.).
- Automate version bumping and changelog generation.
- Set up staged rollout (beta → production).

**Acceptance Criteria**:

- [ ] Release builds automated and reproducible.
- [ ] Installers created for target platforms.
- [ ] Distribution pipeline tested end-to-end.
- [ ] Version and changelog auto-managed.

**Notes**: Deferred until feature-complete and stable.

---

## Legend

**Priority**: CRITICAL > HIGH > MEDIUM > LOW  
**Effort**: S (small, 1-2 days) | M (medium, 3-5 days) | L (large, >5 days)  
**Status**: Not Started | In Progress | In Review | Complete

---

## Summary

- **Total Tasks**: 15
- **Total Estimated Effort**: ~80-100 days (4-5 months with team of 3-4)
- **Critical Path**: P0.1 → P0.2 → P1.1 → P2.1 (~6-8 weeks)
- **Owner Assignment Pending**: All tasks marked "(Assign)" require PM/Tech Lead assignment.

**Next Steps**:

1. Assign owners and estimates per task.
2. Create sprints in project management tool.
3. Review and refine with stakeholders.
4. Begin Phase 0 (Stabilization) immediately.

See [vray_eval.md](vray_eval.md) for roadmap and strategy context.

- T004 (Setup) Add a LICENSE.
- T006 (Build) Add a simple CMake build system for a minimal raylib app.
- T007 (Build) Document development prerequisites.
- T009 (Card) Define `Card` and `Sequence` data structures.
- T010 (Card) Implement card sequence execution semantics.
- T012 (GameLoop) Implement a basic game loop with keyboard-triggered sequences.
- T014 (UI) Add basic UI for hand/sequence management.
- T023 (Sim) Define the 12x12 grid data structure.
- T024 (Sim) Implement a basic entity system for grid objects.
- T025 (Sim) Create procedural vector art meshes for 3D objects.
- T026 (Sim) Add 3D rendering of the grid and entities.
- T046 (Render) Implement Line Drawing with Alpha Blending.
- T047 (Render) Create a Custom Shader for marker edge effect.
- T048 (Render) Implement Line Thickness Control.
- T052 (Render) Implement a shader-based grid rendering system for crisp, anti-aliased lines.

</details>
