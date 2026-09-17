#include "Playfield.h"
#include "Mesh.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <cmath>

Playfield::Playfield(float width, float length, float tiltAngle)
    : width(width), length(length), tiltAngle(tiltAngle) {
    name = "Playfield";
    mesh = createPlane(width, length);
    baseHeight = 0.72f;
    transform.position = glm::vec3(0.0f, 0.0f, baseHeight);
    color = glm::vec3(0.18f, 0.48f, 0.28f);
}

float Playfield::calculateZ(float y) const {
    (void)y;
    return baseHeight;
}

glm::vec3 Playfield::localToWorld(const glm::vec3& localPos) const {
    return glm::vec3(localPos.x, localPos.y, baseHeight + localPos.z);
}

void Playfield::update(float dt) {
    (void)dt;
}

void Playfield::draw(unsigned int shaderProgram) const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, baseHeight));
    model = glm::scale(model, transform.scale);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, &color[0]);
    mesh.draw();
}
