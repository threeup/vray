Simulation (Visuals & Mechanics)

1. Achieving the Visuals

Palettized Shading: Use a custom shader that snaps lighting levels to 3 or 4 brightness steps. This creates the sharp color bands seen on the rock.

- Note: Add a palette/posterize pass to xflat.fs (or a tiny post-process) that clamps diffuse/ambient to 3–4 bands while keeping specular at ~1% so edges stay crisp but matte.

2. Replace monsters with mech meshes (meshMech.cpp wiring)

   - Fix includes: pull in raylib, raymath, mesh headers, and std headers; ensure all Mesh/Model/Matrix identifiers resolve.
   - Build modular parts (head, torso, legs, pods) as Mesh + local Matrix; store parts in a struct and compose with MatrixMultiply + MatrixTranslate/Rotate.
   - Bake each assembled mech into a Model via LoadModelFromMesh or keep parts and draw with DrawModel using per-part transforms; assign the flat shader to all mech materials.
   - Tint: enemies use red/brown palette; heroes use green/blue palette; keep scale to fit one tile footprint (<= tile size, height ~1.0f).

3. Integrate mechs into world entities

   - Swap hero/enemy spawn to use the mech models instead of cubes in PlaceActorsFromOccupants.
   - Cache mech models once (static in World_Init) and reuse; set `model.materials[0].shader = appCtx.shaders.flat` to stay matte.
   - Position: anchor feet at tile top (TileHeight + small offset), center on tile; allow per-entity color tints.

4. Validate and test the mech pipeline

   - Run ./go.ps1; ensure meshMech.cpp compiles cleanly with all raylib symbols resolved.
   - Visually verify: no missing parts, normals correct, matte shading consistent with tiles/mountains.
   - Optional: add MeshUtils::checkIsValid on generated meshes before upload to catch bad normals or NaNs.

5. Game tick timer and turn system

   - Add a static timer in `World` or `AppContext` to track elapsed time per turn (5-second cycle).
   - Expose `Occupant` type to include AI state (current position, patrol path, etc.).
   - Call a new `World_Update(world, dt)` function during main loop to advance turn logic.

6. Enemy patrol path definition

   - Hard-code a square patrol pattern for each enemy (e.g., tile sequence of 4 waypoints forming a square).
   - Store waypoint index and elapsed time per enemy within occupant state.
   - Reset to first waypoint after completing the loop.

7. Enemy movement animation

   - During each 5-second turn, smoothly interpolate enemy position from current waypoint to next.
   - Use `Occupant` to track fractional progress (0.0–1.0) so visual position lerps over the turn duration.
   - Update entity position in render loop based on interpolation state.

8. UI turn counter and state display
   - Add a turn counter to `UiState`; increment every 5 seconds.
   - Display turn number in render controls panel or HUD.
   - Optional: show current enemy positions or waypoint arrows for debugging.
