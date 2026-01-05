# Engineering Evaluation: VRay

## Executive Summary

Architecture remains demo-grade: clear module split but tight raylib coupling, implicit ownership, and no DI or config/logging seams. Recent boss/play changes add complexity without added tests. Testability and observability are the main risks.

**Overall Grade: C**

### Key Strengths

- Visible separation (platform/render/game/ui) keeps code discoverable
- Scope small; debt surface area still containable
- Recent logging hooks (phase/move) improve diagnosability
- Build remains stable on raylib baseline

### Critical Gaps

- No abstraction seams over raylib (render/input/assets); hard to test or swap
- Raw pointers/implicit lifetimes (AppContext game/boss) risk UB
- Config/logging are ad-hoc; no centralized config or structured logs
- No DI pattern; wiring is manual; state resets are manual

## 1. Module Organization & Structure

### 1.1 Directory Layout & Cohesion

- Platform/render/game/ui directories map to domains; utils thin.
- Some files mix orchestration and state (e.g., boss blends scheduling + execution).

**Grade: B-**

### 1.2 Layer Separation

- Layers exist conceptually, but cross-cutting concerns (error/log/config) are inlined.
- Build targets follow demo layout, not layered artifacts.

**Grade: C**

## 2. Design Patterns & Trade-offs

### 2.1 Pattern Usage & Appropriateness

- Minimal patterns; recent phase subphases are hand-rolled state machine.
- Low complexity, but missing guardrails/tests.

**Grade: C+**

### 2.2 Separation of Concerns & Boundaries

- Rendering/input are directly consumed by game logic; little inversion.
- Data flow is imperative; no eventing/commands.

**Grade: C**

## 3. State Management & Instantiation

### 3.1 State Strategy

- Raw pointers in context ([src/app.h#L120](src/app.h#L120)) for `game`/`boss`; unclear ownership.
- RAII missing for shaders/models; default-inited handles risk misuse.
- UI state is mutable/global within `AppContext`.

**Grade: C-**

**State Inventory**
| Area | Current Pattern | Risk | Suggested Change |
| ---- | ---------------- | ---- | ---------------- |
| Game/boss pointers | Raw in AppContext | Lifetime/UB | unique_ptr or references with owner scope |
| Render handles | Plain structs default-init | Resource leaks/misuse | RAII wrappers + validity checks |
| Config | Hardcoded in main/render | No overrides/portability | Central config loader |
| UI flags | Global mutable struct | Hard to reset/test | Encapsulate + reset hooks |

### 3.2 State Flow & Concurrency

- Single-threaded; mutations scattered across boss/game/ui.
- Reset between rounds is manual; test harness would be brittle.

**Grade: C-**

## 4. Testability & Dependency Management

### 4.1 Abstraction Seams

- Raylib calls are direct ([src/render.cpp](src/render.cpp)), blocking mocks/backends.
- Input polling is direct; no handler interface.

**Grade: D+**

**Abstraction Seams Inventory**
| Area | Current Pattern | Risk | Seam/Refactor |
| ---- | ---------------- | ---- | ------------- |
| Rendering | Direct raylib draw/RT ops | Tight coupling, untestable | RenderBackend + adapter |
| Input | Direct key polling | Cannot fake in tests | InputHandler interface |
| Assets | Direct LoadModel/Shader | Crash on missing assets | AssetLoader facade + fallbacks |
| Window lifecycle | Platform owns, context holds refs | Ownership unclear | Document/encapsulate lifetimes |

### 4.2 Component Isolation & DI

- No DI; globals/references passed around; difficult to isolate game/boss for tests.

**Grade: D**

### 4.3 Global/Hardcoded Dependencies

- Hardcoded window/FPS/camera values in [src/main.cpp](src/main.cpp); no config.

**Grade: C-**

## 5. Code Reuse, Boundaries, and Duplication

- Utilities are minimal; shared math/asset helpers absent; risk of repetition as scope grows.

**Grade: C**

## 6. Observability & Operability

- Logging improved for phases/moves, but no structured levels/rotation; no metrics/profiling hooks.

**Grade: D+**

## 7. Technical Debt & Roadmap

### 7.1 Identified Debt Items

| Item                        | Type   | Impact | Unlock                   | Effort | Priority |
| --------------------------- | ------ | ------ | ------------------------ | ------ | -------- |
| Raw pointers in AppContext  | Design | Med    | Ownership clarity, tests | Low    | High     |
| No render/input abstraction | Design | High   | Mocking/backends         | Med    | High     |
| Hardcoded window/FPS/config | Infra  | Med    | Portability/config       | Low    | Med      |
| No asset/error handling     | Design | High   | Stability, UX            | Med    | High     |
| No shader/model RAII        | Design | Med    | Cleanup safety           | Low    | Med      |

**Grade: C**

### 7.2 Strategic Limitations

- Raylib lock-in acceptable for demo; costly to swap without seams.
- No config/logging infra; scaling to production would need refactor.

**Grade: C**

### 7.3 Refactoring Roadmap

- Near: Add render/input interfaces; wrap resources in RAII; basic logging/config loader.
- Medium: Constructor injection for game/boss; split orchestration from state; asset loader with fallbacks.
- Long: Event/command bus for game actions; profiling/telemetry hooks; module boundary hardening.

**Grade: C+**

## 8. Summary & Recommendations

- Structural health: Adequate; testability: Poor; maintainability: Medium-Low; debt: Manageable.
- Priorities: Introduce seams (render/input/assets), fix ownership (unique_ptr/RAII), centralize config/logging, and add basic profiling hooks.
