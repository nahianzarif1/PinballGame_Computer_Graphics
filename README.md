# 3D Interactive Pinball Machine

A modern OpenGL project demonstrating 3D modeling, hierarchical transformations, user interaction, lighting, collision detection, and shading techniques (Flat, Gouraud, Phong).

## 🎨 Modern Color Scheme

The project features a carefully curated color palette for visual appeal and contrast:

### Primary Colors
- **Playfield**: Forest Green `rgb(26, 102, 51)` - Classic pinball table surface
- **Ball**: Silver `rgb(242, 242, 242)` - Metallic chrome appearance
- **Base**: Dark Charcoal `rgb(38, 38, 46)` - Elegant machine foundation

### Accent Colors
- **Bumper 1**: Hot Pink `rgb(255, 51, 102)` - Vibrant coral red
- **Bumper 2**: Cyan `rgb(51, 204, 255)` - Electric blue
- **Bumper 3**: Amber `rgb(255, 204, 51)` - Golden yellow

### Metallic Finishes
- **Walls**: Steel Gray `rgb(102, 102, 115)` - Brushed metal
- **Rails**: Chrome `rgb(179, 179, 191)` - Polished silver
- **Flippers**: Sky Blue `rgb(51, 153, 255)` - Modern accent
- **Plunger**: Bronze `rgb(204, 153, 102)` - Warm metallic

### Background
- **Scene Background**: Deep Navy `rgb(13, 13, 20)` - High contrast for lighting effects

## 🎮 Controls

### Object Control Mode
Press `TAB` to cycle through objects. When an object is selected:

| Key | Action |
|-----|--------|
| `TAB` | Select next object (Ball → Bumper 1 → Bumper 2 → Bumper 3 → Left Flipper → Right Flipper → Plunger) |
| `W` | Move selected object forward (+Y) |
| `S` | Move selected object backward (-Y) |
| `A` | Move selected object left (-X) |
| `D` | Move selected object right (+X) |
| `Q` | Move selected object up (+Z) |
| `E` | Move selected object down (-Z) |
| `Z` | Rotate selected object counter-clockwise (flippers only) |
| `X` | Rotate selected object clockwise (flippers only) |
| `R` | Reset selected object to initial position |

### Light Control Mode
Press `F1`, `F2`, or `F3` to select a light. When a light is selected:

| Key | Action |
|-----|--------|
| `F1` | Select Light 1 (Pink - above Bumper 1) |
| `F2` | Select Light 2 (Cyan - above Bumper 2) |
| `F3` | Select Light 3 (Amber - above Bumper 3) |
| `W` | Move selected light forward (+Y) |
| `S` | Move selected light backward (-Y) |
| `A` | Move selected light left (-X) |
| `D` | Move selected light right (+X) |
| `Q` | Move selected light up (+Z) |
| `E` | Move selected light down (-Z) |
| `+` | Increase light intensity |
| `-` | Decrease light intensity |
| `R` | Reset selected light to bumper position |

### Shading Control
Switch between shading modes in real-time:

| Key | Shading Mode |
|-----|--------------|
| `1` | Flat Shading (faceted appearance) |
| `2` | Gouraud Shading (per-vertex lighting) |
| `3` | Phong Shading (per-pixel lighting - default) |

### Camera Control
Hold `SHIFT` while using movement keys for camera control:

| Key | Action |
|-----|--------|
| `SHIFT + W` | Move camera forward |
| `SHIFT + S` | Move camera backward |
| `SHIFT + A` | Move camera left |
| `SHIFT + D` | Move camera right |
| `SHIFT + Q` | Move camera up |
| `SHIFT + E` | Move camera down |
| `Mouse` | Look around |
| `Scroll` | Zoom in/out |

### General
| Key | Action |
|-----|--------|
| `ESC` | Exit application |

## 🏗️ Project Structure

```
PinballOpenGL/
├── CMakeLists.txt           # Build configuration
├── README.md                # This file
├── include/                 # Header files
│   ├── Mesh.h              # Mesh data structure and geometry functions
│   ├── Shader.h            # Shader loading and uniform setting
│   ├── Camera.h            # Camera class with view/projection matrices
│   ├── Transform.h         # Transform structure (position, rotation, scale)
│   ├── GameObject.h        # Base game object class
│   ├── Ball.h              # Ball physics and collision
│   ├── Bumper.h            # Bumper collision and interaction
│   ├── Flipper.h           # Flipper rotation and transformation
│   ├── Plunger.h           # Plunger movement
│   ├── Playfield.h         # Tilted playfield with Z calculation
│   ├── Light.h             # Point light structure
│   └── PinballMachine.h    # Main machine class
├── src/                    # Source files
│   ├── main.cpp            # Application entry point and game loop
│   ├── Mesh.cpp            # Geometry generation (exact mathematical vertices)
│   ├── Shader.cpp          # Shader implementation
│   ├── Camera.cpp          # Camera implementation
│   ├── Transform.cpp       # Transform implementation
│   ├── GameObject.cpp      # Base object implementation
│   ├── Ball.cpp            # Ball physics
│   ├── Bumper.cpp          # Bumper collision
│   ├── Flipper.cpp         # Flipper transformation
│   ├── Plunger.cpp         # Plunger movement
│   ├── Playfield.cpp       # Playfield tilt calculation
│   └── PinballMachine.cpp # Machine logic and rendering
└── shaders/                # GLSL shaders
    ├── phong.vert          # Phong vertex shader
    ├── phong.frag          # Phong fragment shader
    ├── gouraud.vert        # Gouraud vertex shader
    ├── gouraud.frag        # Gouraud fragment shader
    ├── flat.vert           # Flat vertex shader
    └── flat.frag           # Flat fragment shader
```

## 🔧 Building the Project

### Prerequisites
- CMake 3.10 or higher
- C++17 compatible compiler
- GLFW 3.x
- GLAD
- GLM
- OpenGL 3.3+

### Installation on macOS

1. **Install dependencies via Homebrew:**
```bash
brew install cmake glfw glm
```

2. **Setup GLAD:**
```bash
cd PinballOpenGL
mkdir -p external/glad
cd external/glad
# Download GLAD from https://glad.dav1d.de/
# Select OpenGL 3.3 Core, C language, generate
# Extract and place in external/glad/
```

3. **Build the project:**
```bash
cd PinballOpenGL
mkdir build
cd build
cmake ..
make
```

4. **Run the application:**
```bash
./PinballOpenGL
```

## 📐 Mathematical Foundations

### Playfield Tilt Equation
Objects on the tilted playfield automatically calculate their Z coordinate:
```
z = z₀ + y × sin(θ)
```
Where:
- `z₀` = base height (1.2)
- `θ` = tilt angle (8°)
- `y` = object's Y position

### Sphere Generation
Exact mathematical vertices using latitude/longitude:
```
x = R × cos(φ) × cos(θ)
y = R × cos(φ) × sin(θ)
z = R × sin(φ)
```

### Cylinder Generation
```
x = x_c + r × cos(θ)
y = y_c + r × sin(θ)
z = z_c
```

### Collision Reflection
Simple velocity reflection:
```
V' = V - 2(V · N)N
```

### Flipper Transformation
Hierarchical transformation around pivot:
```
M = T(P) × R(θ) × T(-P)
```

### Point Light Attenuation
```
F_att = 1 / (k_c + k_l × d + k_q × d²)
```

## 🎯 Key Features Demonstrated

1. **3D Modeling**: Exact mathematical geometry generation for cubes, cylinders, spheres, springs, and custom shapes
2. **Hierarchical Transformations**: Flipper rotation around pivot points using transformation matrices
3. **User Interaction**: Comprehensive keyboard control for objects, lights, and camera
4. **Lighting**: Three interactive point lights with adjustable position, intensity, and color
5. **Collision Detection**: Sphere-to-sphere collision with velocity reflection
6. **Shading Techniques**: Real-time switching between Flat, Gouraud, and Phong shading
7. **Coordinate Systems**: Local → Playfield → World → View → Clip → Screen transformation pipeline

## 🎓 Educational Value

This project demonstrates core OpenGL concepts suitable for a semester project:
- **No random vertices**: All geometry is mathematically determined
- **Proper Z calculation**: Automatic height adjustment based on playfield tilt
- **Interactive lighting**: Real-time manipulation of light sources
- **Shading comparison**: Visual demonstration of different lighting models
- **Manual control**: Debuggable object movement without complex physics

## 📊 Object Specifications

### Machine Base
- Dimensions: 8×14×1 units
- Position: (0, 0, 0)
- Color: Dark Charcoal

### Playfield
- Dimensions: 7×12 units
- Tilt: 8° around X-axis
- Base height: 1.2 units
- Color: Forest Green

### Ball
- Radius: 0.25 units
- Restitution: 0.9 (bouncy)
- Color: Silver

### Bumpers
- Radius: 0.4 units
- Height: 0.6 units
- Positions: (-1.5, 3.0), (0.0, 4.2), (1.5, 3.0)
- Colors: Pink, Cyan, Amber

### Flippers
- Length: 1.5 units
- Width: 0.25 units
- Height: 0.15 units
- Rotation range: ±30°
- Color: Sky Blue

### Plunger
- Radius: 0.15 units
- Height: 1.0 units
- Travel range: -6.0 to -4.0 units
- Color: Bronze

## 🚀 Development Roadmap

The project follows a phased development approach:

1. ✅ OpenGL setup and window creation
2. ✅ Coordinate system and basic rendering
3. ✅ Geometry generation (exact mathematical vertices)
4. ✅ Static pinball machine structure
5. ✅ Tilted playfield with Z calculation
6. ✅ Manual object control
7. ✅ Simple ball physics and collision
8. ✅ Point lighting implementation
9. ✅ Manual light control
10. ✅ Shading mode switching

## 📝 License

This project is created for educational purposes as an OpenGL semester project.

## 🤝 Contributing

This is a standalone educational project. Feel free to use it as a reference for your own OpenGL learning journey.
