# Tasks — Vray Demo

This file tracks development tasks for the Vray Demo project.

## Current Goals

- **Platform Abstraction**: Finalize the abstraction layer to decouple the core game from `raylib`.
- **Simulation Core**: Implement AI and concurrent turn resolution.
- **Web Export**: Ensure the Emscripten build is stable and feature-complete.
- **Procedural Assets (side tangent)**: Keep the recent procedural mesh work around trees/pyramids organized for reuse without derailing core goals.

## Active Tasks

### Platform Abstraction

- [ ] T041 (Platform) Abstract raylib-specific types (`Vector3`, `Color`, `Model`, `Camera3D`) from the platform interfaces. Create platform-agnostic equivalents (e.g., `Vec3f`, `ColorRGBA`, `RenderableHandle`).
- [ ] T042 (Platform) Update `RendererInterface` and its implementations to use the new platform-agnostic types, making the core rendering API truly library-independent.
- [ ] T043 (Platform) Refactor modules like `game.cpp`, `camControl.h`, and `character_renderer.cpp` to remove direct dependencies on raylib types.

### Simulation & Gameplay

- [ ] T016 (Sim) Implement simple AI players that generate and submit card sequences.
- [ ] T017 (Sim) Define deterministic, interleaved execution for concurrent sequences.
- [ ] T055 (Card UI) Create a holding area for cards and a target area to drag them into for sequence building.

### Rendering

- [x] T052 (Render) Implement a shader-based grid rendering system for crisp, anti-aliased lines.
- [ ] T056 (Render) Create a general-purpose library for rendering lines via a distance-field shader.
- [ ] T060 (Render) Consolidate procedural mesh generators (cube/octa canopy tree, cubic star, pyramid) into a reusable module with documented parameters.
- [ ] T061 (Render) Add a simple instancing/placement helper that scatters N models over a grid with minimum spacing (no manual coordinates in render code).

---

## Backlog

These are approved architectural goals to be scheduled into active development.

- [ ] T050 (Arch) Implement a core Entity-Component-System (ECS) to replace the monolithic `Game` struct.
- [ ] T051 (Arch) Refactor game logic to follow a "Functional Core, Imperative Shell" pattern.
- [ ] T053 (Render) Create a layered rendering pipeline for effects (world -> entities -> UI -> post-fx).
- [ ] T054 (Test) Build a headless simulation harness for automated testing and CI validation.

---

<details>
<summary>Completed Tasks</summary>

- T002 (Setup) Create `src/`, `assets/`, `build/`, `docs/` directories.
- T003 (Setup) Add a minimal `README.md` and `tasks.md`.
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
