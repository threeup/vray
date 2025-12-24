# Architecture — Vray Demo (Revised)

This document outlines the architectural design for the Vray Demo project.

## Overview

- **Project**: A C++ card-sequence game where players (human, AI) submit ordered card sequences that execute deterministically on a grid-based world.
- **Core Systems**:
  - **Simulation**: Grid-based world (chessboard-like) for entity placement, movement, and interactions.
  - **Card Engine**: Card definitions, sequence construction, ordering, and deterministic application of effects.
- **Targets**:
  - Desktop: `raylib` + `raygui`
  - Web: Emscripten (WASM)
- **Build System**: `CMake` for cross-platform builds and Emscripten integration.
- **Rendering**:
  - Procedural/vector meshes instead of bitmap sprites for scalability and lightweight web deployment.
  - Side tangent kept: procedural tree/pyramid variants and auto-instanced placement; keep reusable but optional.
- **Grid**:
  - Recommended bounded 12×12 grid (configurable), serving as spatial substrate for positioning and area-of-effect resolution.

---

## Components

### Game Model

- **Card**: ID, name, parameters (target, magnitude, duration, metadata).
- **Sequence**: Ordered list of card references with execution metadata (owner, timestamp).
- **GameState**: Entities, resources, turn/time, event queue.

### Execution / Semantics

- Playing a sequence applies each card’s effect to the game state in order.
- Effects are pure functions: `(GameState, Card, Context) -> GameStateDelta` for deterministic replay and testing.
- Concurrency: Each actor maintains its own sequence; main loop interleaves deltas for smooth visual updates.

### Game Loop

- Fixed-step loop (e.g., 60Hz): poll input, advance simulation, apply sequence steps, render.
- Headless mode for unit tests and deterministic validation.

### UI & Rendering

- `raylib`: rendering + input.
- `raygui`: immediate-mode GUI for card hand, sequence editor, play controls.
- UI issues commands to the model; model owns state.
- Procedural mesh utilities: `MeshGenerator` hosts cube/octa canopy trees, cubic stars, pyramids; keep parameterized for reuse.
- Instancing helper: simple grid scatter with spacing ≥2 units to place demo entities without manual coordinates.

### Build & Export

- `CMake` for native and web builds.
- Native: link against raylib.
- Web: Emscripten → WASM + HTML shell.

---

## Proposed File/Layout Refactor

Goal: Move subsystems out of `main.cpp` into focused modules for composability.

- **src/app.h/.cpp**

  - `AppContext` struct: window, camera, render targets, shaders, models, game, UI state.
  - `init_app`, `shutdown_app`, `rebuild_render_targets`.

- **src/render.h/.cpp**

  - `load_shaders`, `load_models`.
  - `render_scene`, `post_process`.

- **src/ui.h/.cpp**

  - Panels for card/sequence editing and render controls.

- **src/game.h/.cpp**

  - `Game` struct: grid, entities, sequences, hand, rotations.
  - `init_game`, `update_game`.

- **src/resources.h/.cpp**

  - Helpers for meshes/models/shaders/targets with RAII cleanup.

- **src/constants.h**

  - Tunables: window size, grid cell height, AA defaults, palette, radii, speeds.

- **src/main.cpp**
  - Thin orchestration: init, loop, shutdown.

**Benefits**:

- Clear separation of concerns.
- Centralized GPU resource management.
- Easier AA/post-effect experimentation without touching simulation.

---

# Suggested Architectural Improvements

Your current design is solid for a raylib demo, but here are stronger architectural bases to consider:

### 1. **Entity-Component-System (ECS) Core**

- Replace monolithic `Game` struct with ECS.
- Entities = IDs; Components = data (position, health, sequence queue); Systems = logic (movement, card effects).
- Benefits: scalability, modularity, easier AI and multiplayer extension.

### 2. **Functional Core, Imperative Shell**

- Keep game logic pure (functional transformations of state).
- Wrap with imperative shell for rendering, input, and side effects.
- Benefits: deterministic replay, easier testing, clean separation.

### 3. **Shader-Based Grid Rendering**

- Instead of geometry lines, render grid via distance-field shader.
- Benefits: crisp anti-aliased lines, resolution independence, simpler pipeline.

### 4. **Layered Rendering Pipeline**

- Separate passes: world → entities → effects → UI.
- Post-process AA (FXAA/SMAA) layered after scene render.
- Benefits: flexible visual stack, easier experimentation with effects.

### 5. **Cross-Platform Abstraction Layer**

- Abstract rendering/input so raylib is a backend, not a dependency.
- Benefits: easier to swap in SDL, WebGPU, or native APIs later.

**Decision**: This approach is being implemented. Tasks `T041`-`T043` in `tasks.md` define the work to create this abstraction layer, with an initial backend implementation using `raylib`.

### 6. **Simulation Harness**

- Build a headless simulation runner for CI/testing.
- Benefits: reproducible game state validation, automated regression tests.

### Procedural Assets as Optional Layer

- Treat procedural mesh generation and instanced placement as an optional library layer: reusable for demos/tests, but isolated from core simulation. Document defaults and keep them behind a thin API so they do not leak into game logic.

---

## Summary & Next Steps

The immediate focus is on completing the platform abstraction layer (`T041`-`T043`) and implementing core simulation features.

For future scalability, the project will evolve from the monolithic `Game` struct towards an **Entity-Component-System (ECS)** architecture. This will provide a more flexible and performant foundation for handling complex game logic, AI, and potentially multiplayer features.
