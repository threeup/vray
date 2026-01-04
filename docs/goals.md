# VRay Creative Goals

High-level implementation objectives focused on core game and rendering work.

---

### G_C01: Card Tactics Core

Introduce the 3-mech, 6-card-per-player tactical loop with mirroring rules (lateral swap; forward-only flips to backward). Support deterministic per-mech assignment of one card per turn with validation.

### G_C02: Card UX & AI Baseline

Deliver minimal UI for selecting/mirroring cards and assigning to mechs; add random AI card selection per mech; ensure card flow integrates with turn resolution and can be tested headlessly.

---

### G_001: Build Deterministic Turn-Based Game Loop

Implement core game simulation: turn execution, card sequencing, AI decision-making, and state validation. Game logic must be testable and reproducible independent of rendering.

### G_002: Establish Rendering Pipeline & Effects

Render the game world with clean visuals: camera control, entity rendering, lighting, and basic effects (bloom, FXAA). Rendering must be abstracted from game logic via clean interfaces.

### G_003: Decouple from RayLib

Replace direct raylib calls with platform-agnostic interfaces (RenderBackend, InputHandler). This enables testing, portability, and future renderer swaps without touching game code.

### G_004: Crash-Free & Error Handling

All errors surface gracefully with helpful messages. No silent failures or undefined behavior. Players understand when something goes wrong and can recover.

### G_005: Expand Game Mechanics & Content

Add more card types, enemies, levels, and gameplay variety. Mechanics are extensible and balance is testable via AI vs. AI automation.

### G_006: Visual Quality & Performance

Smooth 60 FPS on target hardware. Procedural meshes, instancing, distance-field rendering, layered pipeline. Visual effects feel intentional and hardware-aware.

### G_007: AI Players & Deterministic Replay

AI generates and executes card sequences with personality. Turn execution is deterministic, supporting replay, debugging, and testing. Foundation for future multiplayer.

### G_008: Headless Testing Infrastructure

Game runs and validates itself without graphics window. AI vs. AI matches execute in CI for regression detection. Deterministic execution captures replays for debugging.

### G_009: ECS Framework & Core Refactoring

Replace monolithic Game struct with ECS. Functional Core + Imperative Shell separates pure logic from I/O. Architecture supports 10x growth without rewrite.

### G_010: Player Preferences & Accessibility

Player settings (camera angles, control bindings) persist. Input remapping, high-contrast mode, hold/toggle options support diverse players and hardware profiles.

### G_011: Documentation & Support

In-game help, user manual, and support channel. Player feedback shapes future updates. Studio builds reputation for listening and iteration.

### G_012: Deployment & Automation

Build/deploy automation, full test suite, CI/CD pipeline. Version management, update delivery, and performance tracking.
