#include "Ball.h"
#include "Mesh.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <algorithm>

Ball::Ball(float radius) : radius(radius), restitution(0.72f) {
    name = "Ball";
    mesh = createSphere(radius, 18, 28);
    color = glm::vec3(0.92f, 0.93f, 0.96f);
    transform.position = glm::vec3(2.95f, -4.6f, 1.25f);
}

void Ball::update(float dt) {
    transform.position += velocity * dt;

    float speed = glm::length(velocity);
    if (speed > maxSpeed) {
        velocity *= maxSpeed / speed;
    }
}

void Ball::applyForce(const glm::vec3& force) {
    velocity += force;
}

void Ball::bounce(const glm::vec3& normal) {
    glm::vec3 n = glm::normalize(normal);
    float vn = glm::dot(velocity, n);
    if (vn >= 0.0f) {
        return;
    }
    velocity = velocity - (1.0f + restitution) * vn * n;
}

void Ball::reset() {
    transform.position = glm::vec3(2.95f, -4.6f, 1.25f);
    velocity = glm::vec3(0.0f);
    transform.rotation = glm::vec3(0.0f);
}

void Ball::sitOnPlayfield(float surfaceZ) {
    transform.position.z = surfaceZ + radius;
}

void Ball::draw(unsigned int shaderProgram) const {
    glm::mat4 model = transform.getModelMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, &color[0]);
    mesh.draw();
}
