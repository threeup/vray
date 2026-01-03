# VRay Project Evaluation Workflow

Comprehensive multi-lens evaluation of the VRay raylib-based demo project across product experience, architecture/engineering, and low-level implementation quality.

## Overview

Use three evaluation templates to assess VRay from different stakeholder perspectives:

1. **vray_vision** (Experience/Product): Does it work as a playable product for users?
2. **vray_strategy** (Architecture/Engineering): Is the system design sound and maintainable?
3. **vray_implement** (Code Quality): Are implementation practices solid?

Each evaluation is self-contained, scored A-F, and includes detailed findings with actionable recommendations.

---

## Step 1: Fill Evaluation Reports

### Input Files

- [templ_vision.md](templ_vision.md) – Product/UX evaluation template
- [templ_strategy.md](templ_strategy.md) – Architecture/engineering template
- [templ_implement.md](templ_implement.md) – Code quality template
- VRay codebase in [src/](../src/) and [build/](../build/)

### Process

#### 1.1 Implementation Evaluation (vray_implement.md)

Refer to [templ_implement.md](templ_implement.md) for detailed section-by-section guidance.

**Fill in** [vray_implement.md](vray_implement.md) with:

- Correctness, validation, type safety, readability, and red-flag assessment.
- Validation Gaps, Ownership/Const Issues, and Magic Numbers tables.
- Issues Found table with file/line references and severity.
- Grade and recommendations related to coder implementation.

#### 1.2 Architecture Evaluation (vray_strategy.md)

Refer to [templ_strategy.md](templ_strategy.md) for detailed section-by-section guidance.

**Fill in** [vray_strategy.md](vray_strategy.md) with:

- Module organization, design patterns, state management, dependency assessment.
- State Inventory and Abstraction Seams inventory tables.
- Technical debt items with effort/priority; refactoring roadmap.
- Grade and recommendations related to architecture strategy.

#### 1.3 Experience Evaluation (vray_vision.md)

Refer to [templ_vision.md](templ_vision.md) for detailed section-by-section guidance.

**Fill in** [vray_vision.md](vray_vision.md) with:

- Feature completeness matrix, input/UX grades, error handling assessment.
- Adoption blockers and performance observations tables.
- Grade and recommendations related to vision.

---

## Step 2: Synthesize Summary Report

**Create** [vray_eval.md](vray_eval.md):

1. **Executive Summary** (paragraph): Integrate key findings from all three evals; overall project health assessment.
2. **Scorecard** (table): Overall grades for Vision, Strategy, Implement; highlight hot areas.
3. **Strengths** (bullet list): Consolidate key strengths from all three lenses.
4. **Critical Gaps** (bullet list): Consolidate blockers and high-risk items.
5. **Risk Zones** (table): Top 5 risk areas (e.g., no test coverage, missing error handling, globals) with severity, impact, and owner.
6. **Action Items** (table): Consolidate top fixes across all evals; prioritize by impact/effort; assign owners.
7. **Roadmap** (section): Phased plan (P0/P1/P2) to address critical issues; estimate effort; track dependencies.

---

## Step 3: Generate Task List

Update [tasks.md](tasks.md):

1. For a decent number of suggestions in vray_eval.md, create a task:

   - **Title**: Concise action (e.g., "Add smoke tests for render/input loop").
   - **Priority**: P0 (blocker), P1 (high), P2 (medium).
   - **Component**: Which area (implementation, architecture, UX).
   - **Description**: What and why (reference eval section); acceptance criteria.
   - **Estimated Effort**: T-shirt size (S/M/L/XL) or hours.
   - **Owner**: Who (if assigned); dependencies.
   - **Blocking**: What tasks it unblocks or depends on.

2. Group tasks by phase:

   - **Phase 0 (Stabilization)**: Critical fixes (crashes, error handling, basic validation).
   - **Phase 1 (Hardening)**: Test coverage, logging, config centralization.
   - **Phase 2 (Evolution)**: Refactoring, DI seams, seam insertion, code reuse extraction.
   - **Phase 3 (Polish)**: Onboarding, accessibility, performance tuning.

3. Track dependencies: e.g., "DI seams → easier mocking → unit tests".

---

## Notes for the AI Agent

- **Scope**: Focus on the codebase as-is; do not assume changes already made.
- **Specificity**: Back assertions with file paths and line numbers where possible (e.g., "src/main.cpp:42" for a global).
- **Pragmatism**: Grades should be realistic (C/C+ is fine for a demo); recommendations should be incremental and achievable.
- **Interdependence**: Some fixes unlock others (e.g., reducing globals → easier testing → faster regression checks).
- **Evidence**: Cite examples: "render() function is ~200 lines with 5 levels of nesting" rather than just "long functions."
- **Ownership**: Assign owners where possible (current devs, teams). Mark unknown/orphaned code.
- **Timeline**: Estimate effort for each task; track blockers and critical path.

---

## Files

- **Templates**: [templ_vision.md](templ_vision.md), [templ_strategy.md](templ_strategy.md), [templ_implement.md](templ_implement.md)
- **Filled Evals**: [vray_vision.md](vray_vision.md), [vray_strategy.md](vray_strategy.md), [vray_implement.md](vray_implement.md)
- **Summary**: [vray_eval.md](vray_eval.md) (to be created)
- **Tasks**: [tasks.md](tasks.md) (to be created/updated)
