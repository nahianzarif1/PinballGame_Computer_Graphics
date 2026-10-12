# The Game Hub — Neon Circuit Pinball

A C++17 / OpenGL 3.3 pinball game inside a compact Technology gaming lounge modeled on the supplied room photograph: white porcelain tiles, navy acoustic TV panels, angular blue LED borders, suspended Y and square pendants, two gray beanbags on an electric-blue rug, a TV and black console table, a beverage fridge and white cabinets, a mascot, an air conditioner, and a glass side wall. A separate four-blade ceiling fan rotates above the lounge. The pinball machine sits to the side, with its original physics and score rules.


## Run the updated game

Double-click **Launch Game.command**, or run:

```bash
./build/GameHubPinball
```

The game starts in the reference room view. Use **1** for the lights, **2** for the fan, and **B / G / P** for Flat / Gouraud / Phong shading. Press **T** for ray tracing and **O** to open or close the refrigerator. These actions also have clickable console buttons. Click either labeled wall rocker or the fridge door, or aim the crosshair and press **F**. Press **H** to restore the reference camera. Shading and ray-tracing shortcuts use ordinary letter keys, so no Fn key is needed on a MacBook.

Press **C** to play pinball, or press **Space** to switch to the table and launch a waiting ball onto the board. Use **A / D** to flip. Assets are found next to the executable, so starting from another working directory also works.

## Controls

| Key | Action |
| --- | --- |
| Space | Quick launch of a waiting ball |
| Enter | Hold to charge the plunger; release to launch |
| A / D | Left / right flipper |
| R | Restart score, balls, bonus and target bank |
| C | Switch between table and room cameras |
| W / S | Camera forward / backward |
| Left / right arrows | Camera strafe |
| Q / E | Camera up / down |
| Right mouse | Look around |
| Scroll | Zoom |
| 1 | Room lights |
| 2 | Ceiling fan on/off, with acceleration and coast to stop |
| Left click / F | Use a wall rocker switch (F uses the crosshair) |
| H | Restore reference room camera |
| 3 / 4 | Day / night |
| B / G / P | Flat / Gouraud / Phong shading |
| T | Toggle ray tracing; B / G / P return to raster shading |
| O | Open / close refrigerator door, including reversal during motion |
| Escape | Exit |

Space and Enter do not reset a ball that is already playing. A game has three balls. A drain ends the current ball, collects its bonus, and places the next ball in the shooter lane. An eight-second ball save protects the first launch of each ball once; a saved ball can be launched again with Space. At game over, press R.

## Scoring

| Event | Base score | Added end-of-ball bonus |
| --- | ---: | ---: |
| Left / right main pop bumper | 100 | 10 |
| Upper middle pop bumper | 150 | 10 |
| Small side bumper | 50 | 10 |
| Drop target | 100 | 50 |
| Slingshot | 10 | 0 |
| Left orbit, traveled upward from the side lane | 250 | 100 |
| Complete all four drop targets | 1,000 | 0 |

The current multiplier applies to scoring events. Completing the target bank also raises the multiplier by one, up to 5x, and resets the bank after its completion animation. For example, the first bank scores 4 × 100 + 1,000 = **1,400**, then raises the multiplier to **2x**. Its 200 accumulated bonus becomes **400** when the ball drains. The multiplier and bonus reset on a lost ball; a ball save preserves them.

Flippers and ordinary walls award no points. Contact cooldowns prevent repeated points from the same impact. The console shows score, **session** best score, ball number, remaining balls, multiplier, pending drain bonus, target-bank progress, ball-save countdown, and recent scoring events. The session best is retained when pressing R and resets when the application closes.

## Build and test

Dependencies: CMake, GLFW, GLM and OpenGL. On macOS:

```bash
brew install cmake glfw glm
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build -j8
ctest --test-dir build --output-on-failure
./build/GameHubPinball
```

Physics runs at a fixed 240 Hz. Regression checks cover shooter-lane exit at 30/60/144/10 FPS and three launch powers, plunger release, idle balls, Space during play, surface alignment, bumper cooldowns, target-bank points, multiplied drain bonuses, ball saves, three-ball accounting, game over, flipper launches, center drains and matching motion at different render rates. A second room-control suite verifies independent switches, wall picking, fan acceleration and stopping, camera bounds, console scaling, and compatibility with pinball.

The room suite also checks door opening, closing, reversal, picking, and the new buttons at different window sizes. The third suite checks ray intersections against an independent exhaustive reference, transformed normals, camera projection, finite shadow segments, transparent shadows, mirror reflections, Snell refraction, Fresnel reflectance, total internal reflection, texture filtering, deterministic threaded rendering, and animated scene snapshots. All three suites run without an OpenGL window.

For a reproducible rendered capture (PPM):

```bash
./build/GameHubPinball --capture /tmp/table.ppm --frames 100 --launch
./build/GameHubPinball --capture /tmp/room.ppm --frames 120 --room
./build/GameHubPinball --capture /tmp/off.ppm --frames 100 --room --lights-off --fan-off
./build/GameHubPinball --capture /tmp/flat.ppm --frames 100 --room --shading flat
./build/GameHubPinball --capture /tmp/rays.ppm --frames 90 --room --raytrace --ray-width 960 --ray-samples 4
./build/GameHubPinball --capture /tmp/open.ppm --frames 90 --room --door-open --raytrace
./build/GameHubPinball --capture /tmp/ball-rays.ppm --frames 100 --table --launch --raytrace
```

Source: `src/` and `include/`; shaders: `shaders/`; physics regression checks: `tests/PhysicsTests.cpp`.

## Implemented graphics requirements

| Requirement | Implementation and demonstration |
| --- | --- |
| Lighting | Attenuated point lights, table spotlight with cone falloff, material specular highlights, emissive fixtures, light switch, day/night ambient |
| Transformation | Translation, quaternion rotation and nonuniform scale; inverse-transpose normals; parent placement of the entire pinball machine; hinged fridge assembly |
| Gouraud shading | G: lighting evaluated at vertices in `gouraud.vert`, then interpolated across triangles |
| Phong shading | P: interpolated normals normalized and lighting evaluated per fragment in `phong.frag` |
| Motion / animation | Fan acceleration, rotation and coast to stop; door hinge animation; ball physics, flippers, plunger and target animations |
| Button interaction / open-close | Click OPEN FRIDGE / CLOSE FRIDGE or press O; glass, frame and handle rotate together; clicking again reverses direction |
| Ray tracing | T or ENABLE RAY TRACING: full-scene triangle BVH, primary rays, shadow rays, recursive reflections and dielectric refraction |

## Ray tracing implementation

`RayTracer.cpp` implements a CPU Whitted ray tracer. Every visible mesh from the room and pinball machine is exported into an immutable world-space snapshot, with the same animated transforms, colors, normals, UVs, procedural textures, and lighting policy as the raster renderer. It traces geometry directly, including geometry outside the camera view that appears in reflections.

A triangle BVH with twelve-bin surface-area splits accelerates two-sided Möller–Trumbore intersections. Equal-count fallback bounds tree depth for degenerate geometry. Compact cached triangle origins and edges avoid recomputing intersection data for every ray. Bounded shadow rays use one traversal to stop at opaque blockers and collect transparent interfaces, preserving tinted glass shadows without restarting the BVH at every pane. Shared triangle edges count as one glass interface. Secondary rays reflect from the tiled floor, metal kettle, door trim, plunger and steel pinball. Closed glass meshes use Snell refraction, dielectric Fresnel reflectance, and total internal reflection, with origin offsets to avoid self-intersection. Recursion stops after six secondary levels or when a branch contributes less than 0.5%.

Up to eight CPU workers render a frame in the background, leaving one logical CPU available on smaller machines for input and physics. OpenGL presents the RGB image and HUD. Press **T** and hold the camera still: live detail advances from a 480-pixel preview through 960 pixels to 1280 pixels with two samples per pixel. These widths are capped at the native viewport and a two-megapixel image budget. Active pinball uses up to 640 pixels with one sample to refresh more frequently. The console shows the actual ray-image width; the room console also shows frame time and a **REFINE** indicator during refinement.

Camera, lighting and door changes cancel obsolete work. While the camera or door moves, the application shows a clear raster view, then starts a new ray image after a short pause. Animated objects receive fresh scene snapshots; fully refined static scenes stop consuming ray-rendering CPU until the scene changes. Completed detail never drops back to a coarser preview while the view stays unchanged. `--ray-width` and `--ray-samples` now set the final live quality as well as capture quality, for example `./build/GameHubPinball --ray-width 1920 --ray-samples 4`. Higher detail takes longer on this CPU renderer.

Captures default to 480 pixels wide with four samples per pixel and trace the final simulated frame synchronously for reproducible output; use `--ray-width 960` for a more detailed capture. `--ray-samples` accepts 1–64. Automatic captures use a hidden window so they do not interrupt normal desktop input.

Add `--capture-live` to a ray-traced capture command to exercise the asynchronous live renderer instead. It waits for the requested final refinement before saving, including explicitly selected sample counts.

This renderer calculates direct illumination plus recursive mirror and glass transport. Ambient illumination approximates diffuse indirect light; diffuse global illumination, rough reflection sampling, nested dielectric media, and hardware ray-tracing APIs are outside this implementation. Glass panes are modeled as separate solids surrounded by air.

Algorithm references: [BVH acceleration](https://www.pbr-book.org/4ed/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies) and [specular reflection and transmission](https://www.pbr-book.org/4ed/Reflection_Models/Specular_Reflection_and_Transmission) in *Physically Based Rendering*.
