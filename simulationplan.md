Into the Breach Simulation (Visuals & Mechanics)

0. Achieving the Visuals

Palettized Shading: Use a custom shader that snaps lighting levels to 3 or 4 brightness steps. This creates the sharp color bands seen on the rock.

1. Mountain mesh generation

   - Replace generic cubes with asymmetric low-poly clusters using `createCustomOctahedron` or `createSpikyBlobMesh` for jagged peaks.
   - Stack/layer octahedra at different scales to build a more natural rocky formation.
   - Keep brown/gray tints but use vertex AO for depth.

2. Reduce shader reflectivity

   - Switch from flat shader to a matte/diffuse-only pass; lower or remove any specular highlights.
   - Ensure ground tiles, props, and actors all use the toned-down lighting model (no metallic gloss).
   - Verify emissive windows on skyscrapers still shine through.

3. Hero model variation

   - Replace hero cubes with stylized shapes: use `createCustomOctahedron`, `createCustomTetrahedron`, or `createCubicStar` for distinct silhouettes.
   - Keep green tint but assign one shape per hero for quick visual ID.
   - Maintain scale so they fit on tiles without clipping.

4. Enemy model variation

   - Replace enemy cubes with different stylized shapes (e.g., twisted columns via `createTwistedColumnMesh`).
   - Keep red tint but use a distinct form from heroes (spiky blob, pentagonal prism, etc.).
   - Match scale/footprint to tiles.

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
