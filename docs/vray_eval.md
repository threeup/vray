# VRay Evaluation Summary

## Executive Summary

Turn/phase flow is readable and now logged, but the project is still demo-stage: no automated tests, weak validation/ownership, hardcoded config, and tight raylib coupling. Stabilization requires guardrails and seams before feature polish.

**Overall Grade: C (demo-ready, production-unready)**

## Scorecard

| Lens                     | Grade | Evidence                                                                  |
| ------------------------ | ----- | ------------------------------------------------------------------------- |
| Vision (Product/UX)      | C+    | Playable loop and reduced UI clutter; no persistence/help/error surfacing |
| Strategy (Architecture)  | C     | Platform split exists, but raylib is directly used everywhere; few seams  |
| Implement (Code Quality) | C     | Clear short code; sparse validation/RAII; hardcoded config; no tests      |
| Overall                  | **C** | Stabilization required before expanding scope                             |

## Strengths

- Turn/phase and subphase timing are explicit and logged ([src/boss.cpp#L251-L291](src/boss.cpp#L251-L291))
- UI plan panel hides in non-player phases; spawn setup is logged
- Codebase is small and readable; low algorithmic complexity
- Platform scaffold (window/input/renderer) is already factored

## Critical Gaps

- No automated tests or CI; regressions on render/input/turn paths are silent
- Implicit ownership and raw pointers in context ([src/app.h#L111-L124](src/app.h#L111-L124)); graphics handles lack RAII
- Sparse validation/guardrails in turn retries ([src/boss.cpp#L251-L291](src/boss.cpp#L251-L291))
- Hardcoded config (window/FPS/FOV) and camera defaults ([src/main.cpp#L120-L135](src/main.cpp#L120-L135))
- Direct raylib coupling blocks mocks/headless testing

## Risk Zones

| Area                  | Severity | Impact                                   | Evidence                                                                                        |
| --------------------- | -------- | ---------------------------------------- | ----------------------------------------------------------------------------------------------- |
| Missing tests/CI      | High     | Silent regressions in render/input/turn  | No tests in repo; manual-only verification                                                      |
| Ownership/RAII gaps   | High     | Lifetime/UB risk; leaks                  | Raw `game`/`boss` pointers and shader handles ([src/app.h#L111-L124](src/app.h#L111-L124))      |
| Validation/guardrails | Med-High | Stuck turns or hangs on bad plans/assets | Uncapped subphase retries and minimal checks ([src/boss.cpp#L251-L291](src/boss.cpp#L251-L291)) |
| Hardcoded config      | Medium   | Poor portability; user friction          | Window/FPS/FOV literals ([src/main.cpp#L120-L135](src/main.cpp#L120-L135))                      |
| Raylib coupling       | Medium   | Hard to test or swap backends            | Direct calls in render/input/UI modules                                                         |

## Action Items

| Item                                                          | Phase | Impact/Effort | Owner  | Link                                             |
| ------------------------------------------------------------- | ----- | ------------- | ------ | ------------------------------------------------ |
| Add smoke/unit tests for turn flow + utils; wire CI           | P0    | High / M      | Eng    | (repo)                                           |
| Guard play subphase retries and empty plans                   | P0    | High / S      | Eng    | [src/boss.cpp#L251-L291](src/boss.cpp#L251-L291) |
| Replace raw `game`/`boss` pointers with owned refs/unique_ptr | P0    | High / S      | Eng    | [src/app.h#L111-L124](src/app.h#L111-L124)       |
| Centralize config (window/FPS/FOV/camera) and load from file  | P1    | Med / S       | Eng    | [src/main.cpp#L120-L135](src/main.cpp#L120-L135) |
| Wrap shaders/models in RAII; add validity checks              | P1    | Med / S       | Eng    | [src/app.h#L76-L99](src/app.h#L76-L99)           |
| Insert render/input interfaces to decouple raylib             | P1    | Med / M       | Eng    | (repo)                                           |
| Add first-run help/persistence for settings                   | P2    | Med / M       | UX/Eng | (repo)                                           |

## Roadmap

- **P0 Stabilization**: Smoke tests for render/input/turn; guard subphase retries; basic error handling; ownership fixes (unique_ptr/RAII); structure logs.
- **P1 Hardening**: Config system (JSON defaults); render/input interfaces with raylib adapters; RAII for shaders/models; initial CI; camera/config constants centralized.
- **P2 Evolution**: Persistence for settings; onboarding/help; input remapping/accessibility; performance profiling hooks; richer logging/telemetry.
- **P3 Polish**: Broader test suite and CI gating; distribution pipeline; docs/support flow; performance tuning.
