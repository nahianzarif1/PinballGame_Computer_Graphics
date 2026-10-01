#ifndef ROOM_H
#define ROOM_H

#include "GameObject.h"
#include "Light.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <vector>

// Switch for controlling lights and fan
class Switch {
public:
    GameObject switchBase;
    GameObject switchToggle;
    glm::vec3 position;
    bool isOn;
    bool controlsFan; // true = controls fan, false = controls lights
    float animationState; // 0.0 = off position, 1.0 = on position

    Switch(const glm::vec3& pos, bool fanControl = false);
    void toggle();
    void update(float dt);
    void render(unsigned int shaderProgram);
};

// Ceiling fan with animation
class CeilingFan {
public:
    GameObject fanBase;
    GameObject fanBlades[3];
    GameObject fanMotor;
    glm::vec3 position;
    float rotationSpeed;
    float currentRotation;
    bool isOn;

    CeilingFan(const glm::vec3& pos);
    void toggle();
    void update(float dt);
    void render(unsigned int shaderProgram);
};

// Basketball hoop on wall
class BasketballHoop {
public:
    GameObject backboard;
    GameObject rim;
    GameObject net;
    GameObject ball;
    glm::vec3 position;
    float ballAnimation;
    bool ballBouncing;

    BasketballHoop(const glm::vec3& pos);
    void update(float dt);
    void render(unsigned int shaderProgram);
};

// Glass window with transparency
class GlassWindow {
public:
    GameObject windowFrame;
    GameObject windowGlass;
    glm::vec3 position;
    glm::vec2 dimensions;
    bool isTransparent;

    GlassWindow(const glm::vec3& pos, const glm::vec2& dims);
    void render(unsigned int shaderProgram);
};

// Furniture (sofa/couch)
class Furniture {
public:
    GameObject seat;
    GameObject backrest;
    GameObject armrestLeft;
    GameObject armrestRight;
    GameObject legs[4];
    glm::vec3 position;
    glm::vec3 color;

    Furniture(const glm::vec3& pos, const glm::vec3& col);
    void render(unsigned int shaderProgram);
};

// Spotlight for room lighting
struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    bool enabled;

    SpotLight();
};

// Main Room class - the game hub environment
class Room {
public:
    // Room geometry
    GameObject floor;
    GameObject ceiling;
    GameObject walls[4]; // front, back, left, right

    // Room dimensions
    float width;
    float depth;
    float height;

    // Interactive elements
    std::vector<Switch> switches;
    CeilingFan ceilingFan;
    BasketballHoop basketballHoop;
    GlassWindow glassWindow;
    std::vector<Furniture> furniture;

    // Lighting
    std::vector<SpotLight> spotLights;
    glm::vec3 ambientLight;
    glm::vec3 diffuseLight;
    glm::vec3 specularLight;

    // State
    bool mainLightsOn;
    bool fanOn;

    Room(float w = 20.0f, float d = 20.0f, float h = 8.0f);

    void initialize();
    void update(float dt);
    void render(unsigned int shaderProgram);

    // Interaction
    void toggleSwitch(int index);
    void toggleFan();
    void toggleMainLights();

    // Lighting control
    void setAmbientIntensity(float intensity);
    void setDiffuseIntensity(float intensity);
    void setSpecularIntensity(float intensity);

private:
    void createRoomGeometry();
    void createSwitches();
    void createFurniture();
    void setupLighting();
};

#endif // ROOM_H
