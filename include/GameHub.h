#ifndef GAMEHUB_H
#define GAMEHUB_H

#include "PinballMachine.h"
#include "HUD.h"
#include "Light.h"
#include "Camera.h"
#include "GameObject.h"
#include "RayTracer.h"
#include <vector>

class GameHub {
public:
    PinballMachine machine;
    HUD hud;
    Camera hubCamera;
    bool lightsOn{true};
    bool fanOn{true};
    bool dayMode{false};
    bool pinballCam{false};
    float fanAngle{0.0f};
    float fanAngularSpeed{0.0f};
    bool doorOpen{false};
    float doorAngle{0.0f};
    bool rayTracing{false};
    double rayFrameMilliseconds{0};
    int rayFrameWidth{0};
    bool rayRefining{true};

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
    void toggleDoor();
    void toggleRayTracing();
    void setShadingMode(ShadingMode mode);
    RayScene rayScene() const;
    void setDayMode(bool day);
    void toggleCamera();
    void resetAll();
    void resetRoomCamera();
    bool interactSwitches(const glm::vec3& origin, const glm::vec3& direction);
    void applyLighting(unsigned int shaderProgram) const;
    glm::vec3 skyColor() const;

private:
    struct LightSurface { size_t objectIndex; float emission; };
    std::vector<GameObject> scenery;
    std::vector<GameObject> transparent;
    std::vector<LightSurface> lightSurfaces;
    std::vector<PointLight> roomLights;
    GameObject switchLight, switchFan;
    GameObject fanHub, fanRod, fanBlade[4];
    SpotLight tableSpot;
    glm::mat4 machinePlacement{1.0f};
    size_t fridgeDoorIndex{0};
    struct DoorPart { size_t index; glm::vec3 offset; };
    std::vector<DoorPart> doorParts;

    GameObject& addBox(const char* name, const glm::vec3& pos, const glm::vec3& size,
                       const glm::vec3& color, float emission = 0.0f, bool switched = false);
    void addBeam(const char* name, const glm::vec3& a, const glm::vec3& b, float width,
                 const glm::vec3& color, float emission = 0.0f, bool switched = false);
    void addText(const char* text, const glm::vec3& pos, float height, const glm::vec3& color);
    void buildRoom();
    void buildFeatureWall();
    void buildLounge();
    void buildCabinets();
    void buildLighting();
    void buildSwitches();
    void buildFan();
    void updateFan(float dt);
    void updateDoor(float dt);
    glm::vec3 sceneAmbient() const;
    std::vector<PointLight> effectiveLights() const;
    SpotLight effectiveSpot() const;
    void constrainCameras();
};
#endif
