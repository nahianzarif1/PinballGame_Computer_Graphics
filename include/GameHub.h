#ifndef GAMEHUB_H
#define GAMEHUB_H

#include "PinballMachine.h"
#include "HUD.h"
#include "Light.h"
#include "Camera.h"
#include "GameObject.h"
#include <vector>

class GameHub {
public:
    PinballMachine machine;
    HUD hud;
    Camera hubCamera;

    bool lightsOn{true};
    bool fanOn{false};
    bool dayMode{true};
    bool pinballCam{false};

    float fanAngle{0.0f};
    float basketballPhase{0.0f};

    GameHub();
    ~GameHub();

    void initialize();
    void update(float dt);
    void render(unsigned int shaderProgram);
    void renderHUD(int fbW, int fbH);

    Camera& activeCamera();
    const Camera& activeCamera() const;

    void toggleLights();
    void toggleFan();
    void setDayMode(bool day);
    void toggleCamera();
    void resetAll();

    void applyLighting(unsigned int shaderProgram) const;
    glm::vec3 skyColor() const;

private:
    GameObject floor;
    GameObject ceiling;
    GameObject wallLeft;
    GameObject wallRight;
    GameObject wallBack;
    GameObject wallFrontLeft;
    GameObject wallFrontRight;
    GameObject glassWall;
    GameObject trimLeft;
    GameObject trimRight;
    GameObject glassLintel;
    GameObject glassSill;
    GameObject rug;

    GameObject sofaSeat;
    GameObject sofaBack;
    GameObject sofaArmL;
    GameObject sofaArmR;
    GameObject couchSeat;
    GameObject couchBack;
    GameObject couchArmL;
    GameObject couchArmR;
    GameObject tableTop;
    GameObject tableLeg1;
    GameObject tableLeg2;
    GameObject tableLeg3;
    GameObject tableLeg4;

    GameObject switchPanel;
    GameObject switchLight;
    GameObject switchFan;
    GameObject switchLabelBar;

    GameObject fanHub;
    GameObject fanRod;
    GameObject fanBlade[4];

    GameObject hoopBoard;
    GameObject hoopRim;
    GameObject hoopNet;
    GameObject basketball;
    GameObject ballString;

    GameObject lampArm;
    GameObject lampHead;
    GameObject lampBulb;

    GameObject sky;
    GameObject sunMoon;
    GameObject exteriorGround;
    std::vector<GameObject> buildings;
    std::vector<GameObject> windows;

    SpotLight tableSpot;
    PointLight roomLights[2];

    void buildRoom();
    void buildFurniture();
    void buildSwitches();
    void buildFan();
    void buildBasketball();
    void buildExterior();
    void buildSpotlightRig();
    void updateFan(float dt);
    void updateBasketball(float dt);
    void drawTransparent(unsigned int shaderProgram);
};

#endif
