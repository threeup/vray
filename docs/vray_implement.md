# Implementation Evaluation: VRay

## Executive Summary

Readable C++ and a clear turn controller, but validation/ownership are weak and automation absent. New play subphases and logging add logic without tests. Hardcoded config and direct raylib use keep reliability modest.

**Overall Grade: C**

### Key Strengths

- Core loops are short and readable; new phase/move logs aid debugging
- Turn/plan logic is explicit; phased play substeps are easy to trace
- Low algorithmic complexity; contained blast radius

### Critical Gaps

- Sparse validation and error handling on inputs/resources
- Implicit ownership (raw pointers) and missing RAII for graphics handles
- No automated tests on render/input/turn paths
- Hardcoded config/magic numbers and direct engine coupling

## 1. Code Correctness & Quality

### 1.1 Correctness & Logic

- Happy-path turn flow works; edge cases (blocked moves, retries) untested.
- Subphase retries rely on state mutations without guardrails.

**Grade: C+**

### 1.2 Validation & Error Paths

- Minimal bounds/null checks; asset/config failures likely crash/hang.
- Error propagation is ad-hoc; little user feedback.

**Grade: C-**

**Validation Gaps**
| Area | Input/Resource | Missing Checks | Severity | Fix Idea |
| ---- | -------------- | -------------- | -------- | -------- |
| Window init ([src/main.cpp](src/main.cpp)) | Width/height/FPS params | Bounds/config validation | Med | Load from config; clamp values |
| Turn execution ([src/boss.cpp](src/boss.cpp)) | Pending lists/subphase retries | Empty/null plans, max retries | Med | Guard on empty plans; cap retries/log errors |

### 1.3 Type Safety & Modern Practices

- Raw pointers for `game`/`boss` ([src/app.h#L120](src/app.h#L120)); no RAII for shaders/models.
- Const usage uneven; no `enum class` for EntityType.

**Grade: C-**

**Ownership / Const Issues**
| Symbol | Issue | Risk | Fix |
| ------ | ----- | ---- | --- |
| AppContext::game/boss | Raw pointers | Lifetime/UB | unique_ptr or ref with owner scope |
| RenderShaders handles | Plain Shader {0} | Leak/misuse | RAII wrapper + validity checks |
| UiState | Global mutable struct | Hard to reset/test | Provide reset ctor; narrow scope |

### 1.4 Code Quality & Readability

- Naming is clear; functions are short. Magic numbers persist (window size, FPS, camera FOV).

**Grade: C+**

**Magic Numbers Tally**
| Location | Value | Meaning | Replace With |
| -------- | ----- | ------- | ------------ |
| [src/main.cpp](src/main.cpp) | 800, 600, 60, 45.0f | Window size/FPS/FOV | Config constants file |
| [src/constants.h](src/constants.h) | Camera pitch/yaw/dist literals | Camera defaults | Central config |

### 1.5 Red Flags

- Direct raylib calls everywhere; no seams for testing.
- Subphase logic lacks tests and could mask blocked actions.

**Grade: C-**

## 2. Issues Found

| Category   | File                             | Lines | Issue                             | Sev  | Owner | ETA |
| ---------- | -------------------------------- | ----- | --------------------------------- | ---- | ----- | --- |
| Ownership  | [src/app.h#L120](src/app.h#L120) | 120   | Raw pointers for game/boss        | Med  | TBD   | P0  |
| Validation | [src/boss.cpp](src/boss.cpp)     | -     | Subphase retries lack guard/limit | Med  | TBD   | P0  |
| Config     | [src/main.cpp](src/main.cpp)     | -     | Hardcoded window/FPS/FOV          | Low  | TBD   | P1  |
| Coverage   | Project                          | -     | No tests on render/input/turn     | High | TBD   | P0  |

**Grade: C-**

## 3. Testing & Tooling

- No unit/integration/smoke tests; no sanitizers/CI visible.
- Logging exists but is unstructured; no assertions/guards.

**Grade: D+**

## 4. Summary & Recommendations

- **Top 3 Fixes**: Add validation/guards in turn/play; replace raw pointers with owned refs/unique_ptr; centralize config constants.
- **Safety/Robustness**: Add asserts/early returns; wrap shaders/models in RAII; structured logging for errors.
- **Quality/Readability**: Extract config constants; document play subphase intent; keep functions small.
- **Follow-up Tests**: Smoke tests for render/input/turn; unit tests for TurnPlan validation and boss subphase retries; regression for blocked-move retry.
