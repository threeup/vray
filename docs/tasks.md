# VRay Development Tasks (Tech Lead)

Aligned to goals in goals.md. Adds status visibility, merges overlapping tracks into epics, and uses Now/Next/Later to surface focus while retaining the full 37-task scope.

---

## Legend

- **Status**: Planned | In Progress | Done | Blocked | Superseded
- **Effort**: XS (0.5d) | S (1-2d) | M (3-5d) | L (>5d)
- **Depends**: hard prerequisites; **Related**: helpful but not blocking

---

## Now / Next / Later (work-in-focus)

- **Now**: T_002, T_003, T_008
- **Next**: (none)
- **Later**: All remaining items prioritized by epic below

---

## Epic: Card System (G_C01, G_C02)

### T_000: End-to-End Card Turn Loop

**Status**: In Progress | **Effort**: M | **Depends**: T_001, T_002, T_003, T_004

- Unify player flow on TurnPlan (per-mech assignments) with facing-aware MoveVector and mech roster.
- Decide card ownership (shared vs separate player/AI hands) and remove double resets; clarify turn counter semantics (one increment per round).
- Add HUD for player assignments and active mech/mirror indicators; expose seed control for reproducible AI turns.
- Tests: deterministic round test (player plan + seeded AI), ensure card tests run in CI/build.
- DoD: Player+AI round executes via TurnPlan end-to-end, hand semantics defined, HUD reflects both sides, deterministic test passes.

### T_001: Define Card Data & Mirroring

**Status**: Done | **Effort**: S | **Depends**: None

- Add card data (CardType, CardEffect with MoveVector, Card with precomputed mirroredEffect).
- Mirror rules: lateral swap; forward-only flips sign; copy other fields unchanged.
- Seed a 6-card starting hand.
- DoD: Mirror unit tests pass; hand seeds correctly.

### T_002: Turn Planner (Per-Mech Assignment)

**Status**: In Progress | **Effort**: S | **Depends**: T_001

- Create TurnPlan (up to 3 mech assignments); validate unique mech ids and card availability.
- Apply movement using MoveVector relative to facing; reuse grid bounds/collision checks; ensure player flow uses TurnPlan (not Sequence) with per-mech targeting.
- DoD: Validation tests (duplicates rejected) and movement tests (mirrored vs non-mirrored) pass.

### T_003: Card Hand Management

**Status**: In Progress | **Effort**: S | **Depends**: T_001

- Manage Hand (play-once-per-turn, reset at turn start; allow duplicates if present).
- Decide shared vs separate hands (player/AI), avoid double resets mid-round; expose API to query available cards and mark used.
- DoD: Tests cover play/mark/reset semantics.

### T_004: Random AI Assignment

**Status**: Done | **Effort**: S | **Depends**: T_002, T_003

- For each mech, pick a random card from hand; 50% mirror if allowed.
- Fill TurnPlan and execute.
- DoD: Seeded RNG test verifies selections and mirror flags.

### T_005: Minimal Card UI (Superseded)

**Status**: Superseded by T_029 | **Effort**: M | **Depends**: T_002, T_003

- Retained for traceability; main UI path is T_029.

### T_006: Headless Card Tests

**Status**: Done | **Effort**: S | **Depends**: T_002

- Add unit tests for mirror logic, movement application (grid clamp/collision), and turn validation (duplicate mech id rejected).
- DoD: Tests in tests/card_tests.cpp (or similar) cover mirror, validation, movement.

### T_007: Card Serialization Hooks (Prep)

**Status**: Done | **Effort**: XS | **Depends**: T_001

- Add (de)serialization stubs for card/hand/turn plan; JSON helpers only (no persistence yet).
- DoD: Round-trip test for card/hand/turn plan.

### T_008: Integrate Card Flow into Turn Loop

**Status**: In Progress | **Effort**: S | **Depends**: T_002, T_004, T_005/T_029

- Hook TurnPlan execution into turn progression; ensure AI and player share the same path.
- HUD/text shows assignments for both sides and mirror indicators; seed can be controlled for reproducible AI turns; turn counter increments once per round.
- DoD: Turn executes with card-driven movement for all 3 mechs via TurnPlan; deterministic round test passes; HUD reflects player and AI plans.

### T_029: Card UI Sequence Builder (Drag-Drop)

**Status**: Planned | **Effort**: M | **Depends**: T_015

- Hold/Target areas, drag/drop, remove/reorder, preview updates.
- DoD: Drag/drop moves cards; remove/reorder works; preview reflects target.

---

## Epic: Stability & Observability (G_001)

### T_009: Wrap Render/Input Loop with Error Detection

**Status**: Planned | **Effort**: M | **Depends**: None | **Related**: T_022

- Wrap main loop in try/catch, detect null/missing assets/device failures, graceful shutdown, crash log capture.
- DoD: Error modal displays; app exits cleanly; logs captured.

### T_022: Logging & Telemetry Framework

**Status**: Planned | **Effort**: S-M | **Depends**: T_009

- Integrate logging (levels, rotation); optional opt-in telemetry hooks.
- DoD: Critical events logged; rotation working; telemetry opt-in present.

---

## Epic: Test & CI Track (G_002)

### T_010: Set Up Test Framework (Google Test)

**Status**: Planned | **Effort**: M | **Depends**: None

- Add gtest to CMake, create tests/, add smoke tests (render loop, input polling, resize, asset missing, platform init).
- DoD: Tests build/pass locally <10s; ready for CI.

### T_018: Unit Test Suite for Utility & Math Functions

**Status**: Planned | **Effort**: M | **Depends**: T_010

- Tests for vector ops, matrix math, collision checks, distance/angle; target 80%+ coverage on utility modules.

### T_023: Full Test Suite & CI/CD Automation

**Status**: Planned | **Effort**: L | **Depends**: T_010, T_018

- Expand coverage (render/input/game/save); integration tests; CI/CD pipeline with coverage targets (60%+ overall, 80%+ critical) and gating.

---

## Epic: Memory/Config Safety (G_001, G_004)

### T_011: Replace AppContext::game Raw Pointer with unique_ptr

**Status**: Planned | **Effort**: S | **Depends**: None

- Convert to std::unique_ptr<Game>; remove manual deletes; sanity via sanitizers.

### T_012: Centralize Hardcoded Constants into Config System

**Status**: Planned | **Effort**: S | **Depends**: None

- Add AppConfig, config/default.json, load/replace magic numbers in main/render/constants; test defaults load.

### T_013: Wrap RenderShaders & Models in RAII

**Status**: Planned | **Effort**: S | **Depends**: T_011

- Create ShaderResource/ModelResource wrappers (move-only); update contexts; handle failed loads; test cleanup.

---

## Epic: Platform Abstraction (G_003, G_008)

### T_014: Introduce RenderBackend Interface & Raylib Adapter

**Status**: Planned | **Effort**: M | **Depends**: T_010

- Define RenderBackend, implement Raylib + Mock backends; refactor render.cpp to use interface; tests with both.

### T_015: Introduce InputHandler Interface & Test Doubles

**Status**: Planned | **Effort**: M | **Depends**: T_010

- Define InputHandler; implement Raylib/Test versions; refactor input + cam control; unit tests for camera control.

### T_026: Abstract Raylib Types into Platform-Agnostic Equivalents

**Status**: Planned | **Effort**: M | **Depends**: T_014

- Add math_types.h, adapters, remove raylib types from game modules; conversion tests.

---

## Epic: Persistence & Onboarding (G_004)

### T_016: Implement Save/Config Persistence

**Status**: Planned | **Effort**: M | **Depends**: T_012

- Save/load config/preferences; round-trip tests.

### T_017: Add First-Run Onboarding & Help Flow

**Status**: Planned | **Effort**: M | **Depends**: T_016

- Onboarding modal, help screen, persistent flag; skip path.

---

## Epic: Input & Accessibility (G_005)

### T_019: Input Remapping UI

**Status**: Planned | **Effort**: M | **Depends**: T_015, T_016

- Controls menu, rebind flow with conflict detection; persist bindings.

### T_020: Accessibility Toggles (Contrast, Hold/Toggle, Haptics)

**Status**: Planned | **Effort**: M | **Depends**: T_015, T_016

- High contrast, hold/toggle camera, optional haptics; persisted preferences.

---

## Epic: Performance & Rendering (G_006, G_010)

### T_021: Performance Profiling & Optimization (Culling, Async Load)

**Status**: Planned | **Effort**: M | **Depends**: T_014

- Profile, frustum culling, async asset loading with progress; benchmark to 60 FPS target.

### T_030: Distance-Field Line Rendering Library

**Status**: Planned | **Effort**: M | **Depends**: T_014

- LineRenderer with distance-field shader; configurable thickness/color; render tests.

### T_031: Consolidate Procedural Mesh Generators

**Status**: Planned | **Effort**: M | **Depends**: None

- ProceduralMesh module; refactor generators (tree/pyramid/cube) into reusable functions; vertex-count tests.

### T_032: Instancing & Placement System for Scattered Objects

**Status**: Planned | **Effort**: M | **Depends**: T_014

- InstancedRenderer + ScatterPlacement; perf improvement vs individual draws; profiler validation.

### T_035: Layered Rendering Pipeline (World -> Entities -> UI -> Post-FX)

**Status**: Planned | **Effort**: M | **Depends**: T_014, T_030

- Render layers and pipeline ordering; optional post-FX (bloom/FXAA/color grading); ordering tests.

---

## Epic: AI, Determinism, ECS (G_007, G_009, G_011, G_012)

### T_027: Simple AI Players with Sequence Generation

**Status**: Planned | **Effort**: M | **Depends**: None

- AIPlayer generates sequences with heuristics and difficulty levels; seeded randomness.

### T_028: Deterministic Turn Execution & Replay

**Status**: Planned | **Effort**: M | **Depends**: T_027

- Deterministic turn execution, move history, replay support; determinism tests.

### T_033: Implement Entity-Component-System (ECS) Skeleton

**Status**: Planned | **Effort**: L | **Depends**: T_028

- ECS framework (Entity, Component, System, World); migrate game logic; lifecycle tests.

### T_034: Functional Core + Imperative Shell Refactor

**Status**: Planned | **Effort**: M | **Depends**: T_033

- Extract pure logic module; shell handles I/O; tests for deterministic logic.

### T_036: Headless Simulation Harness for AI vs. AI

**Status**: Planned | **Effort**: M | **Depends**: T_028, T_033

- HeadlessSimulation using mocks; runs AI vs. AI; determinism verified; CI hook.

### T_037: Deterministic Replay & Debug Playback

**Status**: Planned | **Effort**: M | **Depends**: T_028, T_036

- ReplayRecorder/ReplayPlayer, debug UI controls; record->replay parity tests.

---

## Epic: UX, Docs, Distribution (G_010, G_011, G_012)

### T_024: User Documentation & Support

**Status**: Planned | **Effort**: S | **Depends**: T_017

- User manual/help, support channel, in-app feedback.

### T_025: Build/Deploy Automation & Distribution

**Status**: Planned | **Effort**: S | **Depends**: T_023

- Release builds, installers, distribution pipeline, version/changelog automation.

---

## Totals & Critical Path

- **Total Tasks**: 37 (includes superseded T_005 for traceability)
- **Critical Path (Card MVP)**: T_001 -> T_002 -> T_003 -> T_006 -> T_008
- **Critical Path (Core Platform)**: T_010 -> T_014 -> T_015 -> T_019
- **Critical Path (Advanced)**: T_027 -> T_028 -> T_033 -> T_034 -> T_036 -> T_037

---

## Notes on Reorg

- Duplicates merged into epics: Stability (T_009+T_022), Test/CI (T_010+T_018+T_023), Platform (T_014+T_015+T_026).
- Minimal Card UI (T_005) superseded by sequence builder (T_029) but retained for traceability.
- Advanced/long-tail items (T_027-T_037) explicitly in later epics to reduce clutter in Now/Next.
- Status fields added; set to Planned by default - update as work completes.
