#include "Playfield.h"
#include "Mesh.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <cmath>

const float PI = 3.14159265358979323846f;

Playfield::Playfield(float width, float length, float tiltAngle) 
    : width(width), length(length), tiltAngle(tiltAngle) {
    name = "Playfield";
    mesh = createPlane(width, length);
    baseHeight = 1.2f;
    
    transform.position = glm::vec3(0.0f, 0.0f, baseHeight);
    color = glm::vec3(0.1f, 0.4f, 0.2f); // Forest green
}

float Playfield::calculateZ(float y) const {
    // z = z0 + y * sin(theta)
    return baseHeight + y * sin(glm::radians(tiltAngle));
}

glm::vec3 Playfield::localToWorld(const glm::vec3& localPos) const {
    float z = calculateZ(localPos.y);
    return glm::vec3(localPos.x, localPos.y, z);
}

void Playfield::update(float dt) {
    // Apply tilt rotation to transform
    transform.rotation.x = tiltAngle;
}

void Playfield::draw(unsigned int shaderProgram) const {
    glm::mat4 model = glm::mat4(1.0f);
    
    // Apply position
    model = glm::translate(model, transform.position);
    
    // Apply tilt rotation around X axis
    model = glm::rotate(model, glm::radians(tiltAngle), glm::vec3(1.0f, 0.0f, 0.0f));
    
    // Apply scale
    model = glm::scale(model, transform.scale);
    
    // Set uniforms directly using OpenGL
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, &color[0]);
    
    mesh.draw();
}
