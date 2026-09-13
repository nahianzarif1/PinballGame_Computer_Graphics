#ifndef PINBALLMACHINE_H
#define PINBALLMACHINE_H

#include "Playfield.h"
#include "Ball.h"
#include "Bumper.h"
#include "Flipper.h"
#include "Plunger.h"
#include "Light.h"
#include "Camera.h"
#include <vector>

enum class ShadingMode {
    FLAT,
    GOURAUD,
    PHONG
};

enum class SelectionMode {
    OBJECT,
    LIGHT
};

class PinballMachine {
public:
    Playfield playfield;
    Ball ball;
    std::vector<Bumper> bumpers;
    Flipper leftFlipper;
    Flipper rightFlipper;
    Plunger plunger;
    std::vector<PointLight> lights;
    Camera camera;
    
    // Machine structure
    GameObject base;
    GameObject leftWall;
    GameObject rightWall;
    GameObject backWall;
    GameObject leftRail;
    GameObject rightRail;
    
    // Selection system
    int selectedObjectIndex{0}; // 0=ball, 1-3=bumpers, 4=leftFlipper, 5=rightFlipper, 6=plunger
    int selectedLightIndex{0};
    SelectionMode selectionMode{SelectionMode::OBJECT};
    ShadingMode shadingMode{ShadingMode::PHONG};
    
    PinballMachine();
    
    void initialize();
    void update(float dt);
    void render(unsigned int shaderProgram);
    void checkCollisions();
    
    // Object control
    void selectNextObject();
    void moveSelectedObject(const glm::vec3& delta);
    void rotateSelectedObject(float delta);
    void resetSelectedObject();
    
    // Light control
    void selectLight(int index);
    void moveSelectedLight(const glm::vec3& delta);
    void adjustLightIntensity(float delta);
    void resetSelectedLight();
    
    // Shading control
    void setShadingMode(ShadingMode mode);
    ShadingMode getShadingMode() const { return shadingMode; }
    
private:
    void createMachineStructure();
    void updateObjectZFromPlayfield(GameObject& obj);
    Mesh createCube(float width, float height, float depth);
    Mesh createCylinder(float radius, float height, int segments);
};

#endif // PINBALLMACHINE_H
