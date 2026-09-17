#include "Plunger.h"
#include "Mesh.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <algorithm>

Plunger::Plunger(const glm::vec3& /*position*/) {
    name = "Plunger";
    mesh = createCylinder(0.09f, 1.15f, 18);
    color = glm::vec3(0.78f, 0.52f, 0.28f);

    minPosition = -5.85f;
    maxPosition = -4.35f;
    speed = 4.5f;
    springCompression = 0.0f;

    transform.position = glm::vec3(2.95f, minPosition, 1.15f);
    transform.rotation = glm::vec3(90.0f, 0.0f, 0.0f);
}

void Plunger::update(float dt) {
    if (pulling) {
        springCompression = std::min(1.0f, springCompression + dt * 1.6f);
        transform.position.y = minPosition - springCompression * 0.55f;
    } else if (springCompression > 0.0f) {
        springCompression = std::max(0.0f, springCompression - dt * 8.0f);
        transform.position.y = minPosition - springCompression * 0.55f;
    } else {
        transform.position.y = minPosition;
    }
}

void Plunger::setPulling(bool on) {
    pulling = on;
}

float Plunger::releaseLaunchSpeed() {
    float power = springCompression;
    pulling = false;
    return 6.0f + power * 16.0f;
}

void Plunger::reset() {
    transform.position = glm::vec3(2.95f, minPosition, 1.15f);
    transform.rotation = glm::vec3(90.0f, 0.0f, 0.0f);
    springCompression = 0.0f;
    pulling = false;
}

void Plunger::draw(unsigned int shaderProgram) const {
    glm::mat4 model = transform.getModelMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, &color[0]);
    mesh.draw();
}
