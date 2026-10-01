#ifndef PINBALLMACHINE_H
#define PINBALLMACHINE_H

#include "Playfield.h"
#include "Ball.h"
#include "Bumper.h"
#include "Flipper.h"
#include "Plunger.h"
#include "Target.h"
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
    std::vector<Target> targets;
    Flipper leftFlipper;
    Flipper rightFlipper;
    Plunger plunger;
    std::vector<PointLight> lights;
    Camera camera;

    GameObject base;
    GameObject cabinet;
    GameObject leftWall;
    GameObject rightWall;
    GameObject backWall;
    GameObject frontWallLeft;
    GameObject frontWallRight;
    GameObject laneWall;
    GameObject leftRail;
    GameObject rightRail;
    GameObject legFL;
    GameObject legFR;
    GameObject legBL;
    GameObject legBR;
    GameObject backbox;
    GameObject backglass;
    GameObject glassCover;
    GameObject slingLeft;
    GameObject slingRight;
    GameObject laneDeflector;
    GameObject apronLeft;
    GameObject apronRight;
    GameObject centerPost;

    int selectedObjectIndex{0};
    int selectedLightIndex{0};
    SelectionMode selectionMode{SelectionMode::OBJECT};
    ShadingMode shadingMode{ShadingMode::PHONG};

    int score{0};
    int lives{3};
    int ballNumber{1};
    bool gameOver{false};
    float flipperScoreCooldown{0.0f};

    PinballMachine();

    void initialize();
    void update(float dt);
    void render(unsigned int shaderProgram, bool withLighting = true);
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

    void addScore(int points);
    void resetGame();
    void launchFromLane(float speed);
    void drainBall();

    float tableHalfWidth() const { return playfield.width * 0.5f; }
    float tableHalfLength() const { return playfield.length * 0.5f; }

    void applyLighting(unsigned int shaderProgram) const;

private:
    void createMachineStructure();
    void placeOnPlayfield(GameObject& obj, float heightOffset);
    void seatBall();
    void clampObjectToTable(glm::vec3& position, float radius) const;
    void collideBallWithFlipper(Flipper& flipper);
    void collideBallWithSegment(const glm::vec2& a, const glm::vec2& b, float radius, float bounce);
};

#endif // PINBALLMACHINE_H
