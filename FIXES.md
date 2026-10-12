# Gameplay and reference-inspired scene update

## Launch and physics

- Replaced the lower deflector that bounced the launch back into the shooter lane with an open full-length lane and an upper guide that turns the ball left onto the playfield.
- Made rail rendering and collision resolution use the same segment geometry.
- Corrected the rendered board slope to match the physical surface and raised it clear of the cabinet.
- Removed accumulating vertical free-fall velocity from the surface-constrained ball, and removed duplicate damping.
- Added a fixed 240 Hz physics accumulator to resolve fast flippers and thin targets consistently across render frame rates.
- Introduced ready / shooter / in-play states. Ready balls stay still, Space only launches a ready ball, and Enter release uses the same launch path.
- Adjusted flipper pivots, stroke angles, relative contact velocity and rest gap so flipping sends the ball uphill and a missed ball can drain.
- Added single-use eight-second ball saves and exactly three balls per game.

## Scoring

- Points are awarded for distinct bumper, target, sling and upward orbit events.
- Removed flipper points and repeated contact scoring.
- Added four-target bank completion, a 1,000-point bank reward and 1x–5x multipliers.
- Added accumulated end-of-ball bonus, collected once on a lost ball.
- Restart clears all game and contact state while preserving the session best.

## Visuals and usability

- Rebuilt the board with navy circuit artwork, purple lanes, a cyan reactor, illuminated inserts, layered bumper caps, chrome guides and a themed backglass.
- Fixed playfield UVs so its artwork maps once across the entire board.
- Added RGB PC towers and keyboards, larger gaming stations, chairs, palms, wall branding, exposed ceiling beams and suspended LED rectangles.
- Added restrained emission and tone mapping in all three shading modes for a dark gaming-room atmosphere with visible ball and board details.
- Rebuilt the score console with an adaptive layout, bonus, multiplier, target progress, launch instructions and scoring feedback.
- Starts in the table camera and switches to it when launching from the room view.
- Finds shaders beside the executable; added Launch Game.command and reproducible capture flags.

## Verification

- Release build succeeds on macOS.
- Headless CTest physics regression suite passes, including 12 launch combinations and scoring / lives / flipper checks.
- Actual OpenGL table and room captures were inspected; shaders compiled successfully and the launch capture showed a ball in play with earned score.


## Technology lounge reference update

- Replaced the large LAN cafe with the compact lounge arrangement from `506463711.jpg`: white tiles and cabinets, a beverage refrigerator, navy TV wall with zigzag blue LEDs, a black console table, gray beanbags, an electric-blue rug, glass side wall, mascot, and air conditioner.
- Added suspended Y and square pendant lights separately from an actual four-blade fan. Fan power controls acceleration and a coast to stop; it is independent of room lighting.
- Added clickable labeled wall rocker switches and clickable console controls. Keys 1/2 and crosshair key F activate the same controls.
- Added a room console with fan RPM, lighting state, and clickable Flat/Gouraud/Phong buttons. Polished tiles, fabric, glass, and metal use distinct shininess values.
- Starts in the room view; H restores its reference composition. Space/Enter still switch to playable pinball.
- Applied a scene transform to render the pinball machine at an appropriate room scale without modifying its physical board coordinates.
- Added camera bounds for the new room, day/night window lighting, correctly oriented TV UVs, and sorted transparent glass.
- Release build and both CTest suites pass; room, lights-off, Flat, Gouraud, Phong, and active pinball captures were verified.
