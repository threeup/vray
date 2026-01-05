# VRay Development Tasks (Tech Lead)

**Current Focus**: UI Implementation for Card Deck Interaction — Make the card selection gameplay fun and intuitive.

---

## Legend

- **Status**: Planned | In Progress | Done | Blocked | Superseded
- **Effort**: XS (0.5d) | S (1-2d) | M (3-5d) | L (>5d)
- **Depends**: hard prerequisites; **Related**: helpful but not blocking

---

## Current Sprint: UI Implementation (7-11 Tasks)

### T_050: GameUIPanel Root Layout Container

**Priority**: P1 | **Component**: UI | **Status**: Done | **Effort**: S

- Create GameUIPanel as root container managing viewport/screen coordinate mapping.
- Layout 5 sub-regions: DeckPanel (top-right), GameBoardPanel (center), MechSlotContainer (bottom-center), HandPanel (bottom).
- Acceptance: Panel resizes with window; all sub-panels positioned correctly.

#### Implementation

- **File**: `src/ui.h` - Add `struct GameUIPanel` with `Rectangle` regions for each sub-panel plus layout helper methods.
- **File**: `src/ui.cpp` - Implement `GameUIPanel::computeLayout(int winW, int winH)` that calculates all sub-regions:
  - `deckRect`: top-right, fixed 140x80 size, 20px margin from edges.
  - `gameBoardRect`: center area, spans from below phase bar to above mech slots (dynamically sized).
  - `mechSlotRect`: bottom-center band, 160px height, full width minus margins.
  - `handRect`: very bottom, 120px height, full width minus margins.
  - `phaseRect`: top bar (existing), 34px height.
- **Integration**: Refactor `UI_Draw()` to use `GameUIPanel` computed rects instead of inline calculations.
- **Helper**: Add `struct PanelMetrics { float margin, padding, cornerRadius; }` for consistent styling.
- **Test**: Add unit test verifying layout proportions remain valid at different window sizes (800x600, 1920x1080).

### T_051: DeckPanel (Top-Right Deck Display)

**Priority**: P1 | **Component**: UI | **Status**: Done | **Effort**: S

- Display remaining card count in top-right corner.
- Implement clickable draw button to draw cards from deck.
- Show visual card stack indicator.
- Acceptance: Deck count updates on draw; click to draw works; visual feedback on interaction.

#### Implementation

- **File**: `src/ui.h` - Add `struct DeckPanel` with state: `int deckSize`, `bool isHovered`, `bool drawButtonPressed`.
- **File**: `src/card.h` - Add `struct Deck { std::vector<Card> cards; Card draw(); int remaining() const; void shuffle(uint32_t seed); }`.
- **File**: `src/game.h` - Add `Deck deck` member to `Game` struct (separate from `Hand`).
- **File**: `src/ui.cpp` - Implement `DeckPanel_Draw(Rectangle deckRect, Game& game, UiActions& actions)`:
  - Draw rounded rectangle background with subtle shadow.
  - Draw stacked card visual (3-4 offset rectangles to simulate depth).
  - Display "DECK: N" text centered.
  - Add `GuiButton()` for "Draw" action; on click set `actions.drawCard = true`.
  - Highlight border on hover using `CheckCollisionPointRec(GetMousePosition(), deckRect)`.
- **File**: `src/game.cpp` - Implement `Deck::draw()` that pops a card from deck and adds to `hand.cards`.
- **UiActions**: Add `bool drawCard = false` field.
- **Test**: Unit test for `Deck::draw()` reducing count; UI test for button click triggering action.

### T_052: HandPanel with Drag-Enabled PlayableCards

**Priority**: P1 | **Component**: UI | **Status**: Done | **Effort**: M

- Create HandPanel with darker rounded rectangle background.
- Implement PlayableCard components that display card art, stats (power, effect).
- Enable drag-and-drop from hand; visual feedback on hover/selected.
- Acceptance: Cards draggable; hover highlights; drag originates from hand panel.

#### Implementation

- **File**: `src/ui.h` - Add drag state structs:
  ```cpp
  struct DragState {
      bool isDragging = false;
      int draggedCardId = -1;
      Vector2 dragOffset = {0, 0};
      Vector2 currentPos = {0, 0};
  };
  struct PlayableCardUI {
      Rectangle bounds;
      int cardId;
      bool isHovered;
      bool isSelected;
  };
  ```
- **File**: `src/ui.cpp` - Implement `HandPanel_Draw(Rectangle handRect, Game& game, DragState& drag, UiActions& actions)`:
  - Draw darker rounded rectangle background (`Color{40, 40, 50, 220}`).
  - Calculate card layout: `cardWidth = 100`, `cardHeight = 140`, `spacing = 12`, horizontal centering.
  - For each card in `game.hand.cards`:
    - Draw card rectangle with border; show `card.name`, `cardTypeToString(card.type)`, and effect stats.
    - Highlight on hover (brighter border, slight scale).
    - On mouse down + drag: set `drag.isDragging = true`, `drag.draggedCardId = card.id`.
  - While dragging: draw card at `drag.currentPos` with semi-transparency.
- **File**: `src/game.h` - Add `DragState dragState` to `Game` or `UiState`.
- **Input handling**: In `UI_Draw()`, update `drag.currentPos = GetMousePosition() - drag.dragOffset`.
- **Test**: Verify drag initiates on left-click+move; verify card follows cursor.

### T_053: MechSlotContainer with 3 Mech+Card Pairs

**Priority**: P1 | **Component**: UI | **Status**: Done | **Effort**: M

- Create MechSlotContainer managing 3 horizontal mech+card pairs.
- Implement MechCircle (visual circle, color-coded by variant Alpha/Bravo/Charlie).
- Implement CardSlotPanel (darker rounded rectangle) adjacent to each mech.
- Each slot is a drop target zone with hover highlight.
- Acceptance: 3 pairs layout correctly; hover highlights drop zones; positions update on resize.

#### Implementation

- **File**: `src/ui.h` - Add structs:
  ```cpp
  enum class MechVariant { Alpha, Bravo, Charlie };
  struct MechSlotUI {
      int mechId;
      MechVariant variant;
      Rectangle circleRect;   // bounding box for mech circle
      Rectangle cardSlotRect; // adjacent card slot drop zone
      bool isDropTarget;      // true when card dragged over
      int assignedCardId;     // -1 if empty
      bool useMirror;
  };
  struct MechSlotContainer {
      MechSlotUI slots[3];
      void computeLayout(Rectangle containerRect);
  };
  ```
- **File**: `src/ui.cpp` - Implement `MechSlotContainer::computeLayout()`:
  - Divide `containerRect` into 3 equal sections with 16px gaps.
  - Each section: mech circle (80px diameter) on left, card slot (120x160px) on right.
  - Store computed rects in `slots[i].circleRect` and `slots[i].cardSlotRect`.
- **File**: `src/ui.cpp` - Implement `MechSlotContainer_Draw()`:
  - For each slot, draw mech circle with faction color from `PALETTE_RED/BLUE/GREEN` based on variant.
  - Draw card slot as darker rounded rect; if `assignedCardId != -1`, draw card preview.
  - If `isDropTarget` (detected via collision with drag position), draw highlight border.
- **Mapping**: Link `slots[i].mechId` to player mech entity IDs from `game.entities`.
- **Test**: Layout test for 3 slots; drop zone detection test with mock drag position.

### T_054: Drag-and-Drop Card Placement

**Priority**: P1 | **Component**: UI | **Status**: Done | **Effort**: M

- Implement drag-and-drop logic: drag card from hand to CardSlotPanel.
- Visual feedback: slot highlights when dragging over, cards snap into place or bounce back.
- Update CardSlotPanel to display equipped cards/icons.
- Acceptance: Drop success updates slot; invalid drops rejected; visual feedback works.

#### Implementation

- **File**: `src/ui.h` - Extend `DragState`:
  ```cpp
  int hoverSlotIndex = -1;  // which MechSlotUI is under cursor
  bool dropValid = false;    // true if slot accepts this card
  ```
- **File**: `src/ui.cpp` - Implement drop logic in `UI_Draw()`:
  - During drag, check collision with each `slots[i].cardSlotRect`.
  - Set `drag.hoverSlotIndex = i` and `slots[i].isDropTarget = true` for visual feedback.
  - On mouse release while dragging:
    - If `hoverSlotIndex != -1`: create `PlanAssignment{mechId, draggedCardId, game.mirrorNext}` and add to `game.currentPlan.assignments`.
    - Mark card as used via `game.hand.markUsed(draggedCardId)`.
    - Reset `drag.isDragging = false`.
    - If drop outside valid slot: animate bounce-back (lerp card position to origin over 0.2s).
- **File**: `src/ui.cpp` - Snap animation: store `cardOriginPos` when drag starts; on invalid drop, animate return.
- **Validation**: Before accepting drop, check `game.hand.canPlay(cardId)` and slot not already occupied.
- **Visual**: On successful drop, flash slot green briefly (0.15s highlight fade).
- **Test**: Integration test for drag-drop updating `currentPlan`; rejection test for occupied slots.

### T_055: Mirror Button & Mirroring Interaction

**Priority**: P1 | **Component**: UI | **Status**: Done | **Effort**: S

- Add Mirror button/icon to CardSlotPanel when a card is placed.
- Clicking mirror triggers card flip/duplication effect.
- Show visual indication of mirrored state (flipped appearance or badge).
- Acceptance: Mirror button appears only when card placed; click toggles mirror state; visual feedback clear.

#### Implementation

- **File**: `src/ui.cpp` - In `MechSlotContainer_Draw()`, for each slot with `assignedCardId != -1`:
  - Draw a small mirror icon button (20x20px) at top-right corner of `cardSlotRect`.
  - Use raygui: `if (GuiButton(mirrorBtnRect, "M")) { toggleMirror(slotIndex); }`.
  - On click: find assignment in `game.currentPlan.assignments` where `mechId == slots[i].mechId`, toggle `useMirror`.
- **Visual feedback**:
  - If `useMirror == true`: draw card slot with horizontal flip transform or overlay "(M)" badge.
  - Add subtle flip animation (0.2s scale x from 1→-1→1 with color flash).
- **Card effect**: Reference `card.mirroredEffect` (already exists in `Card` struct) for display.
- **UiActions**: Add `int toggleMirrorSlot = -1` to signal mirror toggle from UI.
- **File**: `src/game.cpp` - In `handle_ui_actions()`, handle `actions.toggleMirrorSlot`:
  ```cpp
  for (auto& a : game.currentPlan.assignments) {
      if (a.mechId == actions.toggleMirrorSlot) {
          a.useMirror = !a.useMirror;
          break;
      }
  }
  ```
- **Test**: Unit test for toggle behavior; visual test confirming badge appears.

### T_056: GameBoardPanel (Center Arena Display)

**Priority**: P1 | **Component**: UI | **Status**: Done | **Effort**: M

- Render enemy mech area (left side) and your mech area (right side).
- Display game state information (health, effects, modifiers).
- Placeholder for combat visualization.
- Acceptance: Both mech areas render; state displays update; ready for animation integration.

#### Implementation

- **File**: `src/ui.h` - Add:
  ```cpp
  struct GameBoardPanel {
      Rectangle enemyArea;   // left half
      Rectangle playerArea;  // right half
      Rectangle centerLine;  // divider
      void computeLayout(Rectangle boardRect);
  };
  struct MechBoardDisplay {
      int entityId;
      Vector2 position;
      int health;
      std::vector<std::string> activeEffects;
  };
  ```
- **File**: `src/ui.cpp` - Implement `GameBoardPanel_Draw(Rectangle rect, Game& game)`:
  - Split rect into left (enemy) and right (player) halves with 4px center divider.
  - For enemy mechs (`type == ENEMY`): draw mech representation (colored circle/icon) with health bar below.
  - For player mechs (`type == PLAYER`): same display, mirrored layout.
  - Health bar: `DrawRectangle()` with green fill proportional to `entity.health / 100`.
  - Effect badges: small icons or text labels for active modifiers.
- **Combat placeholder**: Draw "COMBAT AREA" text in center; reserve space for future animation layer.
- **Data binding**: Loop `game.entities`, filter by `EntityType`, extract `health` and `position`.
- **Styling**: Use faction palettes (`PALETTE_RED` for enemy, `PALETTE_BLUE` for player).
- **Test**: Render test with mock entities; verify health bar updates on entity damage.

### T_057: MechStatDisplay Integration

**Priority**: P2 | **Component**: UI | **Status**: Done | **Effort**: S

- Show mech health, abilities, and equipped card synergies in stat panel.
- Update stats when cards are added/removed/mirrored.
- Acceptance: Stats update in real-time as cards are placed; synergy calculations visible.

#### Implementation

- **File**: `src/ui.h` - Add:
  ```cpp
  struct MechStats {
      int baseHealth;
      int currentHealth;
      int attackBonus;
      int defenseBonus;
      std::string synergyText;  // e.g., "Move+Damage Combo: +10 ATK"
  };
  MechStats calculateMechStats(int mechId, const Game& game);
  ```
- **File**: `src/card.cpp` - Implement synergy calculation:
  ```cpp
  MechStats calculateMechStats(int mechId, const Game& game) {
      MechStats stats = {100, 100, 0, 0, ""};
      const Entity* mech = findEntityById(game.entities, mechId);
      if (mech) stats.currentHealth = mech->health;
      for (const auto& a : game.currentPlan.assignments) {
          if (a.mechId != mechId) continue;
          const Card* c = findCardById(game.hand.cards, a.cardId);
          if (!c) continue;
          const CardEffect& eff = a.useMirror ? c->mirroredEffect : c->effect;
          if (eff.type == CardType::Damage) stats.attackBonus += eff.damage;
          if (eff.type == CardType::Heal) stats.defenseBonus += eff.heal;
      }
      if (stats.attackBonus > 0 && stats.defenseBonus > 0)
          stats.synergyText = "Balanced: +" + std::to_string(stats.attackBonus + stats.defenseBonus);
      return stats;
  }
  ```
- **File**: `src/ui.cpp` - In `MechSlotContainer_Draw()`, below each mech circle:
  - Call `calculateMechStats(slots[i].mechId, game)`.
  - Draw compact stat block: `HP: 85 | ATK: +5 | DEF: +3`.
  - If `synergyText` non-empty, draw in accent color below.
- **Reactivity**: Stats recalculate each frame based on `currentPlan` state.
- **Test**: Unit test for `calculateMechStats()` with various card combinations.

### T_058: Card Interaction Feedback & Hover Tooltips

**Priority**: P2 | **Component**: UI | **Status**: Done | **Effort**: S

- Implement CardHoverTooltip showing full card details, effects, power calculations.
- Preview stat changes when card is dragged near a mech slot.
- Acceptance: Tooltip appears on hover; preview shows during drag; disappears on drop.

#### Implementation

- **File**: `src/ui.h` - Add:
  ```cpp
  struct CardTooltip {
      bool visible = false;
      int cardId = -1;
      Vector2 position;
      float showDelay = 0.3f;  // seconds before showing
      float hoverTime = 0.0f;
  };
  ```
- **File**: `src/ui.cpp` - Implement `CardTooltip_Draw(CardTooltip& tooltip, const Game& game)`:
  - If `!tooltip.visible` return early.
  - Find card by `tooltip.cardId` in `game.hand.cards`.
  - Draw tooltip box (200x120px) with shadow at `tooltip.position + offset(15, 15)`.
  - Content: card name (bold), type, effect details:
    - Move: "Move: Forward +{forward}, Lateral +{lateral}"
    - Damage: "Damage: {damage} to target"
    - Heal: "Heal: +{heal} HP"
  - If mirror available, show mirrored effect in italics.
- **File**: `src/ui.cpp` - In `HandPanel_Draw()`, on card hover:
  - Accumulate `tooltip.hoverTime += dt`.
  - If `hoverTime > showDelay`, set `tooltip.visible = true`, `tooltip.cardId`, `tooltip.position = mousePos`.
  - On mouse exit, reset `hoverTime = 0`, `visible = false`.
- **Drag preview**: When dragging over a mech slot:
  - Call `calculateMechStats()` with hypothetical assignment.
  - Draw delta overlay on slot: "+5 ATK" in green or "-3 HP" in red.
- **Test**: Tooltip visibility timing test; preview accuracy test with mock stats.

### T_059: Play/Submit Turn Button

**Priority**: P2 | **Component**: UI | **Status**: Done | **Effort**: XS

- Implement Play/Submit button to confirm mech configurations and start turn.
- Button disabled until at least one mech has a card.
- Acceptance: Button functional; proper enable/disable logic; triggers turn submission.

#### Implementation

- **File**: `src/ui.cpp` - Modify existing "Confirm" button logic:
  - Relocate button to prominent position (bottom-right of MechSlotContainer or HandPanel).
  - Increase size to 140x36px for visibility.
  - Rename label to "PLAY TURN" or "SUBMIT".
- **Enable/disable logic**:
  ```cpp
  bool canSubmit = !game.currentPlan.assignments.empty();
  GuiSetState(canSubmit ? STATE_NORMAL : STATE_DISABLED);
  if (GuiButton(playBtnRect, "PLAY TURN")) {
      actions.playSequence = true;
  }
  GuiSetState(STATE_NORMAL);  // reset
  ```
- **Visual styling**:
  - Enabled: bright accent color (green), bold text.
  - Disabled: grayed out, reduced opacity.
  - Add subtle pulse animation when enabled (scale oscillate 1.0→1.02→1.0 over 1s).
- **Integration**: Existing `actions.playSequence` handling in `Boss::processUi()` already advances phase.
- **Feedback**: On click, brief button press animation (scale down 0.95 for 0.1s).
- **Test**: Verify button disabled when `assignments.empty()`; verify phase transition on click.

---

## Polish & Animation Tasks (T_060-T_066)

Tasks T_050-T_059 implement core card UI functionality with 85-90% spec compliance. The following tasks capture deferred cosmetic/animation features and comprehensive testing.

### T_060: Play Button Animations

**Priority**: P3 | **Component**: UI | **Status**: Planned | **Effort**: S

- Add subtle animations to "PLAY TURN" button to provide visual feedback.
- Implement pulse animation on enabled state (scale oscillates 1.0 → 1.02 → 1.0 over ~1 second).
- Implement button press animation (scale down to 0.95 for 0.1s on click).
- Acceptance: Animations smooth, non-distracting; button remains clickable during animation.

#### Implementation

- **File**: `src/ui.h` - Extend button tracking:
  ```cpp
  struct UIAnimationState {
      float pulseTime = 0.0f;      // accumulates for pulse oscillation
      float pressTime = 0.0f;      // countdown for press animation
      bool isPulseEnabled = false; // only pulse when button enabled
  };
  ```
- **File**: `src/ui/cardui/hand_panel.cpp` - In `draw_play_turn_button()`:
  - Accumulate `animState.pulseTime += dt`.
  - When `canSubmit && isPulseEnabled`, calculate scale: `1.0 + 0.02 * sin(pulseTime * 2π)`.
  - On click, set `animState.pressTime = 0.1f`.
  - Apply press scale when `pressTime > 0`.
  - Draw button with computed scale applied via matrix transform.
- **Animation**: Use `std::sin()` for smooth pulse; clamp pressTime countdown.
- **Test**: Verify pulse frequency (~1 second period); verify press animation duration (0.1s).

### T_061: Mirror Toggle Animation

**Priority**: P3 | **Component**: UI | **Status**: Planned | **Effort**: S

- Animate mirror button toggle feedback with scale flip and color flash.
- On click: scale-x flips from 1 → -1 → 1 over 0.2s (horizontal flip effect).
- Color flash: momentary highlight (yellow to normal over 0.15s) on successful toggle.
- Acceptance: Animation visually indicates state change; no disruption to card placement.

#### Implementation

- **File**: `src/ui.h` - Track mirror animation state:
  ```cpp
  struct MirrorAnimationState {
      int animatingMechId = -1;  // which slot is animating
      float flipTime = 0.0f;      // 0.0 = idle, >0 = animating
      float flashTime = 0.0f;     // color flash overlay time
  };
  ```
- **File**: `src/ui/cardui/mech_slot_container.cpp` - In `MechSlotContainer_Draw()`:
  - On mirror button click, set `animState.animatingMechId = mechId` and `flipTime = 0.2f`.
  - Calculate flip scale: `1.0 - 2.0 * min(flipTime / 0.2, 1.0)` (goes 1→-1→1).
  - Apply scale-x transform when drawing mirror button.
  - Set `flashTime = 0.15f` on toggle, fade to normal color.
  - Decrement both timers each frame.
- **Visual**: Mirror button draws with horizontal flip; color flashes yellow briefly.
- **Test**: Verify flip completes in 0.2s; verify color fades over 0.15s.

### T_062: Drop Feedback Animations

**Priority**: P3 | **Component**: UI | **Status**: Planned | **Effort**: M

- Provide visual feedback on card drop: green flash on success, bounce-back on rejection.
- Successful drop: briefly highlight target slot in green (0.15s fade).
- Invalid drop: animate card back to origin with bounce easing (0.2s).
- Acceptance: Player clearly sees drop outcome; bounce-back smooth and satisfying.

#### Implementation

- **File**: `src/ui.h` - Extend DragState:
  ```cpp
  struct DragState {
      // ... existing fields ...
      Vector2 cardOriginPos = {0, 0};  // starting position for bounce-back
      float dropFeedbackTime = 0.0f;   // green flash duration
      bool dropWasValid = false;       // last drop succeeded?
  };
  ```
- **File**: `src/ui/cardui/hand_panel.cpp` - In `HandPanel_Draw()` drag initiation:
  - Store `drag.cardOriginPos = cardRect.x, cardRect.y` when drag starts.
- **File**: `src/ui/cardui/mech_slot_container.cpp` - On invalid drop:
  - Set `drag.dropWasValid = false`, `drag.dropFeedbackTime = 0.2f`.
  - Animate `drag.currentPos` from current position back to `cardOriginPos` with ease-out cubic.
  - Use lerp: `newPos = mix(cardOriginPos, currentPos, easeOutCubic(1 - feedbackTime/0.2))`.
- **File**: `src/ui/cardui/mech_slot_container.cpp` - On valid drop:
  - Set `drag.dropWasValid = true`, `drag.dropFeedbackTime = 0.15f`.
  - Highlight target slot in green (draw overlay rect with fading alpha).
- **Easing**: Implement `easeOutCubic(t) = 1 - (1-t)^3` for bounce-back.
- **Test**: Verify bounce-back reaches origin; verify green highlight fades.

### T_063: Drag Preview Stat Delta Display

**Priority**: P3 | **Component**: UI | **Status**: Planned | **Effort**: M

- Show real-time stat delta preview when dragging card over mech slot.
- When card over slot: display overlay text showing stat changes: "+5 ATK" (green), "-3 HP" (red).
- Calculate delta using `calculateMechStats()` with hypothetical assignment.
- Acceptance: Preview appears on drag-over; disappears on drag-exit; calculations accurate.

#### Implementation

- **File**: `src/ui.h` - Add to DragState:
  ```cpp
  struct DragStateDelta {
      int hoverSlotIndex = -1;
      int deltaMechId = -1;
      int deltaHealth = 0;
      int deltaAttack = 0;
      int deltaDefense = 0;
  };
  ```
- **File**: `src/ui/cardui/mech_slot_container.cpp` - In `MechSlotContainer_Draw()` drop zone detection:
  - When card is dragged over slot (collision detected):
    - Find the mech ID at that slot.
    - Create hypothetical assignment: `Assignment{mechId, draggedCardId, false}`.
    - Call `calculateMechStats()` with current plan + hypothetical assignment.
    - Compare to current stats: `deltaAttack = newStats.attackBonus - currentStats.attackBonus`, etc.
    - Store in `dragState.delta*` fields.
  - Draw delta display at card slot center:
    - "+5 ATK" in green if deltaAttack > 0, red if < 0.
    - "-3 HP" in red if health decreased.
    - Small font (11px), semi-transparent background for readability.
  - Reset deltas when dragging outside any slot.
- **Positioning**: Overlay on mech slot at (slotCenter - textWidth/2, slotCenter - textHeight/2).
- **Test**: Verify deltas calculate correctly; verify display appears/disappears with drag.

### T_064: Text Styling Polish

**Priority**: P3 | **Component**: UI | **Status**: Planned | **Effort**: XS

- Add italic styling to mirrored effect text in CardTooltip for visual distinction.
- Add bold/larger font to card names in CardTooltip.
- Acceptance: Mirrored effects clearly visually differentiated; readability improved.

#### Implementation

- **File**: `src/ui.cpp` - In `CardTooltip_Draw()`:
  - For mirrored effect display: Use italic font rendering or apply slant transform (scale x = 0.8, add offset).
  - For card name: Draw with slightly larger font size (16 vs. 14) or apply text styling.
- **Alternative** (simpler): Use color differentiation instead of italics:
  - Mirrored text in lighter gray (`Color{180, 180, 180, 255}`) vs. dark gray for normal.
  - Card name in white (bold equivalent via size increase).
- **Raylib limitation**: Raylib doesn't natively support italic/bold fonts; use size/color tricks instead.
- **Test**: Verify visual distinction between normal and mirrored text.

### T_065: Card Slot Visual Depth

**Priority**: P3 | **Component**: UI | **Status**: Planned | **Effort**: S

- Add stacked rectangle visual depth effect to card slots (similar to deck panel card stack).
- When card is placed in slot, draw 3 offset rectangles (2-3px staggered) behind the main card rect.
- Acceptance: Slot appears to have depth; visual polish without gameplay impact.

#### Implementation

- **File**: `src/ui/cardui/mech_slot_container.cpp` - In slot drawing when `assignedCardId != -1`:
  - Before drawing main card rect, draw 3 layers of offset rectangles.
  - Each layer: slightly smaller, offset down/right by 2-3px, darker color (40, 40, 40, 150).
  - Main card rect draws on top with full opacity.
  - Visual: stacked depth effect like deck panel.
- **Dimensions**: Main card rect 120×160; layers at (0,0), (2,2), (4,4) offsets.
- **Color**: Layers progressively darker (80, 80, 80) → (60, 60, 60) → (40, 40, 40).
- **Test**: Verify depth effect visible with and without assigned card.

### T_066: Layout System Unit Tests

**Priority**: P2 | **Component**: Testing | **Status**: Planned | **Effort**: S

- Add unit tests for GameUIPanel layout calculations at various window sizes.
- Verify layout proportions remain valid and no overlaps occur across screen sizes.
- Acceptance: Tests pass; layout verified for 800×600, 1280×720, 1920×1080 resolutions.

#### Implementation

- **File**: `tests/layout_tests.cpp` - Create new test file with parameterized tests:
  ```cpp
  TEST_P(GameUIPanelLayoutTest, ComputesValidLayout) {
      GameUIPanel panel;
      panel.metrics = PanelMetrics{10.0f, 12.0f, 4.0f};
      panel.computeLayout(GetParam().width, GetParam().height);

      // Verify no overlaps
      EXPECT_FALSE(CheckCollisionRecs(panel.phaseRect, panel.deckRect));
      EXPECT_FALSE(CheckCollisionRecs(panel.handRect, panel.mechSlotRect));
      // ... etc for all pairs

      // Verify within bounds
      EXPECT_GE(panel.phaseRect.y, panel.metrics.margin);
      EXPECT_LE(panel.handRect.y + panel.handRect.height, GetParam().height - panel.metrics.margin);
  }
  ```
- **Test cases**: 800×600, 1024×768, 1280×720, 1920×1080, 2560×1440.
- **Assertions**:
  - No negative dimensions (all width/height > 0).
  - No overlapping regions.
  - All regions fit within window.
  - Phase bar at top, hand bar at bottom.
  - Game board centered.
- **Test**: Run `ctest --filter "LayoutTest"` and verify all pass.

---

## Backlog

### T_042: RAII for Shaders/Models

**Priority**: P1 | **Component**: Safety/Render | **Status**: Planned | **Effort**: S | **Depends**: T_040 | **Blocking**: None

- Wrap Shader/Model handles in move-only RAII structs with validity checks and cleanup.
- Acceptance: No raw Shader/Model ownership in contexts; cleanup exercised in tests.

#### T_043: Render/Input Interfaces (Raylib Adapters + Mocks)

**Priority**: P1 | **Component**: Architecture/Testability | **Status**: Planned | **Effort**: M | **Depends**: T_038 | **Blocking**: None

- Introduce RenderBackend/InputHandler interfaces; provide raylib and mock implementations; refactor core loops to use interfaces.
- Acceptance: Smoke tests run against mocks; raylib path unchanged for runtime.

#### T_044: Persistence + Onboarding/Help

**Priority**: P2 | **Component**: UX/Feature | **Status**: Planned | **Effort**: M | **Depends**: T_041 | **Blocking**: None

- Persist settings (window, camera, bindings); add first-run help/onboarding overlay.
- Acceptance: Settings saved/loaded; onboarding/help reachable in-game; telemetry/logging captures errors.

- No new evaluation-driven tasks; rely on existing epic items (docs, distribution, performance polish).

### T_009: Wrap Render/Input Loop with Error Detection

**Status**: Planned | **Effort**: M | **Depends**: None | **Related**: T_022

- Wrap main loop in try/catch, detect null/missing assets/device failures, graceful shutdown, crash log capture.
- DoD: Error modal displays; app exits cleanly; logs captured.

### T_022: Logging & Telemetry Framework

**Status**: Planned | **Effort**: S-M | **Depends**: T_009

- Integrate logging (levels, rotation); optional opt-in telemetry hooks.
- DoD: Critical events logged; rotation working; telemetry opt-in present.

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

### T_011: Replace AppContext::game Raw Pointer with unique_ptr

**Status**: Planned | **Effort**: S | **Depends**: None

- Convert to std::unique_ptr<Game>; remove manual deletes; sanity via sanitizers.

### T_012: Centralize Hardcoded Constants into Config System

**Status**: Planned | **Effort**: S | **Depends**: None

- Add AppConfig, config/default.json, load/replace magic numbers in main/render/constants; test defaults load.

### T_013: Wrap RenderShaders & Models in RAII

**Status**: Planned | **Effort**: S | **Depends**: T_011

- Create ShaderResource/ModelResource wrappers (move-only); update contexts; handle failed loads; test cleanup.

### T_014: Introduce RenderBackend Interface & Raylib Adapter

**Status**: Planned | **Effort**: M | **Depends**: T_010

- Define RenderBackend, implement Raylib + Mock backends; refactor render.cpp to use interface; tests with both.

### T_015: Introduce InputHandler Interface & Test Doubles

**Status**: Planned | **Effort**: M | **Depends**: T_010

- Define InputHandler; implement Raylib/Test versions; refactor input + cam control; unit tests for camera control.

### T_026: Abstract Raylib Types into Platform-Agnostic Equivalents

**Status**: Planned | **Effort**: M | **Depends**: T_014

- Add math_types.h, adapters, remove raylib types from game modules; conversion tests.

### T_016: Implement Save/Config Persistence

**Status**: Planned | **Effort**: M | **Depends**: T_012

- Save/load config/preferences; round-trip tests.

### T_017: Add First-Run Onboarding & Help Flow

**Status**: Planned | **Effort**: M | **Depends**: T_016

- Onboarding modal, help screen, persistent flag; skip path.

### T_019: Input Remapping UI

**Status**: Planned | **Effort**: M | **Depends**: T_015, T_016

- Controls menu, rebind flow with conflict detection; persist bindings.

### T_020: Accessibility Toggles (Contrast, Hold/Toggle, Haptics)

**Status**: Planned | **Effort**: M | **Depends**: T_015, T_016

- High contrast, hold/toggle camera, optional haptics; persisted preferences.

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

### T_024: User Documentation & Support

**Status**: Planned | **Effort**: S | **Depends**: T_017

- User manual/help, support channel, in-app feedback.

### T_025: Build/Deploy Automation & Distribution

**Status**: Planned | **Effort**: S | **Depends**: T_023

- Release builds, installers, distribution pipeline, version/changelog automation.
