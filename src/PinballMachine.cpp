#include "PinballMachine.h"
#include "Mesh.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <string>

PinballMachine::PinballMachine() {
    initialize();
}

void PinballMachine::initialize() {
    playfield = Playfield(7.0f, 12.0f, 8.0f);
    const float surface = playfield.baseHeight;

    ball = Ball(0.28f);
    ball.reset();
    ball.sitOnPlayfield(surface);

    bumpers.clear();
    bumpers.emplace_back(0.42f, glm::vec3(-1.45f, 2.9f, surface + 0.225f));
    bumpers.emplace_back(0.42f, glm::vec3(0.0f, 4.05f, surface + 0.225f));
    bumpers.emplace_back(0.42f, glm::vec3(1.45f, 2.9f, surface + 0.225f));
    bumpers[0].restColor = glm::vec3(0.92f, 0.22f, 0.38f);
    bumpers[1].restColor = glm::vec3(0.18f, 0.72f, 0.92f);
    bumpers[2].restColor = glm::vec3(0.95f, 0.72f, 0.18f);
    for (auto& bumper : bumpers) {
        bumper.color = bumper.restColor;
    }

    leftFlipper = Flipper(true);
    rightFlipper = Flipper(false);
    leftFlipper.pivot.z = surface + 0.12f;
    rightFlipper.pivot.z = surface + 0.12f;
    leftFlipper.transform.position = leftFlipper.pivot;
    rightFlipper.transform.position = rightFlipper.pivot;

    plunger = Plunger();
    plunger.transform.position.z = surface + 0.16f;

    lights.resize(3);
    glm::vec3 lightColors[3] = {
        glm::vec3(1.0f, 0.85f, 0.75f),
        glm::vec3(0.85f, 0.95f, 1.0f),
        glm::vec3(1.0f, 0.9f, 0.7f)
    };
    glm::vec3 lightPos[3] = {
        glm::vec3(-2.2f, -1.0f, surface + 4.5f),
        glm::vec3(2.2f, 2.5f, surface + 4.8f),
        glm::vec3(0.0f, -4.0f, surface + 3.8f)
    };
    for (int i = 0; i < 3; i++) {
        lights[i].position = lightPos[i];
        lights[i].intensity = 1.35f;
        lights[i].setColor(lightColors[i]);
        lights[i].ambient = glm::vec3(0.22f);
        lights[i].constant = 1.0f;
        lights[i].linear = 0.045f;
        lights[i].quadratic = 0.0075f;
        lights[i].enabled = true;
    }

    createMachineStructure();

    camera = Camera(glm::vec3(0.0f, -16.5f, 12.5f), glm::vec3(0.0f, 0.0f, 1.0f), 90.0f, -36.0f);
    camera.movementSpeed = 8.0f;
    camera.zoom = 42.0f;
}

void PinballMachine::createMachineStructure() {
    const float surface = playfield.baseHeight;
    const float wallH = 1.15f;
    const float wallZ = surface + wallH * 0.5f;

    base.name = "Base";
    base.mesh = createCube(8.2f, 14.2f, 0.7f);
    base.transform.position = glm::vec3(0.0f, 0.0f, 0.35f);
    base.color = glm::vec3(0.22f, 0.22f, 0.26f);

    leftWall.name = "LeftWall";
    leftWall.mesh = createCube(0.22f, 12.0f, wallH);
    leftWall.transform.position = glm::vec3(-3.61f, 0.0f, wallZ);
    leftWall.color = glm::vec3(0.38f, 0.38f, 0.44f);

    rightWall.name = "RightWall";
    rightWall.mesh = createCube(0.22f, 12.0f, wallH);
    rightWall.transform.position = glm::vec3(3.61f, 0.0f, wallZ);
    rightWall.color = glm::vec3(0.38f, 0.38f, 0.44f);

    backWall.name = "BackWall";
    backWall.mesh = createCube(7.44f, 0.22f, wallH);
    backWall.transform.position = glm::vec3(0.0f, 6.11f, wallZ);
    backWall.color = glm::vec3(0.38f, 0.38f, 0.44f);

    frontWallLeft.name = "FrontWallLeft";
    frontWallLeft.mesh = createCube(2.4f, 0.22f, wallH);
    frontWallLeft.transform.position = glm::vec3(-2.4f, -6.11f, wallZ);
    frontWallLeft.color = glm::vec3(0.38f, 0.38f, 0.44f);

    frontWallRight.name = "FrontWallRight";
    frontWallRight.mesh = createCube(1.35f, 0.22f, wallH);
    frontWallRight.transform.position = glm::vec3(2.85f, -6.11f, wallZ);
    frontWallRight.color = glm::vec3(0.38f, 0.38f, 0.44f);

    laneWall.name = "LaneWall";
    laneWall.mesh = createCube(0.16f, 4.6f, wallH);
    laneWall.transform.position = glm::vec3(2.42f, -3.7f, wallZ);
    laneWall.color = glm::vec3(0.45f, 0.45f, 0.5f);

    leftRail.name = "LeftRail";
    leftRail.mesh = createCylinder(0.08f, 12.0f, 18);
    leftRail.transform.position = glm::vec3(-3.72f, 0.0f, surface + 0.85f);
    leftRail.color = glm::vec3(0.75f, 0.76f, 0.8f);

    rightRail.name = "RightRail";
    rightRail.mesh = createCylinder(0.08f, 12.0f, 18);
    rightRail.transform.position = glm::vec3(3.72f, 0.0f, surface + 0.85f);
    rightRail.color = glm::vec3(0.75f, 0.76f, 0.8f);
}

void PinballMachine::placeOnPlayfield(GameObject& obj, float heightOffset) {
    obj.transform.position.z = playfield.calculateZ(obj.transform.position.y) + heightOffset;
}

void PinballMachine::clampObjectToTable(glm::vec3& position, float radius) const {
    const float xmin = -tableHalfWidth() + radius + 0.12f;
    const float xmax = tableHalfWidth() - radius - 0.12f;
    const float ymin = -tableHalfLength() + radius + 0.12f;
    const float ymax = tableHalfLength() - radius - 0.12f;
    position.x = std::clamp(position.x, xmin, xmax);
    position.y = std::clamp(position.y, ymin, ymax);
}

void PinballMachine::update(float dt) {
    dt = std::min(dt, 0.033f);

    playfield.update(dt);
    leftFlipper.update(dt);
    rightFlipper.update(dt);

    plunger.update(dt);

    const float g = 9.81f;
    const float tilt = glm::radians(playfield.tiltAngle);
    ball.velocity.y -= g * std::sin(tilt) * dt;
    ball.velocity *= (1.0f - 0.28f * dt);

    ball.update(dt);
    ball.sitOnPlayfield(playfield.baseHeight);

    for (auto& bumper : bumpers) {
        bumper.update(dt);
        clampObjectToTable(bumper.transform.position, bumper.radius);
        bumper.transform.position.z = playfield.baseHeight + bumper.height * 0.5f;
    }

    clampObjectToTable(leftFlipper.pivot, 0.2f);
    clampObjectToTable(rightFlipper.pivot, 0.2f);
    leftFlipper.pivot.z = playfield.baseHeight + 0.12f;
    rightFlipper.pivot.z = playfield.baseHeight + 0.12f;
    leftFlipper.transform.position = leftFlipper.pivot;
    rightFlipper.transform.position = rightFlipper.pivot;

    clampObjectToTable(plunger.transform.position, 0.12f);
    plunger.transform.position.z = playfield.baseHeight + 0.16f;

    checkCollisions();
    ball.sitOnPlayfield(playfield.baseHeight);
}

void PinballMachine::collideBallWithSegment(const glm::vec2& a, const glm::vec2& b, float radius, float bounce) {
    glm::vec2 p(ball.transform.position.x, ball.transform.position.y);
    glm::vec2 ab = b - a;
    float abLen2 = glm::dot(ab, ab);
    if (abLen2 < 1e-8f) {
        return;
    }
    float t = std::clamp(glm::dot(p - a, ab) / abLen2, 0.0f, 1.0f);
    glm::vec2 closest = a + t * ab;
    glm::vec2 delta = p - closest;
    float dist = glm::length(delta);
    float minDist = ball.radius + radius;
    if (dist >= minDist) {
        return;
    }
    glm::vec2 n = (dist > 1e-5f) ? (delta / dist) : glm::vec2(0.0f, 1.0f);
    float overlap = minDist - std::max(dist, 1e-5f);
    ball.transform.position.x += n.x * overlap;
    ball.transform.position.y += n.y * overlap;
    float saved = ball.restitution;
    ball.restitution = bounce;
    ball.bounce(glm::vec3(n.x, n.y, 0.0f));
    ball.restitution = saved;
}

void PinballMachine::collideBallWithFlipper(Flipper& flipper) {
    glm::vec2 pivot(flipper.pivot.x, flipper.pivot.y);
    glm::vec2 along = flipper.alongDirection();
    glm::vec2 tip = pivot + along * flipper.length;
    glm::vec2 p(ball.transform.position.x, ball.transform.position.y);
    glm::vec2 ab = tip - pivot;
    float abLen2 = glm::dot(ab, ab);
    float t = std::clamp(glm::dot(p - pivot, ab) / abLen2, 0.0f, 1.0f);
    glm::vec2 closest = pivot + t * ab;
    glm::vec2 delta = p - closest;
    float dist = glm::length(delta);
    float minDist = ball.radius + flipper.width * 0.5f;
    if (dist >= minDist) {
        return;
    }
    glm::vec2 n = (dist > 1e-5f) ? (delta / dist) : glm::vec2(0.0f, 1.0f);
    float overlap = minDist - std::max(dist, 1e-5f);
    ball.transform.position.x += n.x * overlap;
    ball.transform.position.y += n.y * overlap;

    float saved = ball.restitution;
    ball.restitution = 0.35f;
    ball.bounce(glm::vec3(n.x, n.y, 0.0f));
    ball.restitution = saved;

    float omega = glm::radians(flipper.angularVelocity);
    glm::vec2 r = closest - pivot;
    glm::vec2 vFlip(-omega * r.y, omega * r.x);
    ball.velocity.x += vFlip.x * 1.35f;
    ball.velocity.y += vFlip.y * 1.35f;
    if (flipper.powered) {
        ball.velocity += glm::vec3(n.x, n.y, 0.0f) * 3.5f;
    }
}

void PinballMachine::checkCollisions() {
    for (auto& bumper : bumpers) {
        bumper.checkCollision(ball);
    }

    collideBallWithFlipper(leftFlipper);
    collideBallWithFlipper(rightFlipper);

    const float wallX = tableHalfWidth() - 0.11f;
    const float wallY = tableHalfLength() - 0.11f;
    const float r = ball.radius;

    if (ball.transform.position.x > wallX - r) {
        ball.transform.position.x = wallX - r;
        if (ball.velocity.x > 0.0f) ball.velocity.x = -ball.velocity.x * 0.55f;
    }
    if (ball.transform.position.x < -wallX + r) {
        ball.transform.position.x = -wallX + r;
        if (ball.velocity.x < 0.0f) ball.velocity.x = -ball.velocity.x * 0.55f;
    }
    if (ball.transform.position.y > wallY - r) {
        ball.transform.position.y = wallY - r;
        if (ball.velocity.y > 0.0f) ball.velocity.y = -ball.velocity.y * 0.55f;
    }

    collideBallWithSegment(glm::vec2(2.42f, -6.0f), glm::vec2(2.42f, -1.4f), 0.08f, 0.4f);

    bool inDrain = ball.transform.position.x > -1.15f && ball.transform.position.x < 1.15f;
    if (ball.transform.position.y < -wallY - r) {
        if (inDrain || ball.transform.position.x > 2.2f) {
            if (ball.transform.position.x > 2.2f) {
                ball.transform.position.y = -wallY + r;
                if (ball.velocity.y < 0.0f) ball.velocity.y = -ball.velocity.y * 0.2f;
            } else {
                ball.reset();
                ball.sitOnPlayfield(playfield.baseHeight);
            }
        } else {
            ball.transform.position.y = -wallY + r;
            if (ball.velocity.y < 0.0f) ball.velocity.y = -ball.velocity.y * 0.35f;
        }
    }

    if (plunger.pulling && ball.transform.position.x > 2.45f && ball.transform.position.y < -3.4f) {
        float stopY = plunger.transform.position.y + 0.7f;
        if (ball.transform.position.y < stopY + r) {
            ball.transform.position.y = stopY + r;
            ball.velocity = glm::vec3(0.0f);
        }
    }
}

void PinballMachine::applyLighting(unsigned int shaderProgram) const {
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, &camera.position[0]);
    glUniform1i(glGetUniformLocation(shaderProgram, "numPointLights"), static_cast<int>(lights.size()));
    glUniform1f(glGetUniformLocation(shaderProgram, "shininess"), 48.0f);
    glm::vec3 ambient(0.22f, 0.22f, 0.24f);
    glUniform3fv(glGetUniformLocation(shaderProgram, "sceneAmbient"), 1, &ambient[0]);

    for (int i = 0; i < static_cast<int>(lights.size()); i++) {
        std::string prefix = "pointLights[" + std::to_string(i) + "].";
        const PointLight& light = lights[i];
        glUniform3fv(glGetUniformLocation(shaderProgram, (prefix + "position").c_str()), 1, &light.position[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, (prefix + "ambient").c_str()), 1, &light.ambient[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, (prefix + "diffuse").c_str()), 1, &light.diffuse[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, (prefix + "specular").c_str()), 1, &light.specular[0]);
        glUniform1f(glGetUniformLocation(shaderProgram, (prefix + "intensity").c_str()), light.intensity);
        glUniform1f(glGetUniformLocation(shaderProgram, (prefix + "kc").c_str()), light.constant);
        glUniform1f(glGetUniformLocation(shaderProgram, (prefix + "kl").c_str()), light.linear);
        glUniform1f(glGetUniformLocation(shaderProgram, (prefix + "kq").c_str()), light.quadratic);
        glUniform1i(glGetUniformLocation(shaderProgram, (prefix + "enabled").c_str()), light.enabled ? 1 : 0);
    }
}

void PinballMachine::render(unsigned int shaderProgram) {
    applyLighting(shaderProgram);

    base.draw(shaderProgram);
    playfield.draw(shaderProgram);
    leftWall.draw(shaderProgram);
    rightWall.draw(shaderProgram);
    backWall.draw(shaderProgram);
    frontWallLeft.draw(shaderProgram);
    frontWallRight.draw(shaderProgram);
    laneWall.draw(shaderProgram);
    leftRail.draw(shaderProgram);
    rightRail.draw(shaderProgram);

    for (const auto& bumper : bumpers) {
        bumper.draw(shaderProgram);
    }
    leftFlipper.draw(shaderProgram);
    rightFlipper.draw(shaderProgram);
    plunger.draw(shaderProgram);
    ball.draw(shaderProgram);
}

void PinballMachine::selectNextObject() {
    selectedObjectIndex = (selectedObjectIndex + 1) % 7;
    selectionMode = SelectionMode::OBJECT;
}

void PinballMachine::moveSelectedObject(const glm::vec3& delta) {
    if (selectionMode != SelectionMode::OBJECT) return;

    glm::vec3 movement = delta * 2.0f * 0.016f;

    switch (selectedObjectIndex) {
        case 0:
            ball.transform.position += movement;
            ball.velocity = glm::vec3(0.0f);
            clampObjectToTable(ball.transform.position, ball.radius);
            ball.sitOnPlayfield(playfield.baseHeight);
            break;
        case 1:
        case 2:
        case 3:
            bumpers[selectedObjectIndex - 1].transform.position += movement;
            clampObjectToTable(bumpers[selectedObjectIndex - 1].transform.position, bumpers[selectedObjectIndex - 1].radius);
            bumpers[selectedObjectIndex - 1].transform.position.z = playfield.baseHeight + bumpers[selectedObjectIndex - 1].height * 0.5f;
            break;
        case 4:
            leftFlipper.pivot += movement;
            clampObjectToTable(leftFlipper.pivot, 0.2f);
            leftFlipper.pivot.z = playfield.baseHeight + 0.12f;
            leftFlipper.transform.position = leftFlipper.pivot;
            break;
        case 5:
            rightFlipper.pivot += movement;
            clampObjectToTable(rightFlipper.pivot, 0.2f);
            rightFlipper.pivot.z = playfield.baseHeight + 0.12f;
            rightFlipper.transform.position = rightFlipper.pivot;
            break;
        case 6:
            plunger.transform.position += movement;
            clampObjectToTable(plunger.transform.position, 0.12f);
            plunger.transform.position.z = playfield.baseHeight + 0.16f;
            break;
    }
}

void PinballMachine::rotateSelectedObject(float delta) {
    if (selectionMode != SelectionMode::OBJECT) return;
    if (selectedObjectIndex == 4) leftFlipper.rotate(delta);
    if (selectedObjectIndex == 5) rightFlipper.rotate(delta);
}

void PinballMachine::resetSelectedObject() {
    if (selectionMode != SelectionMode::OBJECT) return;
    const float surface = playfield.baseHeight;

    switch (selectedObjectIndex) {
        case 0:
            ball.reset();
            ball.sitOnPlayfield(surface);
            break;
        case 1:
            bumpers[0].transform.position = glm::vec3(-1.45f, 2.9f, surface + 0.225f);
            break;
        case 2:
            bumpers[1].transform.position = glm::vec3(0.0f, 4.05f, surface + 0.225f);
            break;
        case 3:
            bumpers[2].transform.position = glm::vec3(1.45f, 2.9f, surface + 0.225f);
            break;
        case 4:
            leftFlipper.reset();
            leftFlipper.pivot.z = surface + 0.12f;
            leftFlipper.transform.position = leftFlipper.pivot;
            break;
        case 5:
            rightFlipper.reset();
            rightFlipper.pivot.z = surface + 0.12f;
            rightFlipper.transform.position = rightFlipper.pivot;
            break;
        case 6:
            plunger.reset();
            plunger.transform.position.z = surface + 0.16f;
            break;
    }
}

void PinballMachine::selectLight(int index) {
    if (index >= 0 && index < static_cast<int>(lights.size())) {
        selectedLightIndex = index;
        selectionMode = SelectionMode::LIGHT;
    }
}

void PinballMachine::moveSelectedLight(const glm::vec3& delta) {
    if (selectionMode != SelectionMode::LIGHT) return;
    lights[selectedLightIndex].position += delta * 2.0f * 0.016f;
}

void PinballMachine::adjustLightIntensity(float delta) {
    if (selectionMode != SelectionMode::LIGHT) return;
    lights[selectedLightIndex].intensity = std::clamp(lights[selectedLightIndex].intensity + delta * 0.1f, 0.0f, 3.0f);
}

void PinballMachine::resetSelectedLight() {
    if (selectionMode != SelectionMode::LIGHT) return;
    lights[selectedLightIndex].position = bumpers[selectedLightIndex].transform.position + glm::vec3(0.0f, 0.0f, 3.5f);
    lights[selectedLightIndex].intensity = 1.35f;
}

void PinballMachine::setShadingMode(ShadingMode mode) {
    shadingMode = mode;
}

void PinballMachine::setLeftFlipperPowered(bool on) {
    leftFlipper.setPowered(on);
}

void PinballMachine::setRightFlipperPowered(bool on) {
    rightFlipper.setPowered(on);
}

void PinballMachine::setPlungerPulling(bool on) {
    if (!on && plunger.pulling) {
        float launch = plunger.releaseLaunchSpeed();
        bool inLane = ball.transform.position.x > 2.45f && ball.transform.position.y < -3.2f;
        if (inLane) {
            ball.velocity.y = launch;
            ball.velocity.x = -0.4f;
        }
        return;
    }
    plunger.setPulling(on);
}
