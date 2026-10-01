#ifndef PINBALLMACHINE_H
#define PINBALLMACHINE_H

#include "Playfield.h"
#include "Ball.h"
#include "Bumper.h"
#include "Flipper.h"
#include "Plunger.h"
#include "Light.h"
#include "Camera.h"
#include "Room.h"
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

    GameObject base;
    GameObject leftWall;
    GameObject rightWall;
    GameObject backWall;
    GameObject frontWallLeft;
    GameObject frontWallRight;
    GameObject laneWall;
    GameObject leftRail;
    GameObject rightRail;

    // Game hub room environment
    Room room;

    int selectedObjectIndex{0};
    int selectedLightIndex{0};
    SelectionMode selectionMode{SelectionMode::OBJECT};
    ShadingMode shadingMode{ShadingMode::PHONG};

    PinballMachine();

    void initialize();
    void update(float dt);
    void render(unsigned int shaderProgram);
    void checkCollisions();

    void selectNextObject();
    void moveSelectedObject(const glm::vec3& delta);
    void rotateSelectedObject(float delta);
    void resetSelectedObject();

    void selectLight(int index);
    void moveSelectedLight(const glm::vec3& delta);
    void adjustLightIntensity(float delta);
    void resetSelectedLight();

    void setShadingMode(ShadingMode mode);
    ShadingMode getShadingMode() const { return shadingMode; }

    void setLeftFlipperPowered(bool on);
    void setRightFlipperPowered(bool on);
    void setPlungerPulling(bool on);

    // Room control methods
    void toggleRoomSwitch(int index);
    void toggleRoomFan();
    void toggleRoomLights();
    void setRoomAmbient(float intensity);
    void setRoomDiffuse(float intensity);
    void setRoomSpecular(float intensity);

    float tableHalfWidth() const { return playfield.width * 0.5f; }
    float tableHalfLength() const { return playfield.length * 0.5f; }

private:
    void createMachineStructure();
    void placeOnPlayfield(GameObject& obj, float heightOffset);
    void clampObjectToTable(glm::vec3& position, float radius) const;
    void applyLighting(unsigned int shaderProgram) const;
    void collideBallWithFlipper(Flipper& flipper);
    void collideBallWithSegment(const glm::vec2& a, const glm::vec2& b, float radius, float bounce);
};

#endif // PINBALLMACHINE_H
