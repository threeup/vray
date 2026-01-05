# Architecture — Vray Demo

This document describes the actual architecture of the Vray Demo project: a C++ card-sequence turn-based tactical game with deterministic grid-based combat.

## Overview

**Vray** is a turn-based card game where:

- **Players** (human + AI) assign cards to mechs during the **PlayerSelect phase**
- **AI** builds its plan during **NpcSelect phase**
- **Both plans execute simultaneously** during the **Play phase** with deterministic card effects on a grid-based world
- **Rendering** uses Raylib with procedural meshes and a custom UI panel system

**Tech Stack**:

- C++20 (std::vector, std::unique_ptr, ranges)
- Raylib 5.6 (graphics, input, windowing)
- CMake (build system)
- Deterministic simulation with pure card effect functions

---

## Core Systems

### 1. Game Simulation (`src/game.h/cpp`)

The `Game` struct owns:

- **Grid**: 12×12 spatial substrate for entity placement
- **Entities**: Player mechs (IDs 1-3), enemy mechs (IDs 4-6), objects
- **Hand**: Available cards for the player, usage tracking
- **Deck**: Card pool for drawing (T_051)
- **CurrentPlan**: Active card-to-mech assignments
- **Turn Counter**: Track round progression

**Key Functions**:

- `begin_turn()`: Reset hand usage flags for new turn
- `init_game()`: Seed initial player/enemy/object placement
- `update_game()`: Per-frame simulation (rotations, etc.)
- `handle_ui_actions()`: Process UI intents without auto-resolve
- `handle_input()`: Process keyboard/mouse input

---

### 2. Turn Phase Manager (`src/boss.h/cpp`)

The `Boss` class orchestrates the three-phase turn cycle:

```
PlayerSelect → NpcSelect → Play → PlayerSelect (repeat)
```

**Phases**:

- **Phase::PlayerSelect** (0):

  - Player assigns cards to mechs via card UI drag-drop
  - Player clicks OK to lock their plan
  - UI shows: phase bar, deck panel, game board, mech slots, hand

- **Phase::NpcSelect** (1):

  - Boss generates random NPC plan (via `buildNpcPlan()`)
  - No player input allowed
  - UI shows: only phase bar and hand (other panels hidden)

- **Phase::Play** (2):
  - Both player and NPC plans execute simultaneously
  - Cards apply effects to entities (movement, damage, heal)
  - Effects resolved subphase-by-subphase
  - UI shows: only phase bar and hand

**State Management**:

- `playerPlan_`: Locked plan from PlayerSelect
- `npcPlan_`: Generated plan during NpcSelect
- `pendingPlayer_`, `pendingNpc_`: Cards to execute
- `playSubphase_`: Current execution slot

---

### 3. Card System (`src/card.h/cpp`)

**Card Types**:

- `CardType::Move`: Directional movement (forward/lateral)
- `CardType::Damage`: Direct damage to target
- `CardType::Heal`: Health restoration

**Plan Structure**:

```cpp
struct PlanAssignment {
    int mechId;        // Target mech ID
    int cardId;        // Card ID
    bool useMirror;    // Mirror direction flag
};

struct TurnPlan {
    std::vector<PlanAssignment> assignments;
    bool validate(...);  // Verify plan is legal
};
```

**Card Execution**:

- Pure functions: `(GameState, Card, Context) → GameStateDelta`
- `applyCard()`: Apply single card effect
- Deterministic: same seed + same plan = same result

---

### 4. UI System (`src/ui/cardui/`)

**New Card UI System** (T_050-T_059):

**Layout** (`GameUIPanel`):

```
┌─────────────────────────────────────┐  ← Phase Bar (20px, always visible)
├──────────────┬──────────────────────┤
│   Deck       │    Game Board        │  ← Top Row (48px, PlayerSelect only)
│ (50% width)  │    (50% width)       │
├──────────────────────────────────────┤
│        Mech Slot Container           │  ← Mech Row (160px, PlayerSelect only)
│   [Mech 1]  [Mech 2]  [Mech 3] [OK] │    - 3 mech circles (80px)
│   [Slot 1]  [Slot 2]  [Slot 3]      │    - 3 card slots (120×160px)
│                              (40px)  │    - OK button (40×160px, right edge)
├──────────────────────────────────────┤
│           Hand Panel                 │  ← Hand (120px, always visible)
│  [Draggable Cards]                   │
└──────────────────────────────────────┘
```

**Components**:

- **GameUIPanel** (`game_ui_panel.h`):

  - Root layout container managing 5 panels
  - `computeLayout()`: Calculate positions based on window size
  - `showMechRow`: Visibility flag (hidden after OK or during non-PlayerSelect)

- **DeckPanel** (`deck_panel.h`):

  - Shows remaining deck count
  - Draw button to add cards to hand

- **HandPanel** (`hand_panel.cpp`):

  - Displays draggable cards with color coding by type
  - Handles card hover/selection state
  - Shows card tooltips on hover

- **MechSlotContainer** (`mech_slot_container.cpp`):

  - 3 mech circles (Alpha=red, Bravo=blue, Charlie=green)
  - 3 card drop zones (accept dragged cards)
  - Hover highlight (yellow outline) on valid drop target
  - Mech stat display (HP, bonuses)
  - **OK button** (right edge): validates plan, triggers phase advance

- **GameBoardPanel** (`game_board_panel.cpp`):
  - Placeholder for future combat visualization
  - Shows team/enemy status

**Drag-Drop Mechanics** (T_054):

- Rectangle-based collision detection (100×140px card bounding box)
- `CheckCollisionRecs()`: Detect card-to-slot overlap
- Hover state updates `drag.hoverSlotIndex`
- Release: `update_cardui_drop()` validates and assigns card or bounces it back

**Card Removal**:

- Red X button on each mech slot removes assigned card
- Card returned to hand via `game.hand.unmarkUsed()`

---

### 5. Entity & Grid System (`src/entity.h`, `src/grid.h`)

**Entity**:

```cpp
struct Entity {
    int id;
    Vector2 position;
    EntityType type;  // PLAYER, ENEMY, OBJECT
    float health;
};
```

**Grid**:

- 12×12 grid (configurable)
- `getOccupant()`: Check occupancy
- Movement collision detection
- Entity placement validation

---

## Data Flow

### Turn Execution Sequence

```
Main Loop (main.cpp)
  ↓
[Phase 0: PlayerSelect]
  draw_cardui()              ← Render phase bar, deck, board, mech slots, hand
  update_cardui_drop()       ← Check card releases, validate & assign
  HandPanel_UpdateDrag()     ← Clear drag state
  boss.processUi()           ← Check if actions.playSequence set
    if valid plan → boss.enterPhase(NpcSelect)
  ↓
[Phase 1: NpcSelect]
  boss.update()              ← Time-based phase transitions
    buildNpcPlan()           ← Random mech-card assignments
    validatePlan()
    enterPhase(Play)
  ↓
[Phase 2: Play]
  boss.update()              ← Subphase-based execution
    runPlaySubphase()        ← Apply assignments in order
      applyAssignment()
        applyCard()          ← Movement, damage, heal effects
      SyncWorldActorsFromGame()  ← Update 3D world
    when all subphases done → finishRound() → enterPhase(PlayerSelect)
  ↓
[Loop back to Phase 0]
```

---

## File Organization

```
src/
├── main.cpp                 ← Main loop orchestrator
├── app.h/cpp                ← AppContext, platform initialization
├── boss.h/cpp               ← Phase manager (PlayerSelect/NpcSelect/Play)
├── game.h/cpp               ← Game state, turn management
├── card.h/cpp               ← Card definitions, effects
├── entity.h                 ← Entity struct
├── grid.h/cpp               ← Grid spatial queries
├── ui.h/cpp                 ← UiActions, MechStats structs
│
├── ui/cardui/               ← NEW Card UI System (T_050-T_059)
│   ├── cardui.h/cpp         ← Main orchestrator
│   ├── game_ui_panel.h      ← Layout container
│   ├── deck_panel.h         ← Deck display
│   ├── hand_panel.h/cpp     ← Hand display, draggable cards
│   ├── game_board_panel.h/cpp
│   └── mech_slot_container.h/cpp  ← Mech circles, card slots, OK button
│
├── platform/                ← Platform abstraction (raylib backend)
│   ├── platform.h/cpp
│   ├── raylib_window.h/cpp
│   ├── raylib_input.h/cpp
│   ├── raylib_renderer.h/cpp
│   └── renderer_interface.h
│
├── world/                   ← 3D world simulation
│   └── world.h/cpp
│
└── utils/                   ← Helper utilities
    ├── meshGenerateUtils.h/cpp
    ├── meshMech.h/cpp
    ├── luaUtils.h/cpp
    └── ...
```

---

## Key Design Decisions

### 1. **Deterministic Simulation**

Card effects are pure functions:

```cpp
GameState after = applyCard(before, card, mechId, useMirror);
```

**Benefits**:

- Replay-able: same seed + same plan = same result
- Testable: unit tests validate effects
- Network-ready: send plans, not full state

### 2. **Phase-Based Turn System**

Explicit phases (PlayerSelect → NpcSelect → Play) provide:

- **UI Clarity**: Player knows what's happening
- **Determinism**: Clear execution order
- **Timing**: Time-based transitions (not input-driven)

### 3. **Immutable Layout**

`GameUIPanel` layout is computed fresh each frame:

- Responsive to window resize
- Panels scale intelligently
- No state mutation in rendering

### 4. **Drag-Drop via Collision**

Rectangle-to-rectangle collision detection (not point-to-point):

- Tolerant to slight misses
- Natural "hover" feedback (yellow outline)
- Card snaps to slot on release

---

## UI Action Flow

```
User Input
  ↓
draw_cardui()              ← Renders UI, captures clicks
  ├─ draw_phase_indicator()
  ├─ DeckPanel_Draw()      ← Draw button checks mouseClick
  ├─ HandPanel_Draw()      ← Card hover/drag initiation
  ├─ MechSlotContainer_Draw()
  │   └─ OK button check → sets actions.playSequence
  └─ GameBoardPanel_Draw()
  ↓
update_cardui_drop()       ← Process card releases
  ├─ Check collision (drag rect vs slot rects)
  ├─ Validate assignment
  └─ Update game.currentPlan
  ↓
HandPanel_UpdateDrag()     ← Update drag state, clear on release
  ↓
boss.processUi(actions)    ← Check actions.playSequence
  └─ If valid plan → enter NpcSelect phase
```

---

## State Management

**Mutable State**:

- `Game::currentPlan` - Active card assignments
- `Game::hand` - Card usage flags
- `Boss::phase_` - Current phase
- `DragState` - Drag-drop tracking
- `GameUIPanel::showMechRow` - Mech row visibility

**Immutable/Pure**:

- Card effects (applyCard)
- Plan validation
- Entity updates (new GameState returned, old not modified)

---

## Testing & Validation

**Unit Tests** (`tests/`):

- Card logic tests (T_063: card_logic_tests.cpp)
- Plan validation
- Grid collision
- Mech stat calculations (T_057)

**Smoke Tests**:

- Full turn cycle execution
- Phase transitions
- UI rendering (regression visual tests)

---

## Future Extensions

### Planned (T_060-T_066)

- **Animations**: Card play/assignment feedback, card movement on grid
- **Visual Polish**: Particle effects, sound cues, screen shake
- **Testing**: Full game state validation tests

### Potential

- **ECS Migration**: Move to Entity-Component-System for complex entities
- **Networking**: Send plans to server, receive opponent's plan
- **AI Difficulty**: Parameterized NPC strategy
- **Procedural Generation**: Map/card pool generation

---

## Performance Considerations

- **Grid Queries**: O(1) occupancy check via hash
- **Card Effects**: Instant (no animation) for determinism
- **Rendering**:
  - Procedural meshes cached
  - Raylib batching handles many entities
  - FXAA post-process on render target
- **Dragging**: Per-frame collision vs 3 slots (negligible cost)
