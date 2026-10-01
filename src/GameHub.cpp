#include "GameHub.h"
#include "Mesh.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <string>
#include <algorithm>

namespace {
void makeBox(GameObject& obj, const char* name, const glm::vec3& pos, const glm::vec3& size, const glm::vec3& color) {
    obj.name = name;
    obj.mesh = createCube(size.x, size.y, size.z);
    obj.transform.position = pos;
    obj.color = color;
}

void uploadLight(unsigned int shader, int index, const PointLight& light) {
    std::string p = "pointLights[" + std::to_string(index) + "].";
    glUniform3fv(glGetUniformLocation(shader, (p + "position").c_str()), 1, &light.position[0]);
    glUniform3fv(glGetUniformLocation(shader, (p + "ambient").c_str()), 1, &light.ambient[0]);
    glUniform3fv(glGetUniformLocation(shader, (p + "diffuse").c_str()), 1, &light.diffuse[0]);
    glUniform3fv(glGetUniformLocation(shader, (p + "specular").c_str()), 1, &light.specular[0]);
    glUniform1f(glGetUniformLocation(shader, (p + "intensity").c_str()), light.intensity);
    glUniform1f(glGetUniformLocation(shader, (p + "kc").c_str()), light.constant);
    glUniform1f(glGetUniformLocation(shader, (p + "kl").c_str()), light.linear);
    glUniform1f(glGetUniformLocation(shader, (p + "kq").c_str()), light.quadratic);
    glUniform1i(glGetUniformLocation(shader, (p + "enabled").c_str()), light.enabled ? 1 : 0);
}
}

GameHub::GameHub() {
    initialize();
}

GameHub::~GameHub() {
    hud.shutdown();
}

void GameHub::initialize() {
    hubCamera = Camera(glm::vec3(0.0f, -11.8f, 3.2f), glm::vec3(0.0f, 0.0f, 1.0f), 90.0f, -12.0f);
    hubCamera.movementSpeed = 9.0f;
    hubCamera.zoom = 50.0f;

    tableSpot.position = glm::vec3(0.0f, 0.0f, 7.8f);
    tableSpot.direction = glm::vec3(0.0f, 0.0f, -1.0f);
    tableSpot.cutOff = 38.0f;
    tableSpot.outerCutOff = 48.0f;
    tableSpot.exponent = 20.0f;
    tableSpot.intensity = 1.8f;
    tableSpot.diffuse = glm::vec3(1.0f, 0.95f, 0.8f);
    tableSpot.specular = glm::vec3(1.0f);
    tableSpot.ambient = glm::vec3(0.06f, 0.05f, 0.04f);

    roomLights[0].position = glm::vec3(-6.0f, -4.0f, 8.2f);
    roomLights[0].setColor(glm::vec3(1.0f, 0.92f, 0.8f));
    roomLights[0].intensity = 1.4f;
    roomLights[0].linear = 0.03f;
    roomLights[0].quadratic = 0.004f;
    roomLights[1].position = glm::vec3(6.0f, 5.0f, 8.2f);
    roomLights[1].setColor(glm::vec3(0.8f, 0.9f, 1.0f));
    roomLights[1].intensity = 1.25f;
    roomLights[1].linear = 0.03f;
    roomLights[1].quadratic = 0.004f;

    buildRoom();
    buildFurniture();
    buildSwitches();
    buildFan();
    buildBasketball();
    buildExterior();
    buildSpotlightRig();
    hud.initialize();
}

void GameHub::buildRoom() {
    floor.name = "Floor";
    floor.mesh = createPlane(36.0f, 30.0f);
    floor.transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    floor.color = glm::vec3(0.62f, 0.48f, 0.32f);
    floor.mesh.loadTexture("wood");

    ceiling.name = "Ceiling";
    ceiling.mesh = createCube(36.0f, 30.0f, 0.25f);
    ceiling.transform.position = glm::vec3(0.0f, 0.0f, 9.1f);
    ceiling.color = glm::vec3(0.18f, 0.2f, 0.28f);

    makeBox(wallLeft, "WallLeft", glm::vec3(-18.0f, 0.0f, 4.5f), glm::vec3(0.35f, 30.0f, 9.0f), glm::vec3(0.12f, 0.22f, 0.32f));
    makeBox(wallRight, "WallRight", glm::vec3(18.0f, 0.0f, 4.5f), glm::vec3(0.35f, 30.0f, 9.0f), glm::vec3(0.12f, 0.22f, 0.32f));
    makeBox(wallBack, "WallBack", glm::vec3(0.0f, -15.0f, 4.5f), glm::vec3(36.0f, 0.35f, 9.0f), glm::vec3(0.1f, 0.18f, 0.28f));
    makeBox(wallFrontLeft, "FrontL", glm::vec3(-12.5f, 14.85f, 4.5f), glm::vec3(11.0f, 0.3f, 9.0f), glm::vec3(0.1f, 0.18f, 0.28f));
    makeBox(wallFrontRight, "FrontR", glm::vec3(12.5f, 14.85f, 4.5f), glm::vec3(11.0f, 0.3f, 9.0f), glm::vec3(0.1f, 0.18f, 0.28f));

    glassWall.name = "Glass";
    glassWall.mesh = createCube(14.0f, 0.08f, 7.4f);
    glassWall.transform.position = glm::vec3(0.0f, 14.9f, 4.2f);
    glassWall.color = glm::vec3(0.65f, 0.85f, 1.0f);
    glassWall.alpha = 0.18f;

    makeBox(trimLeft, "TrimL", glm::vec3(-7.05f, 14.9f, 4.5f), glm::vec3(0.25f, 0.28f, 9.0f), glm::vec3(0.75f, 0.7f, 0.55f));
    makeBox(trimRight, "TrimR", glm::vec3(7.05f, 14.9f, 4.5f), glm::vec3(0.25f, 0.28f, 9.0f), glm::vec3(0.75f, 0.7f, 0.55f));
    makeBox(glassLintel, "GlassLintel", glm::vec3(0.0f, 14.9f, 8.85f), glm::vec3(14.2f, 0.28f, 0.3f), glm::vec3(0.78f, 0.74f, 0.58f));
    makeBox(glassSill, "GlassSill", glm::vec3(0.0f, 14.9f, 0.45f), glm::vec3(14.2f, 0.32f, 0.28f), glm::vec3(0.78f, 0.74f, 0.58f));

    rug.name = "Rug";
    rug.mesh = createCube(10.0f, 8.0f, 0.04f);
    rug.transform.position = glm::vec3(-9.5f, -8.0f, 0.03f);
    rug.color = glm::vec3(0.55f, 0.18f, 0.28f);
    rug.mesh.loadTexture("carpet");
}

void GameHub::buildFurniture() {
    glm::vec3 sofa = glm::vec3(0.35f, 0.22f, 0.42f);
    makeBox(sofaSeat, "SofaSeat", glm::vec3(-9.5f, -9.2f, 0.42f), glm::vec3(4.6f, 1.8f, 0.55f), sofa);
    sofaSeat.mesh.loadTexture("fabric");
    makeBox(sofaBack, "SofaBack", glm::vec3(-9.5f, -10.05f, 1.05f), glm::vec3(4.6f, 0.4f, 1.4f), sofa * 1.1f);
    makeBox(sofaArmL, "SofaArmL", glm::vec3(-11.7f, -9.2f, 0.7f), glm::vec3(0.4f, 1.8f, 0.9f), glm::vec3(0.28f, 0.16f, 0.32f));
    makeBox(sofaArmR, "SofaArmR", glm::vec3(-7.3f, -9.2f, 0.7f), glm::vec3(0.4f, 1.8f, 0.9f), glm::vec3(0.28f, 0.16f, 0.32f));

    glm::vec3 couch = glm::vec3(0.2f, 0.38f, 0.42f);
    makeBox(couchSeat, "CouchSeat", glm::vec3(10.5f, -7.5f, 0.38f), glm::vec3(3.2f, 1.5f, 0.5f), couch);
    couchSeat.mesh.loadTexture("fabric");
    makeBox(couchBack, "CouchBack", glm::vec3(10.5f, -8.2f, 0.95f), glm::vec3(3.2f, 0.35f, 1.2f), couch * 1.15f);
    makeBox(couchArmL, "CouchArmL", glm::vec3(8.95f, -7.5f, 0.62f), glm::vec3(0.32f, 1.5f, 0.8f), glm::vec3(0.12f, 0.28f, 0.32f));
    makeBox(couchArmR, "CouchArmR", glm::vec3(12.05f, -7.5f, 0.62f), glm::vec3(0.32f, 1.5f, 0.8f), glm::vec3(0.12f, 0.28f, 0.32f));

    glm::vec3 wood(0.45f, 0.28f, 0.16f);
    makeBox(tableTop, "CoffeeTable", glm::vec3(-9.5f, -6.6f, 0.45f), glm::vec3(2.4f, 1.2f, 0.12f), wood);
    tableTop.mesh.loadTexture("wood");
    makeBox(tableLeg1, "TLeg1", glm::vec3(-10.4f, -7.05f, 0.22f), glm::vec3(0.1f, 0.1f, 0.44f), wood * 0.7f);
    makeBox(tableLeg2, "TLeg2", glm::vec3(-8.6f, -7.05f, 0.22f), glm::vec3(0.1f, 0.1f, 0.44f), wood * 0.7f);
    makeBox(tableLeg3, "TLeg3", glm::vec3(-10.4f, -6.15f, 0.22f), glm::vec3(0.1f, 0.1f, 0.44f), wood * 0.7f);
    makeBox(tableLeg4, "TLeg4", glm::vec3(-8.6f, -6.15f, 0.22f), glm::vec3(0.1f, 0.1f, 0.44f), wood * 0.7f);
}

void GameHub::buildSwitches() {
    makeBox(switchPanel, "SwitchPanel", glm::vec3(-17.72f, 2.0f, 2.4f), glm::vec3(0.12f, 1.8f, 1.2f), glm::vec3(0.18f, 0.2f, 0.24f));
    makeBox(switchLight, "SwitchLight", glm::vec3(-17.62f, 1.55f, 2.5f), glm::vec3(0.12f, 0.28f, 0.45f), glm::vec3(0.2f, 0.9f, 0.35f));
    makeBox(switchFan, "SwitchFan", glm::vec3(-17.62f, 2.45f, 2.5f), glm::vec3(0.12f, 0.28f, 0.45f), glm::vec3(0.2f, 0.7f, 1.0f));
    makeBox(switchLabelBar, "SwitchBar", glm::vec3(-17.68f, 2.0f, 3.05f), glm::vec3(0.08f, 1.6f, 0.08f), glm::vec3(0.9f, 0.75f, 0.3f));
}

void GameHub::buildFan() {
    fanRod.name = "FanRod";
    fanRod.mesh = createCylinder(0.06f, 1.6f, 10);
    fanRod.transform.position = glm::vec3(0.0f, 3.5f, 8.2f);
    fanRod.color = glm::vec3(0.55f, 0.55f, 0.6f);

    fanHub.name = "FanHub";
    fanHub.mesh = createCylinder(0.35f, 0.22f, 16);
    fanHub.transform.position = glm::vec3(0.0f, 3.5f, 7.4f);
    fanHub.color = glm::vec3(0.75f, 0.78f, 0.85f);
    fanHub.mesh.loadTexture("metal");

    for (int i = 0; i < 4; i++) {
        fanBlade[i].name = "Blade";
        fanBlade[i].mesh = createCube(2.4f, 0.35f, 0.05f);
        fanBlade[i].color = glm::vec3(0.85f, 0.88f, 0.95f);
        fanBlade[i].transform.position = glm::vec3(0.0f, 3.5f, 7.4f);
    }
}

void GameHub::buildBasketball() {
    makeBox(hoopBoard, "HoopBoard", glm::vec3(17.7f, 6.0f, 4.6f), glm::vec3(0.12f, 1.8f, 1.2f), glm::vec3(0.92f, 0.92f, 0.9f));
    hoopRim.name = "Rim";
    hoopRim.mesh = createCylinder(0.38f, 0.06f, 18);
    hoopRim.transform.position = glm::vec3(16.95f, 6.0f, 4.15f);
    hoopRim.color = glm::vec3(0.95f, 0.25f, 0.1f);

    hoopNet.name = "Net";
    hoopNet.mesh = createCylinder(0.32f, 0.55f, 10);
    hoopNet.transform.position = glm::vec3(16.95f, 6.0f, 3.85f);
    hoopNet.color = glm::vec3(0.9f, 0.9f, 0.95f);
    hoopNet.alpha = 0.35f;

    ballString.name = "String";
    ballString.mesh = createCylinder(0.025f, 1.3f, 8);
    ballString.color = glm::vec3(0.7f, 0.7f, 0.72f);

    basketball.name = "Basketball";
    basketball.mesh = createSphere(0.32f, 14, 20);
    basketball.color = glm::vec3(1.0f, 0.45f, 0.12f);
    basketball.mesh.loadTexture("basketball");
}

void GameHub::buildExterior() {
    sky.name = "Sky";
    sky.mesh = createCube(90.0f, 6.0f, 50.0f);
    sky.transform.position = glm::vec3(0.0f, 22.5f, 10.0f);
    sky.color = glm::vec3(0.45f, 0.7f, 0.95f);

    exteriorGround.name = "ExteriorGround";
    exteriorGround.mesh = createCube(90.0f, 16.0f, 0.4f);
    exteriorGround.transform.position = glm::vec3(0.0f, 22.0f, -0.15f);
    exteriorGround.color = glm::vec3(0.22f, 0.28f, 0.2f);

    sunMoon.name = "SunMoon";
    sunMoon.mesh = createSphere(1.6f, 12, 16);
    sunMoon.transform.position = glm::vec3(10.0f, 24.0f, 16.0f);
    sunMoon.color = glm::vec3(1.0f, 0.92f, 0.55f);

    buildings.clear();
    windows.clear();
    glm::vec3 spots[] = {
        glm::vec3(-10.0f, 20.0f, 4.0f),
        glm::vec3(-4.0f, 21.5f, 6.0f),
        glm::vec3(3.5f, 20.5f, 5.0f),
        glm::vec3(9.5f, 22.0f, 7.0f),
        glm::vec3(0.0f, 24.0f, 3.5f)
    };
    glm::vec3 sizes[] = {
        glm::vec3(4.0f, 3.0f, 8.0f),
        glm::vec3(3.2f, 2.6f, 12.0f),
        glm::vec3(3.6f, 2.8f, 10.0f),
        glm::vec3(4.4f, 3.2f, 14.0f),
        glm::vec3(5.0f, 2.4f, 7.0f)
    };
    for (int i = 0; i < 5; i++) {
        buildings.emplace_back("Building");
        buildings.back().mesh = createCube(sizes[i].x, sizes[i].y, sizes[i].z);
        buildings.back().transform.position = spots[i];
        buildings.back().color = glm::vec3(0.35f, 0.38f, 0.45f);
        buildings.back().mesh.loadTexture("brick");

        for (int wy = 0; wy < 3; wy++) {
            for (int wx = 0; wx < 2; wx++) {
                windows.emplace_back("Window");
                windows.back().mesh = createCube(0.45f, 0.08f, 0.55f);
                windows.back().transform.position = spots[i] + glm::vec3(-0.6f + wx * 1.1f, -sizes[i].y * 0.5f - 0.02f, -1.5f + wy * 1.4f);
                windows.back().color = glm::vec3(0.85f, 0.9f, 0.55f);
            }
        }
    }
}

void GameHub::buildSpotlightRig() {
    lampArm.name = "LampArm";
    lampArm.mesh = createCylinder(0.05f, 2.4f, 10);
    lampArm.transform.position = glm::vec3(0.0f, 0.0f, 6.6f);
    lampArm.color = glm::vec3(0.3f, 0.3f, 0.34f);

    lampHead.name = "LampHead";
    lampHead.mesh = createCylinder(0.45f, 0.35f, 16);
    lampHead.transform.position = glm::vec3(0.0f, 0.0f, 5.5f);
    lampHead.color = glm::vec3(0.15f, 0.15f, 0.16f);

    lampBulb.name = "LampBulb";
    lampBulb.mesh = createSphere(0.16f, 10, 12);
    lampBulb.transform.position = glm::vec3(0.0f, 0.0f, 5.28f);
    lampBulb.color = glm::vec3(1.0f, 0.95f, 0.75f);
}

void GameHub::toggleLights() {
    lightsOn = !lightsOn;
}

void GameHub::toggleFan() {
    fanOn = !fanOn;
}

void GameHub::setDayMode(bool day) {
    dayMode = day;
}

void GameHub::toggleCamera() {
    pinballCam = !pinballCam;
}

void GameHub::resetAll() {
    machine.resetGame();
}

Camera& GameHub::activeCamera() {
    return pinballCam ? machine.camera : hubCamera;
}

const Camera& GameHub::activeCamera() const {
    return pinballCam ? machine.camera : hubCamera;
}

void GameHub::updateFan(float dt) {
    if (fanOn) {
        fanAngle += dt * 180.0f;
        if (fanAngle > 3600.0f) fanAngle -= 3600.0f;
    }
    fanHub.transform.rotation = glm::vec3(0.0f, 0.0f, fanAngle);
    for (int i = 0; i < 4; i++) {
        float a = fanAngle + i * 90.0f;
        float rad = glm::radians(a);
        glm::vec3 offset(std::cos(rad) * 1.2f, std::sin(rad) * 1.2f, 0.0f);
        fanBlade[i].transform.position = fanHub.transform.position + offset;
        fanBlade[i].transform.rotation = glm::vec3(0.0f, 0.0f, a);
    }
}

void GameHub::updateBasketball(float dt) {
    basketballPhase += dt;
    float swing = std::sin(basketballPhase * 1.35f) * 0.55f;
    glm::vec3 anchor(17.55f, 4.4f, 6.3f);
    glm::vec3 ballPos = anchor + glm::vec3(-0.15f, swing, -1.15f);
    basketball.transform.position = ballPos;
    basketball.transform.rotation = glm::vec3(0.0f, 0.0f, swing * 25.0f);
    ballString.transform.position = (anchor + ballPos) * 0.5f;
    ballString.transform.rotation = glm::vec3(0.0f, 0.0f, swing * 18.0f);
}

void GameHub::update(float dt) {
    machine.update(dt);
    updateFan(dt);
    updateBasketball(dt);

    switchLight.color = lightsOn ? glm::vec3(0.25f, 1.0f, 0.4f) : glm::vec3(0.35f, 0.12f, 0.12f);
    switchFan.color = fanOn ? glm::vec3(0.25f, 0.75f, 1.0f) : glm::vec3(0.35f, 0.12f, 0.12f);
    switchLight.transform.position.x = lightsOn ? -17.52f : -17.68f;
    switchFan.transform.position.x = fanOn ? -17.52f : -17.68f;
    lampBulb.color = lightsOn ? glm::vec3(1.0f, 0.95f, 0.7f) : glm::vec3(0.25f, 0.22f, 0.18f);

    if (dayMode) {
        sky.color = glm::vec3(0.48f, 0.72f, 0.95f);
        sunMoon.color = glm::vec3(1.0f, 0.92f, 0.45f);
        sunMoon.transform.position = glm::vec3(12.0f, 24.0f, 16.5f);
        for (auto& w : windows) {
            w.color = glm::vec3(0.55f, 0.7f, 0.85f);
        }
        for (auto& b : buildings) {
            b.color = glm::vec3(0.42f, 0.44f, 0.5f);
        }
    } else {
        sky.color = glm::vec3(0.05f, 0.07f, 0.16f);
        sunMoon.color = glm::vec3(0.85f, 0.88f, 1.0f);
        sunMoon.transform.position = glm::vec3(-8.0f, 24.0f, 15.0f);
        for (auto& w : windows) {
            w.color = glm::vec3(1.0f, 0.85f, 0.4f);
        }
        for (auto& b : buildings) {
            b.color = glm::vec3(0.12f, 0.14f, 0.2f);
        }
    }
}

glm::vec3 GameHub::skyColor() const {
    if (dayMode) {
        return lightsOn ? glm::vec3(0.42f, 0.55f, 0.68f) : glm::vec3(0.32f, 0.45f, 0.6f);
    }
    return lightsOn ? glm::vec3(0.04f, 0.05f, 0.1f) : glm::vec3(0.015f, 0.02f, 0.05f);
}

void GameHub::applyLighting(unsigned int shader) const {
    const glm::vec3 viewPos = activeCamera().position;
    glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, &viewPos[0]);
    glUniform1f(glGetUniformLocation(shader, "shininess"), 48.0f);

    glm::vec3 ambient = dayMode ? glm::vec3(0.22f, 0.22f, 0.24f) : glm::vec3(0.05f, 0.06f, 0.1f);
    if (!lightsOn) {
        ambient *= dayMode ? 0.55f : 0.35f;
    }
    glUniform3fv(glGetUniformLocation(shader, "sceneAmbient"), 1, &ambient[0]);

    PointLight packed[4];
    packed[0] = roomLights[0];
    packed[1] = roomLights[1];
    packed[2] = machine.lights.empty() ? PointLight{} : machine.lights[2];
    packed[2].position = glm::vec3(0.0f, -2.0f, 3.4f);
    packed[2].setColor(glm::vec3(1.0f, 0.85f, 0.65f));
    packed[2].intensity = lightsOn ? 0.55f : 0.15f;
    packed[3] = PointLight{};
    packed[3].enabled = false;

    packed[0].enabled = lightsOn;
    packed[1].enabled = lightsOn;
    packed[0].intensity = lightsOn ? (dayMode ? 1.35f : 1.05f) : 0.0f;
    packed[1].intensity = lightsOn ? (dayMode ? 1.2f : 0.95f) : 0.0f;

    glUniform1i(glGetUniformLocation(shader, "numPointLights"), 3);
    for (int i = 0; i < 4; i++) {
        uploadLight(shader, i, packed[i]);
    }

    SpotLight spot = tableSpot;
    spot.enabled = lightsOn;
    spot.intensity = lightsOn ? (dayMode ? 1.8f : 2.1f) : 0.0f;
    glUniform3fv(glGetUniformLocation(shader, "spotLight.position"), 1, &spot.position[0]);
    glUniform3fv(glGetUniformLocation(shader, "spotLight.direction"), 1, &spot.direction[0]);
    glUniform3fv(glGetUniformLocation(shader, "spotLight.ambient"), 1, &spot.ambient[0]);
    glUniform3fv(glGetUniformLocation(shader, "spotLight.diffuse"), 1, &spot.diffuse[0]);
    glUniform3fv(glGetUniformLocation(shader, "spotLight.specular"), 1, &spot.specular[0]);
    glUniform1f(glGetUniformLocation(shader, "spotLight.intensity"), spot.intensity);
    glUniform1f(glGetUniformLocation(shader, "spotLight.cutOff"), spot.cutOff);
    glUniform1f(glGetUniformLocation(shader, "spotLight.outerCutOff"), spot.outerCutOff);
    glUniform1f(glGetUniformLocation(shader, "spotLight.exponent"), spot.exponent);
    glUniform1f(glGetUniformLocation(shader, "spotLight.kc"), spot.constant);
    glUniform1f(glGetUniformLocation(shader, "spotLight.kl"), spot.linear);
    glUniform1f(glGetUniformLocation(shader, "spotLight.kq"), spot.quadratic);
    glUniform1i(glGetUniformLocation(shader, "spotLight.enabled"), spot.enabled ? 1 : 0);
}

void GameHub::drawTransparent(unsigned int shader) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    hoopNet.draw(shader);
    glassWall.draw(shader);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void GameHub::render(unsigned int shaderProgram) {
    applyLighting(shaderProgram);

    sky.draw(shaderProgram);
    exteriorGround.draw(shaderProgram);
    sunMoon.draw(shaderProgram);
    for (auto& b : buildings) b.draw(shaderProgram);
    for (auto& w : windows) w.draw(shaderProgram);

    floor.draw(shaderProgram);
    ceiling.draw(shaderProgram);
    wallLeft.draw(shaderProgram);
    wallRight.draw(shaderProgram);
    wallBack.draw(shaderProgram);
    wallFrontLeft.draw(shaderProgram);
    wallFrontRight.draw(shaderProgram);
    trimLeft.draw(shaderProgram);
    trimRight.draw(shaderProgram);
    glassLintel.draw(shaderProgram);
    glassSill.draw(shaderProgram);
    rug.draw(shaderProgram);

    sofaSeat.draw(shaderProgram);
    sofaBack.draw(shaderProgram);
    sofaArmL.draw(shaderProgram);
    sofaArmR.draw(shaderProgram);
    couchSeat.draw(shaderProgram);
    couchBack.draw(shaderProgram);
    couchArmL.draw(shaderProgram);
    couchArmR.draw(shaderProgram);
    tableTop.draw(shaderProgram);
    tableLeg1.draw(shaderProgram);
    tableLeg2.draw(shaderProgram);
    tableLeg3.draw(shaderProgram);
    tableLeg4.draw(shaderProgram);

    switchPanel.draw(shaderProgram);
    switchLight.draw(shaderProgram);
    switchFan.draw(shaderProgram);
    switchLabelBar.draw(shaderProgram);

    fanRod.draw(shaderProgram);
    fanHub.draw(shaderProgram);
    for (int i = 0; i < 4; i++) fanBlade[i].draw(shaderProgram);

    hoopBoard.draw(shaderProgram);
    hoopRim.draw(shaderProgram);
    ballString.draw(shaderProgram);
    basketball.draw(shaderProgram);

    lampArm.draw(shaderProgram);
    lampHead.draw(shaderProgram);
    lampBulb.draw(shaderProgram);

    machine.render(shaderProgram, false);
    drawTransparent(shaderProgram);
}

void GameHub::renderHUD(int fbW, int fbH) {
    const char* shade = "PHONG";
    switch (machine.getShadingMode()) {
        case ShadingMode::FLAT: shade = "FLAT"; break;
        case ShadingMode::GOURAUD: shade = "GOURAUD"; break;
        default: shade = "PHONG"; break;
    }
    hud.renderConsole(fbW, fbH, machine.score, machine.lives, machine.ballNumber,
                      lightsOn, fanOn, dayMode, pinballCam, machine.gameOver, shade);
}
