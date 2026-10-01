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
    seatBall();

    bumpers.clear();
    bumpers.emplace_back(0.42f, glm::vec3(-1.45f, 2.9f, surface + 0.225f));
    bumpers.emplace_back(0.42f, glm::vec3(0.0f, 4.05f, surface + 0.225f));
    bumpers.emplace_back(0.42f, glm::vec3(1.45f, 2.9f, surface + 0.225f));
    bumpers[0].restColor = glm::vec3(0.92f, 0.22f, 0.38f);
    bumpers[1].restColor = glm::vec3(0.18f, 0.72f, 0.92f);
    bumpers[2].restColor = glm::vec3(0.95f, 0.72f, 0.18f);
    bumpers[0].points = 50;
    bumpers[1].points = 75;
    bumpers[2].points = 50;
    for (auto& bumper : bumpers) {
        bumper.color = bumper.restColor;
    }

    targets.clear();
    targets.emplace_back(glm::vec3(-2.2f, 4.6f, surface + 0.22f));
    targets.emplace_back(glm::vec3(2.2f, 4.6f, surface + 0.22f));
    targets.emplace_back(glm::vec3(-2.35f, 1.6f, surface + 0.22f));
    targets.emplace_back(glm::vec3(2.35f, 1.6f, surface + 0.22f));
    targets[1].restColor = glm::vec3(0.2f, 0.85f, 0.95f);
    targets[2].restColor = glm::vec3(0.95f, 0.3f, 0.7f);
    targets[3].restColor = glm::vec3(0.45f, 0.95f, 0.4f);
    for (auto& t : targets) {
        t.color = t.restColor;
    }

    leftFlipper = Flipper(true);
    rightFlipper = Flipper(false);
    leftFlipper.pivot.z = playfield.calculateZ(leftFlipper.pivot.y) + 0.12f;
    rightFlipper.pivot.z = playfield.calculateZ(rightFlipper.pivot.y) + 0.12f;
    leftFlipper.transform.position = leftFlipper.pivot;
    rightFlipper.transform.position = rightFlipper.pivot;

    plunger = Plunger();
    plunger.transform.position.z = playfield.calculateZ(plunger.transform.position.y) + 0.16f;

    lights.resize(3);
    glm::vec3 lightColors[3] = {
        glm::vec3(1.0f, 0.92f, 0.78f),
        glm::vec3(0.85f, 0.95f, 1.0f),
        glm::vec3(1.0f, 0.9f, 0.7f)
    };
    glm::vec3 lightPos[3] = {
        glm::vec3(-4.5f, -2.0f, 7.2f),
        glm::vec3(4.5f, 3.0f, 7.2f),
        glm::vec3(0.0f, -1.0f, 3.6f)
    };
    for (int i = 0; i < 3; i++) {
        lights[i].position = lightPos[i];
        lights[i].intensity = 1.25f;
        lights[i].setColor(lightColors[i]);
        lights[i].ambient = glm::vec3(0.18f);
        lights[i].constant = 1.0f;
        lights[i].linear = 0.04f;
        lights[i].quadratic = 0.006f;
        lights[i].enabled = true;
    }

    createMachineStructure();

    camera = Camera(glm::vec3(0.0f, -10.2f, 9.4f), glm::vec3(0.0f, 0.0f, 1.0f), 90.0f, -38.0f);
    camera.movementSpeed = 8.0f;
    camera.zoom = 42.0f;

    score = 0;
    lives = 3;
    ballNumber = 1;
    gameOver = false;
}

void PinballMachine::createMachineStructure() {
    const float surface = playfield.baseHeight;
    const float wallH = 1.15f;
    const float wallZ = surface + wallH * 0.5f;

    cabinet.name = "Cabinet";
    cabinet.mesh = createCube(8.4f, 14.4f, 0.55f);
    cabinet.transform.position = glm::vec3(0.0f, 0.0f, 0.28f);
    cabinet.color = glm::vec3(0.12f, 0.1f, 0.18f);
    cabinet.mesh.loadTexture("wood");

    base.name = "Base";
    base.mesh = createCube(8.2f, 14.2f, 0.22f);
    base.transform.position = glm::vec3(0.0f, 0.0f, surface - 0.08f);
    base.color = glm::vec3(0.22f, 0.22f, 0.26f);

    auto makeLeg = [](GameObject& leg, const glm::vec3& pos) {
        leg.mesh = createCylinder(0.14f, 0.7f, 12);
        leg.transform.position = pos;
        leg.color = glm::vec3(0.18f, 0.16f, 0.2f);
    };
    makeLeg(legFL, glm::vec3(-3.6f, -6.3f, 0.12f));
    makeLeg(legFR, glm::vec3(3.6f, -6.3f, 0.12f));
    makeLeg(legBL, glm::vec3(-3.6f, 6.3f, 0.12f));
    makeLeg(legBR, glm::vec3(3.6f, 6.3f, 0.12f));

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
    leftRail.mesh.loadTexture("metal");

    rightRail.name = "RightRail";
    rightRail.mesh = createCylinder(0.08f, 12.0f, 18);
    rightRail.transform.position = glm::vec3(3.72f, 0.0f, surface + 0.85f);
    rightRail.color = glm::vec3(0.75f, 0.76f, 0.8f);
    rightRail.mesh.loadTexture("metal");

    backbox.name = "Backbox";
    backbox.mesh = createCube(8.0f, 0.7f, 3.2f);
    backbox.transform.position = glm::vec3(0.0f, 6.7f, surface + 2.1f);
    backbox.color = glm::vec3(0.08f, 0.09f, 0.16f);

    backglass.name = "Backglass";
    backglass.mesh = createCube(7.2f, 0.12f, 2.4f);
    backglass.transform.position = glm::vec3(0.0f, 6.38f, surface + 2.15f);
    backglass.color = glm::vec3(0.55f, 0.35f, 1.0f);
    backglass.mesh.loadTexture("neon");

    glassCover.name = "GlassCover";
    glassCover.mesh = createCube(7.2f, 12.0f, 0.04f);
    glassCover.transform.position = glm::vec3(0.0f, 0.0f, surface + 1.12f);
    glassCover.color = glm::vec3(0.75f, 0.9f, 1.0f);
    glassCover.alpha = 0.10f;

    slingLeft.name = "SlingLeft";
    slingLeft.mesh = createCube(1.7f, 0.18f, 0.32f);
    slingLeft.transform.position = glm::vec3(-2.05f, -3.45f, surface + 0.18f);
    slingLeft.transform.rotation = glm::vec3(0.0f, 0.0f, -38.0f);
    slingLeft.color = glm::vec3(0.95f, 0.55f, 0.18f);

    slingRight.name = "SlingRight";
    slingRight.mesh = createCube(1.45f, 0.18f, 0.32f);
    slingRight.transform.position = glm::vec3(1.15f, -3.5f, surface + 0.18f);
    slingRight.transform.rotation = glm::vec3(0.0f, 0.0f, 38.0f);
    slingRight.color = glm::vec3(0.95f, 0.55f, 0.18f);

    laneDeflector.name = "LaneDeflector";
    laneDeflector.mesh = createCube(1.15f, 0.16f, 0.4f);
    laneDeflector.transform.position = glm::vec3(2.95f, -1.15f, surface + 0.22f);
    laneDeflector.transform.rotation = glm::vec3(0.0f, 0.0f, 48.0f);
    laneDeflector.color = glm::vec3(0.85f, 0.82f, 0.35f);

    apronLeft.name = "ApronLeft";
    apronLeft.mesh = createCube(1.7f, 0.16f, 0.22f);
    apronLeft.transform.position = glm::vec3(-2.15f, -5.35f, surface + 0.12f);
    apronLeft.transform.rotation = glm::vec3(0.0f, 0.0f, 18.0f);
    apronLeft.color = glm::vec3(0.28f, 0.22f, 0.55f);

    apronRight.name = "ApronRight";
    apronRight.mesh = createCube(1.15f, 0.16f, 0.22f);
    apronRight.transform.position = glm::vec3(1.55f, -5.4f, surface + 0.12f);
    apronRight.transform.rotation = glm::vec3(0.0f, 0.0f, -22.0f);
    apronRight.color = glm::vec3(0.28f, 0.22f, 0.55f);

    centerPost.name = "CenterPost";
    centerPost.mesh = createCylinder(0.12f, 0.38f, 12);
    centerPost.transform.position = glm::vec3(0.0f, -3.55f, surface + 0.2f);
    centerPost.color = glm::vec3(0.9f, 0.9f, 0.95f);
    centerPost.mesh.loadTexture("metal");
}

void PinballMachine::placeOnPlayfield(GameObject& obj, float heightOffset) {
    obj.transform.position.z = playfield.calculateZ(obj.transform.position.y) + heightOffset;
}

void PinballMachine::seatBall() {
    ball.sitOnPlayfield(playfield.calculateZ(ball.transform.position.y));
}

void PinballMachine::clampObjectToTable(glm::vec3& position, float radius) const {
    const float xmin = -tableHalfWidth() + radius + 0.12f;
    const float xmax = tableHalfWidth() - radius - 0.12f;
    const float ymin = -tableHalfLength() + radius + 0.12f;
    const float ymax = tableHalfLength() - radius - 0.12f;
    position.x = std::clamp(position.x, xmin, xmax);
    position.y = std::clamp(position.y, ymin, ymax);
}

void PinballMachine::addScore(int points) {
    if (!gameOver) {
        score += points;
    }
}

void PinballMachine::resetGame() {
    score = 0;
    lives = 3;
    ballNumber = 1;
    gameOver = false;
    ball.reset();
    seatBall();
    plunger.reset();
    plunger.transform.position.z = playfield.calculateZ(plunger.transform.position.y) + 0.16f;
    for (auto& t : targets) {
        t.down = false;
        t.cooldown = 0.0f;
        t.color = t.restColor;
    }
}

void PinballMachine::launchFromLane(float speed) {
    if (gameOver) {
        return;
    }
    ball.reset();
    seatBall();
    ball.velocity.y = speed;
    ball.velocity.x = -0.4f;
}

void PinballMachine::drainBall() {
    if (gameOver) {
        return;
    }
    lives--;
    if (lives <= 0) {
        lives = 0;
        gameOver = true;
        ball.reset();
        seatBall();
        ball.velocity = glm::vec3(0.0f);
        return;
    }
    ballNumber++;
    ball.reset();
    seatBall();
}

void PinballMachine::update(float dt) {
    dt = std::min(dt, 0.033f);
    if (flipperScoreCooldown > 0.0f) {
        flipperScoreCooldown -= dt;
    }

    playfield.update(dt);
    leftFlipper.update(dt);
    rightFlipper.update(dt);
    plunger.update(dt);

    for (auto& t : targets) {
        t.update(dt);
        t.transform.position.z = playfield.calculateZ(t.transform.position.y) + (t.down ? 0.06f : 0.22f);
    }

    if (!gameOver) {
        const float g = 9.81f;
        const float tilt = glm::radians(playfield.tiltAngle);
        ball.velocity.y -= g * std::sin(tilt) * dt;
        float damp = std::pow(0.999f, 60.0f * dt);
        ball.velocity *= damp;

        ball.update(dt);
        seatBall();
    }

    for (auto& bumper : bumpers) {
        bumper.update(dt);
        clampObjectToTable(bumper.transform.position, bumper.radius);
        bumper.transform.position.z = playfield.calculateZ(bumper.transform.position.y) + bumper.height * 0.5f;
    }

    clampObjectToTable(leftFlipper.pivot, 0.2f);
    clampObjectToTable(rightFlipper.pivot, 0.2f);
    leftFlipper.pivot.z = playfield.calculateZ(leftFlipper.pivot.y) + 0.12f;
    rightFlipper.pivot.z = playfield.calculateZ(rightFlipper.pivot.y) + 0.12f;
    leftFlipper.transform.position = leftFlipper.pivot;
    rightFlipper.transform.position = rightFlipper.pivot;

    clampObjectToTable(plunger.transform.position, 0.12f);
    plunger.transform.position.z = playfield.calculateZ(plunger.transform.position.y) + 0.16f;

    if (!gameOver) {
        checkCollisions();
        seatBall();
    }
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
        if (flipperScoreCooldown <= 0.0f) {
            addScore(5);
            flipperScoreCooldown = 0.12f;
        }
    }
}

void PinballMachine::checkCollisions() {
    for (auto& bumper : bumpers) {
        if (bumper.checkCollision(ball)) {
            addScore(bumper.points);
        }
    }
    for (auto& target : targets) {
        if (target.checkCollision(ball)) {
            addScore(target.points);
        }
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
    collideBallWithSegment(glm::vec2(-2.55f, -2.85f), glm::vec2(-1.15f, -4.45f), 0.1f, 0.85f);
    collideBallWithSegment(glm::vec2(1.15f, -4.45f), glm::vec2(2.15f, -2.9f), 0.1f, 0.85f);
    collideBallWithSegment(glm::vec2(2.55f, -1.55f), glm::vec2(3.35f, -0.35f), 0.1f, 0.55f);

    {
        glm::vec2 c(centerPost.transform.position.x, centerPost.transform.position.y);
        glm::vec2 p(ball.transform.position.x, ball.transform.position.y);
        glm::vec2 d = p - c;
        float dist = glm::length(d);
        float minD = ball.radius + 0.12f;
        if (dist < minD) {
            glm::vec2 n = (dist > 1e-5f) ? (d / dist) : glm::vec2(0.0f, 1.0f);
            ball.transform.position.x += n.x * (minD - dist);
            ball.transform.position.y += n.y * (minD - dist);
            ball.bounce(glm::vec3(n.x, n.y, 0.0f));
        }
    }

    bool inDrain = ball.transform.position.x > -1.15f && ball.transform.position.x < 1.15f;
    if (ball.transform.position.y < -wallY - r) {
        if (inDrain || ball.transform.position.x > 2.2f) {
            if (ball.transform.position.x > 2.2f) {
                ball.transform.position.y = -wallY + r;
                if (ball.velocity.y < 0.0f) ball.velocity.y = -ball.velocity.y * 0.2f;
            } else {
                drainBall();
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
    glm::vec3 ambient(0.18f, 0.18f, 0.22f);
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

void PinballMachine::render(unsigned int shaderProgram, bool withLighting) {
    if (withLighting) {
        applyLighting(shaderProgram);
    }

    cabinet.draw(shaderProgram);
    base.draw(shaderProgram);
    legFL.draw(shaderProgram);
    legFR.draw(shaderProgram);
    legBL.draw(shaderProgram);
    legBR.draw(shaderProgram);
    playfield.draw(shaderProgram);
    leftWall.draw(shaderProgram);
    rightWall.draw(shaderProgram);
    backWall.draw(shaderProgram);
    frontWallLeft.draw(shaderProgram);
    frontWallRight.draw(shaderProgram);
    laneWall.draw(shaderProgram);
    slingLeft.draw(shaderProgram);
    slingRight.draw(shaderProgram);
    laneDeflector.draw(shaderProgram);
    apronLeft.draw(shaderProgram);
    apronRight.draw(shaderProgram);
    centerPost.draw(shaderProgram);
    leftRail.draw(shaderProgram);
    rightRail.draw(shaderProgram);
    backbox.draw(shaderProgram);
    backglass.draw(shaderProgram);

    for (const auto& bumper : bumpers) {
        bumper.draw(shaderProgram);
    }
    for (const auto& target : targets) {
        target.draw(shaderProgram);
    }
    leftFlipper.draw(shaderProgram);
    rightFlipper.draw(shaderProgram);
    plunger.draw(shaderProgram);
    ball.draw(shaderProgram);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glassCover.draw(shaderProgram);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
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
            seatBall();
            break;
        case 1:
        case 2:
        case 3:
            bumpers[selectedObjectIndex - 1].transform.position += movement;
            clampObjectToTable(bumpers[selectedObjectIndex - 1].transform.position, bumpers[selectedObjectIndex - 1].radius);
            bumpers[selectedObjectIndex - 1].transform.position.z =
                playfield.calculateZ(bumpers[selectedObjectIndex - 1].transform.position.y) +
                bumpers[selectedObjectIndex - 1].height * 0.5f;
            break;
        case 4:
            leftFlipper.pivot += movement;
            clampObjectToTable(leftFlipper.pivot, 0.2f);
            leftFlipper.pivot.z = playfield.calculateZ(leftFlipper.pivot.y) + 0.12f;
            leftFlipper.transform.position = leftFlipper.pivot;
            break;
        case 5:
            rightFlipper.pivot += movement;
            clampObjectToTable(rightFlipper.pivot, 0.2f);
            rightFlipper.pivot.z = playfield.calculateZ(rightFlipper.pivot.y) + 0.12f;
            rightFlipper.transform.position = rightFlipper.pivot;
            break;
        case 6:
            plunger.transform.position += movement;
            clampObjectToTable(plunger.transform.position, 0.12f);
            plunger.transform.position.z = playfield.calculateZ(plunger.transform.position.y) + 0.16f;
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
            seatBall();
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
            leftFlipper.pivot.z = playfield.calculateZ(leftFlipper.pivot.y) + 0.12f;
            leftFlipper.transform.position = leftFlipper.pivot;
            break;
        case 5:
            rightFlipper.reset();
            rightFlipper.pivot.z = playfield.calculateZ(rightFlipper.pivot.y) + 0.12f;
            rightFlipper.transform.position = rightFlipper.pivot;
            break;
        case 6:
            plunger.reset();
            plunger.transform.position.z = playfield.calculateZ(plunger.transform.position.y) + 0.16f;
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
    if (gameOver) {
        return;
    }
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
