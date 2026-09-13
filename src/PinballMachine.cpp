#include "PinballMachine.h"
#include "Mesh.h"
#include "Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>

PinballMachine::PinballMachine() {
    initialize();
}

void PinballMachine::initialize() {
    // Initialize playfield
    playfield = Playfield(7.0f, 12.0f, 5.0f); // Reduced tilt for better visibility
    
    // Initialize ball
    ball = Ball(0.25f);
    ball.transform.position = glm::vec3(0.0f, -4.0f, playfield.calculateZ(-4.0f) + 0.5f);
    
    // Initialize bumpers at specified positions
    bumpers.resize(3);
    bumpers[0] = Bumper(0.4f, glm::vec3(-1.5f, 2.0f, playfield.calculateZ(2.0f)));
    bumpers[1] = Bumper(0.4f, glm::vec3(0.0f, 3.5f, playfield.calculateZ(3.5f)));
    bumpers[2] = Bumper(0.4f, glm::vec3(1.5f, 2.0f, playfield.calculateZ(2.0f)));
    
    // Set bumper colors (modern palette)
    bumpers[0].color = glm::vec3(1.0f, 0.2f, 0.4f); // Pink
    bumpers[1].color = glm::vec3(0.2f, 0.8f, 1.0f); // Cyan
    bumpers[2].color = glm::vec3(1.0f, 0.8f, 0.2f); // Amber
    
    // Initialize flippers
    leftFlipper = Flipper(true);
    rightFlipper = Flipper(false);
    
    // Initialize plunger
    plunger = Plunger();
    
    // Initialize lights at bumper positions
    lights.resize(3);
    for (int i = 0; i < 3; i++) {
        lights[i].position = bumpers[i].transform.position + glm::vec3(0.0f, 0.0f, 1.0f);
        lights[i].intensity = 1.0f;
        
        // Set light colors to match bumpers
        lights[i].diffuse = bumpers[i].color;
        lights[i].specular = bumpers[i].color;
    }
    
    // Create machine structure
    createMachineStructure();
    
    // Initialize camera - positioned to view entire machine
    camera = Camera(glm::vec3(0.0f, -2.0f, 15.0f), glm::vec3(0.0f, 0.0f, 1.0f), -90.0f, -60.0f);
}

void PinballMachine::createMachineStructure() {
    // Base - dark charcoal
    base.name = "Base";
    base.mesh = createCube(8.0f, 14.0f, 1.0f);
    base.transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    base.color = glm::vec3(0.15f, 0.15f, 0.18f);
    
    // Left wall - metallic
    leftWall.name = "LeftWall";
    leftWall.mesh = createCube(0.3f, 12.0f, 1.5f);
    leftWall.transform.position = glm::vec3(-3.65f, 0.0f, 0.75f);
    leftWall.color = glm::vec3(0.4f, 0.4f, 0.45f);
    
    // Right wall - metallic
    rightWall.name = "RightWall";
    rightWall.mesh = createCube(0.3f, 12.0f, 1.5f);
    rightWall.transform.position = glm::vec3(3.65f, 0.0f, 0.75f);
    rightWall.color = glm::vec3(0.4f, 0.4f, 0.45f);
    
    // Back wall - metallic
    backWall.name = "BackWall";
    backWall.mesh = createCube(7.0f, 0.3f, 1.5f);
    backWall.transform.position = glm::vec3(0.0f, 6.15f, 0.75f);
    backWall.color = glm::vec3(0.4f, 0.4f, 0.45f);
    
    // Left rail - chrome
    leftRail.name = "LeftRail";
    leftRail.mesh = createCylinder(0.08f, 12.0f, 24);
    leftRail.transform.position = glm::vec3(-3.3f, 0.0f, 1.0f);
    leftRail.color = glm::vec3(0.7f, 0.7f, 0.75f);
    
    // Right rail - chrome
    rightRail.name = "RightRail";
    rightRail.mesh = createCylinder(0.08f, 12.0f, 24);
    rightRail.transform.position = glm::vec3(3.3f, 0.0f, 1.0f);
    rightRail.color = glm::vec3(0.7f, 0.7f, 0.75f);
}

void PinballMachine::update(float dt) {
    playfield.update(dt);
    ball.update(dt);
    
    for (auto& bumper : bumpers) {
        bumper.update(dt);
    }
    
    leftFlipper.update(dt);
    rightFlipper.update(dt);
    plunger.update(dt);
    
    checkCollisions();
    
    // Update Z positions for playfield objects
    updateObjectZFromPlayfield(ball);
    for (auto& bumper : bumpers) {
        updateObjectZFromPlayfield(bumper);
    }
}

void PinballMachine::updateObjectZFromPlayfield(GameObject& obj) {
    obj.transform.position.z = playfield.calculateZ(obj.transform.position.y);
}

void PinballMachine::checkCollisions() {
    // Ball vs Bumpers
    for (auto& bumper : bumpers) {
        bumper.checkCollision(ball);
    }
    
    // Ball vs Walls (simple boundary check)
    float wallX = 3.3f;
    float wallY = 5.8f;
    
    if (ball.transform.position.x > wallX - ball.radius) {
        ball.transform.position.x = wallX - ball.radius;
        ball.velocity.x = -ball.velocity.x * ball.restitution;
    }
    if (ball.transform.position.x < -wallX + ball.radius) {
        ball.transform.position.x = -wallX + ball.radius;
        ball.velocity.x = -ball.velocity.x * ball.restitution;
    }
    if (ball.transform.position.y > wallY - ball.radius) {
        ball.transform.position.y = wallY - ball.radius;
        ball.velocity.y = -ball.velocity.y * ball.restitution;
    }
    if (ball.transform.position.y < -wallY + ball.radius) {
        ball.transform.position.y = -wallY + ball.radius;
        ball.velocity.y = -ball.velocity.y * ball.restitution;
    }
}

void PinballMachine::render(unsigned int shaderProgram) {
    // Shader is already active from main.cpp, don't call glUseProgram here
    
    // Set light uniforms
    GLint numLightsLoc = glGetUniformLocation(shaderProgram, "numPointLights");
    glUniform1i(numLightsLoc, lights.size());
    for (size_t i = 0; i < lights.size(); i++) {
        std::string prefix = "pointLights[" + std::to_string(i) + "].";
        glUniform3fv(glGetUniformLocation(shaderProgram, (prefix + "position").c_str()), 1, &lights[i].position[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, (prefix + "ambient").c_str()), 1, &lights[i].ambient[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, (prefix + "diffuse").c_str()), 1, &lights[i].diffuse[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, (prefix + "specular").c_str()), 1, &lights[i].specular[0]);
        glUniform1f(glGetUniformLocation(shaderProgram, (prefix + "intensity").c_str()), lights[i].intensity);
        glUniform1f(glGetUniformLocation(shaderProgram, (prefix + "constant").c_str()), lights[i].constant);
        glUniform1f(glGetUniformLocation(shaderProgram, (prefix + "linear").c_str()), lights[i].linear);
        glUniform1f(glGetUniformLocation(shaderProgram, (prefix + "quadratic").c_str()), lights[i].quadratic);
        glUniform1i(glGetUniformLocation(shaderProgram, (prefix + "enabled").c_str()), lights[i].enabled);
    }
    
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, &camera.position[0]);
    glUniform1f(glGetUniformLocation(shaderProgram, "shininess"), 32.0f);
    
    // Render machine structure
    base.draw(shaderProgram);
    leftWall.draw(shaderProgram);
    rightWall.draw(shaderProgram);
    backWall.draw(shaderProgram);
    leftRail.draw(shaderProgram);
    rightRail.draw(shaderProgram);
    
    // Render playfield
    playfield.draw(shaderProgram);
    
    // Render bumpers
    for (auto& bumper : bumpers) {
        bumper.draw(shaderProgram);
    }
    
    // Render flippers
    leftFlipper.draw(shaderProgram);
    rightFlipper.draw(shaderProgram);
    
    // Render plunger
    plunger.draw(shaderProgram);
    
    // Render ball
    ball.draw(shaderProgram);
    
    // Render light visualization (small spheres)
    for (size_t i = 0; i < lights.size(); i++) {
        if (lights[i].enabled) {
            Mesh lightSphere = createSphere(0.1f, 8, 8);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, lights[i].position);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
            glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, &lights[i].diffuse[0]);
            lightSphere.draw();
        }
    }
}

void PinballMachine::selectNextObject() {
    selectedObjectIndex = (selectedObjectIndex + 1) % 7;
    selectionMode = SelectionMode::OBJECT;
}

void PinballMachine::moveSelectedObject(const glm::vec3& delta) {
    if (selectionMode != SelectionMode::OBJECT) return;
    
    float speed = 2.0f;
    glm::vec3 movement = delta * speed * 0.016f;
    
    switch (selectedObjectIndex) {
        case 0: // Ball
            ball.transform.position += movement;
            ball.velocity = glm::vec3(0.0f);
            break;
        case 1: // Bumper 1
            bumpers[0].transform.position += movement;
            break;
        case 2: // Bumper 2
            bumpers[1].transform.position += movement;
            break;
        case 3: // Bumper 3
            bumpers[2].transform.position += movement;
            break;
        case 4: // Left Flipper
            leftFlipper.pivot += movement;
            leftFlipper.transform.position = leftFlipper.pivot;
            break;
        case 5: // Right Flipper
            rightFlipper.pivot += movement;
            rightFlipper.transform.position = rightFlipper.pivot;
            break;
        case 6: // Plunger
            plunger.transform.position += movement;
            break;
    }
}

void PinballMachine::rotateSelectedObject(float delta) {
    if (selectionMode != SelectionMode::OBJECT) return;
    
    switch (selectedObjectIndex) {
        case 4: // Left Flipper
            leftFlipper.rotate(delta);
            break;
        case 5: // Right Flipper
            rightFlipper.rotate(delta);
            break;
    }
}

void PinballMachine::resetSelectedObject() {
    if (selectionMode != SelectionMode::OBJECT) return;
    
    switch (selectedObjectIndex) {
        case 0: // Ball
            ball.reset();
            break;
        case 1: // Bumper 1
            bumpers[0].transform.position = glm::vec3(-1.5f, 3.0f, playfield.calculateZ(3.0f));
            break;
        case 2: // Bumper 2
            bumpers[1].transform.position = glm::vec3(0.0f, 4.2f, playfield.calculateZ(4.2f));
            break;
        case 3: // Bumper 3
            bumpers[2].transform.position = glm::vec3(1.5f, 3.0f, playfield.calculateZ(3.0f));
            break;
        case 4: // Left Flipper
            leftFlipper.reset();
            break;
        case 5: // Right Flipper
            rightFlipper.reset();
            break;
        case 6: // Plunger
            plunger.reset();
            break;
    }
}

void PinballMachine::selectLight(int index) {
    if (index >= 0 && index < lights.size()) {
        selectedLightIndex = index;
        selectionMode = SelectionMode::LIGHT;
    }
}

void PinballMachine::moveSelectedLight(const glm::vec3& delta) {
    if (selectionMode != SelectionMode::LIGHT) return;
    
    float speed = 2.0f;
    lights[selectedLightIndex].position += delta * speed * 0.016f;
}

void PinballMachine::adjustLightIntensity(float delta) {
    if (selectionMode != SelectionMode::LIGHT) return;
    
    lights[selectedLightIndex].intensity += delta * 0.1f;
    if (lights[selectedLightIndex].intensity < 0.0f) lights[selectedLightIndex].intensity = 0.0f;
    if (lights[selectedLightIndex].intensity > 3.0f) lights[selectedLightIndex].intensity = 3.0f;
}

void PinballMachine::resetSelectedLight() {
    if (selectionMode != SelectionMode::LIGHT) return;
    
    lights[selectedLightIndex].position = bumpers[selectedLightIndex].transform.position + glm::vec3(0.0f, 0.0f, 1.0f);
    lights[selectedLightIndex].intensity = 1.0f;
}

void PinballMachine::setShadingMode(ShadingMode mode) {
    shadingMode = mode;
}
