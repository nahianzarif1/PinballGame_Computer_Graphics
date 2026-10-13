# Media guide and feature coverage

The README uses real application renders and the supplied project recording. The gallery contains **27 feature screenshots**, one combined overview, three H.264 MP4 videos, and three animated GIFs. All referenced media is stored in this repository.

## Video sources

| File | Source | Length |
| --- | --- | --- |
| [Full recording](media/full-demo.mp4) | Complete supplied `Screen Recording 2026-10-09 at 9.57.47 PM.mov`, converted to H.264 | 135.057 seconds |
| [Gameplay excerpt](media/gameplay-demo.mp4) | Original recording from 00:58 through 01:26 | 28 seconds |
| [Room camera excerpt](media/room-tour.mp4) | Original recording from 00:35 through 00:53 | 18 seconds |
| [README preview](media/demo-preview.gif) | Four seconds of the room camera followed by six seconds of pinball gameplay, sampled from the full recording | 10 seconds, looping |
| [Fan animation](media/fan-animation.gif) | Application renders of acceleration, rotation, power-off, and coasting | 4 seconds, looping |
| [Door animation](media/door-animation.gif) | Application renders of opening, reversing during motion, reopening, and closing | 4 seconds, looping |

The MP4 files preserve the recorded composition at **1280 × 792**. The full recording retains all 7,438 source video frames and their timestamps. The supplied recording has no audio track. MP4 encoding uses H.264 with network playback optimization. The original MOV remains outside the repository; the smaller derived videos are the published copies.

The preview is an excerpt montage, not a replacement for the full recording. The fan and door GIFs are additional deterministic captures from the game, not segments from the supplied recording. Their console labels and animated geometry come directly from the application renderer.

## Screenshot provenance

Screenshots were rendered from the completed project on October 13, 2026 using a hidden **1600 × 1000** OpenGL window. Geometry, shading, textures, lighting, picking, and HUD use the normal game core. Ray-traced captures use a **1168 × 1000** ray image with **two samples per pixel** and the raster HUD drawn afterward.

Gameplay examples deliberately set up collision/life states with the same public APIs used by the physics regression suite. For example, target contacts are generated with `checkCollisions()`; the 1,400-point bank and 400-point drain bonus are calculated by the game, not drawn onto the screenshot. These are feature demonstrations rather than a single uninterrupted play session.

The Flat/Gouraud/Phong comparison uses the same room camera, geometry, light settings, and simulated fan time. Individual screenshots are lossless PNG conversions of the exported framebuffer; no generated artwork or replacement HUD was used. `project-overview.png` combines the table and room screenshots with two identifying captions.

## Coverage map

| Feature | Screenshot | How to demonstrate it in the game |
| --- | --- | --- |
| Table objects and shooter lane | [table-ready.png](screenshots/table-ready.png) | C selects the table; a waiting ball remains in the lane |
| Live gameplay and score feedback | [active-gameplay.png](screenshots/active-gameplay.png) | Space, then A / D |
| Charged plunger | [plunger-charged.png](screenshots/plunger-charged.png) | Hold Enter; release to launch |
| Powered flipper animation | [flippers-powered.png](screenshots/flippers-powered.png) | Hold A / D |
| Bumper hit and cooldown scoring | [bumper-hit.png](screenshots/bumper-hit.png) | Strike a pop bumper |
| Slingshot kick and scoring | [slingshot-hit.png](screenshots/slingshot-hit.png) | Strike a side slingshot |
| Upward left orbit and bonus | [orbit-hit.png](screenshots/orbit-hit.png) | Traverse the left lane upward and exit at the top |
| Four-target bank and multiplier | [target-bank.png](screenshots/target-bank.png) | Hit all four drop targets |
| Multiplied end-of-ball bonus | [drain-bonus.png](screenshots/drain-bonus.png) | Drain after earning bonus and a multiplier |
| Single-use ball save | [ball-save.png](screenshots/ball-save.png) | Drain within the first launch's eight-second save window |
| Three-ball game over | [game-over.png](screenshots/game-over.png) | Lose all three balls |
| Restart and retained session best | [restart-session-best.png](screenshots/restart-session-best.png) | R |
| Lounge modeling and procedural textures | [room-overview.png](screenshots/room-overview.png) | H restores the room composition |
| Camera navigation and room layout | [camera-exploration.png](screenshots/camera-exploration.png) | W / S, arrows, Q / E, right mouse, scroll |
| Switched room lighting | [lights-off.png](screenshots/lights-off.png) | 1 or the lights control |
| Day/night ambience | [day-mode.png](screenshots/day-mode.png), [room-overview.png](screenshots/room-overview.png) | 3 / 4 |
| Fan acceleration and rotation | [fan-running.png](screenshots/fan-running.png) | 2; running speed reaches 40 RPM |
| Fan coasting to a stop | [fan-stopped.png](screenshots/fan-stopped.png) | 2 again; lighting remains independent |
| Refrigerator closed assembly | [fridge-closed.png](screenshots/fridge-closed.png) | O or click the door |
| Refrigerator hinge transform | [fridge-open.png](screenshots/fridge-open.png) | O; wait for the 100-degree open angle |
| Picking and independent wall switches | [wall-switches.png](screenshots/wall-switches.png) | Click a rocker, or aim and press F |
| Flat shading | [shading-flat.png](screenshots/shading-flat.png) | B |
| Gouraud shading | [shading-gouraud.png](screenshots/shading-gouraud.png) | G |
| Phong shading | [shading-phong.png](screenshots/shading-phong.png) | P |
| Ray-traced room, shadows, floor reflections | [room-raytrace.png](screenshots/room-raytrace.png) | T; hold the camera still for refinement |
| Transparent glass and refraction | [glass-raytrace.png](screenshots/glass-raytrace.png) | T near the refrigerator |
| Ray-traced table, metal ball, and cover | [table-raytrace.png](screenshots/table-raytrace.png) | C, then T |

Fixed-step consistency and intersection correctness are supported by regression tests, rather than inferred from still images. The gallery shows each class of scoring event; individual bumpers with different base values are listed in the README scoring table.

## Regenerate the feature renders

The [capture helper](tools/capture_features.cpp) links against the same `pinball_core` as the game. It stages the gallery cases, captures actual OpenGL frames, and renders the fan/door animation frames. It is a documentation utility, separate from normal gameplay and the test targets.

Build the main project first. On the verified macOS/Homebrew setup, run these commands from the repository root. Selecting the compiler from the CMake cache keeps the helper compatible with the core library:

```bash
CXX_COMPILER=$(sed -n 's/^CMAKE_CXX_COMPILER:FILEPATH=//p' build/CMakeCache.txt)
BREW_PREFIX=$(brew --prefix)

"$CXX_COMPILER" -std=c++17 -O2 \
  -Iinclude -Iexternal/glad/include -I"$BREW_PREFIX/include" \
  docs/tools/capture_features.cpp build/libpinball_core.a \
  -L"$BREW_PREFIX/lib" -lglfw -framework OpenGL -pthread \
  -o build/capture_features

./build/capture_features /tmp/pinball-feature-captures
```

The output files are PPM images. Convert the feature files to PNG with an image converter; files named `door-frame-*` and `fan-frame-*` are animation sequences. `--animations-only` skips most stills and ray captures while regenerating the wall-switch capture and both animation sequences. `--scoring-only` captures the orbit/slingshot cases and checks that the intended 250/10-point rewards were produced. Use the normal application's capture flags in the README for simpler room/table captures.

The app can render these frames only from a desktop session that permits creation of an OpenGL context. The simulation uses fixed steps; the source video retains its original variable frame timing.

## Validation

- All local links and image references in the README and this guide are checked against files in the repository.
- Every PNG decodes at its intended dimensions; all three GIFs are decoded frame by frame.
- All three MP4 files are decoded through their full length and checked for nonempty frames.
- Video codec, duration, frame counts, dimensions, sizes, and hashes are recorded in [media-manifest.json](media-manifest.json).
- The screenshot contact sheet, full-size scoring/control examples, and animation frames were visually inspected.
- The application source is unchanged by this documentation update. Its clean Release build and all three CTest suites passed during the preceding repository verification.

[Return to the README](../README.md).
