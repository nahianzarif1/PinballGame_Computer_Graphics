#include "Room.h"
#include "Shader.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

const float PI = 3.14159265358979323846f;

// Switch implementation
Switch::Switch(const glm::vec3& pos, bool fanControl)
    : position(pos), isOn(false), controlsFan(fanControl), animationState(0.0f) {
    switchBase.transform.position = pos;
    switchBase.transform.scale = glm::vec3(0.15f, 0.25f, 0.05f);
    switchBase.mesh = createCube(0.15f, 0.25f, 0.05f);
    switchBase.mesh.setupMesh();
    switchBase.color = glm::vec3(0.3f, 0.3f, 0.3f);

    switchToggle.transform.position = pos + glm::vec3(0.0f, 0.0f, 0.04f);
    switchToggle.transform.scale = glm::vec3(0.08f, 0.12f, 0.03f);
    switchToggle.mesh = createCube(0.08f, 0.12f, 0.03f);
    switchToggle.mesh.setupMesh();
    switchToggle.color = controlsFan ? glm::vec3(0.2f, 0.5f, 0.8f) : glm::vec3(0.8f, 0.5f, 0.2f);
}

void Switch::toggle() {
    isOn = !isOn;
}

void Switch::update(float dt) {
    float targetState = isOn ? 1.0f : 0.0f;
    animationState += (targetState - animationState) * 5.0f * dt;

    // Animate toggle rotation
    float angle = animationState * PI * 0.25f;
    switchToggle.transform.rotation = glm::vec3(0.0f, 0.0f, angle);
}

void Switch::render(unsigned int shaderProgram) {
    switchBase.render(shaderProgram);
    switchToggle.render(shaderProgram);
}

// Ceiling Fan implementation
CeilingFan::CeilingFan()
    : position(0.0f), rotationSpeed(5.0f), currentRotation(0.0f), isOn(false) {
    // Initialize with default values
    fanMotor.transform.position = position;
    fanMotor.transform.scale = glm::vec3(0.3f, 0.2f, 0.3f);
    fanMotor.mesh = createCylinder(0.3f, 0.2f, 16);
    fanMotor.mesh.setupMesh();
    fanMotor.color = glm::vec3(0.4f, 0.4f, 0.4f);

    for (int i = 0; i < 3; i++) {
        fanBlades[i].transform.position = position;
        fanBlades[i].transform.scale = glm::vec3(1.5f, 0.05f, 0.3f);
        fanBlades[i].transform.rotation = glm::vec3(0.0f, (float)i * (2.0f * PI / 3.0f), 0.0f);
        fanBlades[i].mesh = createCube(1.5f, 0.05f, 0.3f);
        fanBlades[i].mesh.setupMesh();
        fanBlades[i].color = glm::vec3(0.6f, 0.5f, 0.3f);
    }
}

CeilingFan::CeilingFan(const glm::vec3& pos)
    : position(pos), rotationSpeed(5.0f), currentRotation(0.0f), isOn(false) {
    // Fan base (motor housing)
    fanMotor.transform.position = pos;
    fanMotor.transform.scale = glm::vec3(0.3f, 0.2f, 0.3f);
    fanMotor.mesh = createCylinder(0.3f, 0.2f, 16);
    fanMotor.mesh.setupMesh();
    fanMotor.color = glm::vec3(0.4f, 0.4f, 0.4f);

    // Fan blades
    for (int i = 0; i < 3; i++) {
        fanBlades[i].transform.position = pos;
        fanBlades[i].transform.scale = glm::vec3(1.5f, 0.05f, 0.3f);
        fanBlades[i].transform.rotation = glm::vec3(0.0f, (float)i * (2.0f * PI / 3.0f), 0.0f);
        fanBlades[i].mesh = createCube(1.5f, 0.05f, 0.3f);
        fanBlades[i].mesh.setupMesh();
        fanBlades[i].color = glm::vec3(0.6f, 0.5f, 0.3f);
    }
}

void CeilingFan::toggle() {
    isOn = !isOn;
}

void CeilingFan::update(float dt) {
    if (isOn) {
        currentRotation += rotationSpeed * dt;
    }

    // Update blade rotations
    for (int i = 0; i < 3; i++) {
        fanBlades[i].transform.rotation.y = currentRotation + (float)i * (2.0f * PI / 3.0f);
    }
}

void CeilingFan::render(unsigned int shaderProgram) {
    fanMotor.render(shaderProgram);
    for (int i = 0; i < 3; i++) {
        fanBlades[i].render(shaderProgram);
    }
}

// Basketball Hoop implementation
BasketballHoop::BasketballHoop()
    : position(0.0f), ballAnimation(0.0f), ballBouncing(false) {
    // Initialize with default values
    backboard.transform.position = position;
    backboard.transform.scale = glm::vec3(1.2f, 0.8f, 0.05f);
    backboard.mesh = createCube(1.2f, 0.8f, 0.05f);
    backboard.mesh.setupMesh();
    backboard.color = glm::vec3(0.9f, 0.9f, 0.9f);

    rim.transform.position = position + glm::vec3(0.0f, -0.3f, 0.1f);
    rim.transform.scale = glm::vec3(0.45f, 0.05f, 0.45f);
    rim.mesh = createTorus(0.45f, 0.05f, 16, 32);
    rim.mesh.setupMesh();
    rim.color = glm::vec3(1.0f, 0.3f, 0.0f);

    net.transform.position = position + glm::vec3(0.0f, -0.5f, 0.1f);
    net.transform.scale = glm::vec3(0.4f, 0.4f, 0.4f);
    net.mesh = createCone(0.4f, 0.4f, 16);
    net.mesh.setupMesh();
    net.color = glm::vec3(0.9f, 0.9f, 0.9f);

    ball.transform.position = position + glm::vec3(0.0f, -0.7f, 0.15f);
    ball.transform.scale = glm::vec3(0.25f, 0.25f, 0.25f);
    ball.mesh = createSphere(0.25f, 16, 16);
    ball.mesh.setupMesh();
    ball.color = glm::vec3(0.8f, 0.4f, 0.1f);
}

BasketballHoop::BasketballHoop(const glm::vec3& pos)
    : position(pos), ballAnimation(0.0f), ballBouncing(false) {
    // Backboard
    backboard.transform.position = pos;
    backboard.transform.scale = glm::vec3(1.2f, 0.8f, 0.05f);
    backboard.mesh = createCube(1.2f, 0.8f, 0.05f);
    backboard.mesh.setupMesh();
    backboard.color = glm::vec3(0.9f, 0.9f, 0.9f);

    // Rim
    rim.transform.position = pos + glm::vec3(0.0f, -0.3f, 0.1f);
    rim.transform.scale = glm::vec3(0.45f, 0.05f, 0.45f);
    rim.mesh = createTorus(0.45f, 0.05f, 16, 32);
    rim.mesh.setupMesh();
    rim.color = glm::vec3(1.0f, 0.3f, 0.0f);

    // Net (simplified as cone)
    net.transform.position = pos + glm::vec3(0.0f, -0.5f, 0.1f);
    net.transform.scale = glm::vec3(0.4f, 0.4f, 0.4f);
    net.mesh = createCone(0.4f, 0.4f, 16);
    net.mesh.setupMesh();
    net.color = glm::vec3(0.9f, 0.9f, 0.9f);

    // Ball
    ball.transform.position = pos + glm::vec3(0.0f, -0.7f, 0.15f);
    ball.transform.scale = glm::vec3(0.25f, 0.25f, 0.25f);
    ball.mesh = createSphere(0.25f, 16, 16);
    ball.mesh.setupMesh();
    ball.color = glm::vec3(0.8f, 0.4f, 0.1f);
}

void BasketballHoop::update(float dt) {
    if (ballBouncing) {
        ballAnimation += dt * 3.0f;
        float bounceHeight = sin(ballAnimation) * 0.3f;
        ball.transform.position.y = position.y - 0.7f + bounceHeight;

        if (ballAnimation > PI * 2.0f) {
            ballAnimation = 0.0f;
            ballBouncing = false;
        }
    }
}

void BasketballHoop::render(unsigned int shaderProgram) {
    backboard.render(shaderProgram);
    rim.render(shaderProgram);
    net.render(shaderProgram);
    ball.render(shaderProgram);
}

// Glass Window implementation
GlassWindow::GlassWindow()
    : position(0.0f), dimensions(1.0f, 1.0f), isTransparent(true) {
}

GlassWindow::GlassWindow(const glm::vec3& pos, const glm::vec2& dims)
    : position(pos), dimensions(dims), isTransparent(true) {
    // Window frame
    windowFrame.transform.position = pos;
    windowFrame.transform.scale = glm::vec3(dims.x + 0.1f, dims.y + 0.1f, 0.1f);
    windowFrame.mesh = createCube(dims.x + 0.1f, dims.y + 0.1f, 0.1f);
    windowFrame.mesh.setupMesh();
    windowFrame.color = glm::vec3(0.4f, 0.3f, 0.2f);

    // Window glass
    windowGlass.transform.position = pos;
    windowGlass.transform.scale = glm::vec3(dims.x, dims.y, 0.02f);
    windowGlass.mesh = createCube(dims.x, dims.y, 0.02f);
    windowGlass.mesh.setupMesh();
    windowGlass.color = glm::vec3(0.6f, 0.8f, 1.0f);
}

void GlassWindow::render(unsigned int shaderProgram) {
    windowFrame.render(shaderProgram);

    if (isTransparent) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    windowGlass.render(shaderProgram);

    if (isTransparent) {
        glDisable(GL_BLEND);
    }
}

// Furniture implementation
Furniture::Furniture(const glm::vec3& pos, const glm::vec3& col)
    : position(pos), color(col) {
    // Seat
    seat.transform.position = pos;
    seat.transform.scale = glm::vec3(2.0f, 0.4f, 0.8f);
    seat.mesh = createCube(2.0f, 0.4f, 0.8f);
    seat.mesh.setupMesh();
    seat.color = color;

    // Backrest
    backrest.transform.position = pos + glm::vec3(0.0f, 0.4f, -0.35f);
    backrest.transform.scale = glm::vec3(2.0f, 0.6f, 0.15f);
    backrest.mesh = createCube(2.0f, 0.6f, 0.15f);
    backrest.mesh.setupMesh();
    backrest.color = color;

    // Armrests
    armrestLeft.transform.position = pos + glm::vec3(-1.0f, 0.3f, 0.0f);
    armrestLeft.transform.scale = glm::vec3(0.15f, 0.4f, 0.8f);
    armrestLeft.mesh = createCube(0.15f, 0.4f, 0.8f);
    armrestLeft.mesh.setupMesh();
    armrestLeft.color = color;

    armrestRight.transform.position = pos + glm::vec3(1.0f, 0.3f, 0.0f);
    armrestRight.transform.scale = glm::vec3(0.15f, 0.4f, 0.8f);
    armrestRight.mesh = createCube(0.15f, 0.4f, 0.8f);
    armrestRight.mesh.setupMesh();
    armrestRight.color = color;

    // Legs
    glm::vec3 legPositions[4] = {
        glm::vec3(-0.8f, -0.25f, -0.3f),
        glm::vec3(0.8f, -0.25f, -0.3f),
        glm::vec3(-0.8f, -0.25f, 0.3f),
        glm::vec3(0.8f, -0.25f, 0.3f)
    };

    for (int i = 0; i < 4; i++) {
        legs[i].transform.position = pos + legPositions[i];
        legs[i].transform.scale = glm::vec3(0.1f, 0.2f, 0.1f);
        legs[i].mesh = createCube(0.1f, 0.2f, 0.1f);
        legs[i].mesh.setupMesh();
        legs[i].color = glm::vec3(0.3f, 0.2f, 0.1f);
    }
}

void Furniture::render(unsigned int shaderProgram) {
    seat.render(shaderProgram);
    backrest.render(shaderProgram);
    armrestLeft.render(shaderProgram);
    armrestRight.render(shaderProgram);
    for (int i = 0; i < 4; i++) {
        legs[i].render(shaderProgram);
    }
}

// Room implementation
Room::Room(float w, float d, float h)
    : width(w), depth(d), height(h),
      ambientLight(0.1f, 0.1f, 0.1f),
      diffuseLight(0.8f, 0.8f, 0.8f),
      specularLight(1.0f, 1.0f, 1.0f),
      mainLightsOn(true),
      fanOn(false) {
}

void Room::initialize() {
    createRoomGeometry();
    createSwitches();
    createFurniture();
    setupLighting();

    // Initialize ceiling fan
    ceilingFan = CeilingFan(glm::vec3(0.0f, height - 0.5f, 0.0f));

    // Initialize basketball hoop
    basketballHoop = BasketballHoop(glm::vec3(-width/2 + 1.0f, height - 2.0f, -depth/2 + 0.5f));

    // Initialize glass window
    glassWindow = GlassWindow(glm::vec3(width/2 - 0.5f, height/2, -depth/2 + 0.5f), glm::vec2(3.0f, 4.0f));
}

void Room::createRoomGeometry() {
    // Floor
    floor.transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    floor.transform.scale = glm::vec3(width, 0.2f, depth);
    floor.mesh = createCube(width, 0.2f, depth);
    floor.mesh.setupMesh();
    floor.color = glm::vec3(0.3f, 0.25f, 0.2f); // Wood floor color

    // Ceiling
    ceiling.transform.position = glm::vec3(0.0f, height, 0.0f);
    ceiling.transform.scale = glm::vec3(width, 0.2f, depth);
    ceiling.mesh = createCube(width, 0.2f, depth);
    ceiling.mesh.setupMesh();
    ceiling.color = glm::vec3(0.9f, 0.9f, 0.85f); // White ceiling

    // Walls
    // Back wall
    walls[0].transform.position = glm::vec3(0.0f, height/2, -depth/2);
    walls[0].transform.scale = glm::vec3(width, height, 0.2f);
    walls[0].mesh = createCube(width, height, 0.2f);
    walls[0].mesh.setupMesh();
    walls[0].color = glm::vec3(0.7f, 0.6f, 0.5f);

    // Front wall
    walls[1].transform.position = glm::vec3(0.0f, height/2, depth/2);
    walls[1].transform.scale = glm::vec3(width, height, 0.2f);
    walls[1].mesh = createCube(width, height, 0.2f);
    walls[1].mesh.setupMesh();
    walls[1].color = glm::vec3(0.7f, 0.6f, 0.5f);

    // Left wall
    walls[2].transform.position = glm::vec3(-width/2, height/2, 0.0f);
    walls[2].transform.scale = glm::vec3(0.2f, height, depth);
    walls[2].mesh = createCube(0.2f, height, depth);
    walls[2].mesh.setupMesh();
    walls[2].color = glm::vec3(0.7f, 0.6f, 0.5f);

    // Right wall
    walls[3].transform.position = glm::vec3(width/2, height/2, 0.0f);
    walls[3].transform.scale = glm::vec3(0.2f, height, depth);
    walls[3].mesh = createCube(0.2f, height, depth);
    walls[3].mesh.setupMesh();
    walls[3].color = glm::vec3(0.7f, 0.6f, 0.5f);
}

void Room::createSwitches() {
    // Light switch on wall
    switches.push_back(Switch(glm::vec3(width/2 - 0.5f, 1.5f, depth/2 - 0.3f), false));

    // Fan switch on wall
    switches.push_back(Switch(glm::vec3(width/2 - 0.5f, 1.2f, depth/2 - 0.3f), true));
}

void Room::createFurniture() {
    // Sofa near the wall
    furniture.push_back(Furniture(glm::vec3(-width/2 + 2.0f, 0.3f, depth/2 - 2.0f), glm::vec3(0.6f, 0.4f, 0.3f)));

    // Another couch
    furniture.push_back(Furniture(glm::vec3(width/2 - 4.0f, 0.3f, -depth/2 + 2.0f), glm::vec3(0.4f, 0.3f, 0.5f)));
}

void Room::setupLighting() {
    // Create spotlights for room illumination
    SpotLight spot1;
    spot1.position = glm::vec3(-width/4, height - 0.5f, 0.0f);
    spot1.direction = glm::vec3(0.0f, -1.0f, 0.0f);
    spot1.ambient = glm::vec3(0.1f);
    spot1.diffuse = glm::vec3(0.8f, 0.8f, 0.7f);
    spot1.specular = glm::vec3(1.0f);
    spotLights.push_back(spot1);

    SpotLight spot2;
    spot2.position = glm::vec3(width/4, height - 0.5f, 0.0f);
    spot2.direction = glm::vec3(0.0f, -1.0f, 0.0f);
    spot2.ambient = glm::vec3(0.1f);
    spot2.diffuse = glm::vec3(0.8f, 0.8f, 0.7f);
    spot2.specular = glm::vec3(1.0f);
    spotLights.push_back(spot2);
}

void Room::update(float dt) {
    // Update switches
    for (auto& sw : switches) {
        sw.update(dt);
    }

    // Update ceiling fan
    ceilingFan.update(dt);

    // Update basketball hoop
    basketballHoop.update(dt);
}

void Room::render(unsigned int shaderProgram) {
    // Render room geometry
    floor.render(shaderProgram);
    ceiling.render(shaderProgram);
    for (int i = 0; i < 4; i++) {
        walls[i].render(shaderProgram);
    }

    // Render interactive elements
    for (auto& sw : switches) {
        sw.render(shaderProgram);
    }

    ceilingFan.render(shaderProgram);
    basketballHoop.render(shaderProgram);
    glassWindow.render(shaderProgram);

    // Render furniture
    for (auto& furn : furniture) {
        furn.render(shaderProgram);
    }
}

void Room::toggleSwitch(int index) {
    if (index >= 0 && index < switches.size()) {
        switches[index].toggle();
        if (switches[index].controlsFan) {
            fanOn = switches[index].isOn;
            ceilingFan.isOn = fanOn;
        } else {
            mainLightsOn = switches[index].isOn;
            for (auto& spot : spotLights) {
                spot.enabled = mainLightsOn;
            }
        }
    }
}

void Room::toggleFan() {
    fanOn = !fanOn;
    ceilingFan.toggle();
    switches[1].isOn = fanOn;
}

void Room::toggleMainLights() {
    mainLightsOn = !mainLightsOn;
    for (auto& spot : spotLights) {
        spot.enabled = mainLightsOn;
    }
    switches[0].isOn = mainLightsOn;
}

void Room::setAmbientIntensity(float intensity) {
    ambientLight = glm::vec3(intensity);
}

void Room::setDiffuseIntensity(float intensity) {
    diffuseLight = glm::vec3(intensity);
}

void Room::setSpecularIntensity(float intensity) {
    specularLight = glm::vec3(intensity);
}
