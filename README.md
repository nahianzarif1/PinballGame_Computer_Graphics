# Game Hub Pinball

A complete C++ OpenGL game-hub scene with a playable 3D pinball machine. You walk a room, flip wall switches for lights and a ceiling fan, look through an open glass wall, and play pinball with scoring, lives, and a Space Cadet-style side console.

## Features

- Large 3D game hub (floor, walls, ceiling, rug, sofa, couch, coffee table)
- Open glass wall with an exterior city view (sun/moon, buildings, windows)
- Independent wall switches: room lights and animated ceiling fan
- Day / night lighting (ambient, diffuse, specular, hanging table spotlight)
- Procedural textures (wood, fabric, carpet, metal, brick, stars, neon, basketball)
- Pinball cabinet on a base with legs, inclined playfield, glass cover, backglass
- Ball physics, bumpers, drop targets, slingshots, flippers, plunger, drain/lives
- Score HUD on a right-hand console panel
- Hub free camera and dedicated table camera
- Real-time Flat / Gouraud / Phong shading

## Controls

### Pinball
| Key | Action |
|-----|--------|
| `A` | Left flipper |
| `D` | Right flipper |
| `ENTER` | Hold to charge plunger, release to launch |
| `SPACE` | Reset ball into the lane and launch |
| `R` | Reset the full game (score, lives) |
| `C` | Toggle table camera / hub camera |

### Room
| Key | Action |
|-----|--------|
| `W` / `S` | Walk forward / back |
| `←` / `→` | Strafe |
| `Q` / `E` | Move up / down |
| Right mouse | Look around |
| Scroll | Zoom |

### Environment
| Key | Action |
|-----|--------|
| `1` | Room lights on/off |
| `2` | Fan on/off |
| `3` | Day mode |
| `4` | Night mode |
| `F5` / `F6` / `F7` | Flat / Gouraud / Phong |
| `ESC` | Exit |

Scoring: bumper hits +50 / +75, targets +100, flipper hits +5. Three lives; a drain costs a life.

## Build (macOS)

```bash
brew install cmake glfw glm
cd PinballOpenGL
mkdir -p build && cd build
cmake ..
cmake --build . -j8
./GameHubPinball
```

If CMake cannot find GLFW:

```bash
cmake .. -Dglfw3_DIR="$(brew --prefix glfw)/lib/cmake/glfw3"
cmake --build . -j8
./GameHubPinball
```

Run from the `build/` directory so the copied `shaders/` folder is found.

## Layout

```
             OPEN GLASS WALL / CITY
       ┌───────────────────────────┐
       │  🏀 hoop     FAN          │
       │  SWITCHES                 │
       │       ┌─────────────┐     │
       │       │  PINBALL    │     │
       │       │   MACHINE   │     │
       │       └─────────────┘     │
       │  SOFA              COUCH  │
       └───────────────────────────┘
```

## Lighting

Phong shading is the default:

\[
I = K_a I_a + K_d I_d \max(0, N \cdot L) + K_s I_s \max(0, R \cdot V)^n
\]

Two room point lights plus a downward spotlight over the table (cutoff, outer cutoff, exponent, attenuation). Switch `1` disables the room lights and spotlight. Keys `3` / `4` change sky, building windows, ambient intensity, and sun/moon.

## Project structure

```
PinballOpenGL/
├── CMakeLists.txt
├── include/          GameHub, PinballMachine, HUD, meshes, lights, objects
├── src/              scene, physics, rendering
├── shaders/          phong, gouraud, flat, hud
└── external/glad/    OpenGL loader
```
