#include "PinballMachine.h"
#include "Mesh.h"
#include "Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>

const float PI = 3.14159265358979323846f;

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
        lights[i].ambient = bumpers[i].color * 0.2f;
        lights[i].diffuse = bumpers[i].color;
        lights[i].specular = bumpers[i].color;
        
        // Attenuation values
        lights[i].constant = 1.0f;
        lights[i].linear = 0.09f;
        lights[i].quadratic = 0.032f;
        lights[i].enabled = true;
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
    
    std::cout << "Machine structure created with " << base.mesh.vertices.size() << " base vertices" << std::endl;
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

Mesh PinballMachine::createCube(float width, float height, float depth) {
    Mesh mesh;
    
    float hw = width / 2.0f;
    float hh = height / 2.0f;
    float hd = depth / 2.0f;
    
    // 8 vertices of a cube
    glm::vec3 positions[8] = {
        glm::vec3(-hw, -hh, -hd),
        glm::vec3( hw, -hh, -hd),
        glm::vec3( hw,  hh, -hd),
        glm::vec3(-hw,  hh, -hd),
        glm::vec3(-hw, -hh,  hd),
        glm::vec3( hw, -hh,  hd),
        glm::vec3( hw,  hh,  hd),
        glm::vec3(-hw,  hh,  hd)
    };
    
    // 6 faces with normals
    glm::vec3 normals[6] = {
        glm::vec3(0, 0, -1), // Front
        glm::vec3(0, 0,  1), // Back
        glm::vec3(-1, 0, 0), // Left
        glm::vec3( 1, 0, 0), // Right
        glm::vec3(0, -1, 0), // Bottom
        glm::vec3(0,  1, 0)  // Top
    };
    
    unsigned int faceIndices[6][4] = {
        {0, 1, 2, 3}, // Front
        {4, 7, 6, 5}, // Back
        {0, 4, 7, 3}, // Left
        {1, 5, 6, 2}, // Right
        {0, 4, 5, 1}, // Bottom
        {3, 7, 6, 2}  // Top
    };
    
    glm::vec3 baseColor(1.0f, 1.0f, 1.0f);
    
    for (int face = 0; face < 6; face++) {
        // Two triangles per face
        int i0 = faceIndices[face][0];
        int i1 = faceIndices[face][1];
        int i2 = faceIndices[face][2];
        int i3 = faceIndices[face][3];
        
        // Triangle 1
        mesh.vertices.push_back({positions[i0], normals[face], baseColor});
        mesh.vertices.push_back({positions[i1], normals[face], baseColor});
        mesh.vertices.push_back({positions[i2], normals[face], baseColor});
        
        // Triangle 2
        mesh.vertices.push_back({positions[i0], normals[face], baseColor});
        mesh.vertices.push_back({positions[i2], normals[face], baseColor});
        mesh.vertices.push_back({positions[i3], normals[face], baseColor});
    }
    
    mesh.setupMesh();
    return mesh;
}

Mesh PinballMachine::createCylinder(float radius, float height, int segments) {
    Mesh mesh;
    
    float halfHeight = height / 2.0f;
    glm::vec3 color(1.0f, 1.0f, 1.0f);
    
    // Generate vertices for cylinder sides
    for (int i = 0; i < segments; i++) {
        float theta0 = 2.0f * PI * i / segments;
        float theta1 = 2.0f * PI * (i + 1) / segments;
        
        float x0 = radius * cos(theta0);
        float y0 = radius * sin(theta0);
        float x1 = radius * cos(theta1);
        float y1 = radius * sin(theta1);
        
        // Normal for side (pointing outward)
        glm::vec3 normal0 = glm::normalize(glm::vec3(x0, y0, 0.0f));
        glm::vec3 normal1 = glm::normalize(glm::vec3(x1, y1, 0.0f));
        
        // Two triangles per segment
        // Triangle 1
        mesh.vertices.push_back({glm::vec3(x0, y0, -halfHeight), normal0, color});
        mesh.vertices.push_back({glm::vec3(x1, y1, -halfHeight), normal1, color});
        mesh.vertices.push_back({glm::vec3(x1, y1,  halfHeight), normal1, color});
        
        // Triangle 2
        mesh.vertices.push_back({glm::vec3(x0, y0, -halfHeight), normal0, color});
        mesh.vertices.push_back({glm::vec3(x1, y1,  halfHeight), normal1, color});
        mesh.vertices.push_back({glm::vec3(x0, y0,  halfHeight), normal0, color});
    }
    
    // Top cap
    glm::vec3 topNormal(0.0f, 0.0f, 1.0f);
    for (int i = 0; i < segments; i++) {
        float theta0 = 2.0f * PI * i / segments;
        float theta1 = 2.0f * PI * (i + 1) / segments;
        
        float x0 = radius * cos(theta0);
        float y0 = radius * sin(theta0);
        float x1 = radius * cos(theta1);
        float y1 = radius * sin(theta1);
        
        mesh.vertices.push_back({glm::vec3(0.0f, 0.0f, halfHeight), topNormal, color});
        mesh.vertices.push_back({glm::vec3(x0, y0, halfHeight), topNormal, color});
        mesh.vertices.push_back({glm::vec3(x1, y1, halfHeight), topNormal, color});
    }
    
    // Bottom cap
    glm::vec3 bottomNormal(0.0f, 0.0f, -1.0f);
    for (int i = 0; i < segments; i++) {
        float theta0 = 2.0f * PI * i / segments;
        float theta1 = 2.0f * PI * (i + 1) / segments;
        
        float x0 = radius * cos(theta0);
        float y0 = radius * sin(theta0);
        float x1 = radius * cos(theta1);
        float y1 = radius * sin(theta1);
        
        mesh.vertices.push_back({glm::vec3(0.0f, 0.0f, -halfHeight), bottomNormal, color});
        mesh.vertices.push_back({glm::vec3(x1, y1, -halfHeight), bottomNormal, color});
        mesh.vertices.push_back({glm::vec3(x0, y0, -halfHeight), bottomNormal, color});
    }
    
    mesh.setupMesh();
    return mesh;
}
