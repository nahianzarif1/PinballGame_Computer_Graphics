#include "Flipper.h"
#include "Mesh.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

Flipper::Flipper(bool isLeft, const glm::vec3& /*pivotParam*/)
    : isLeft(isLeft), angle(0.0f), angularVelocity(0.0f), powered(false) {
    name = isLeft ? "LeftFlipper" : "RightFlipper";
    length = 1.55f;
    width = 0.22f;
    height = 0.16f;
    mesh = createCube(length, width, height);

    if (isLeft) {
        restAngle = -28.0f;
        activeAngle = 48.0f;
        color = glm::vec3(0.25f, 0.62f, 1.0f);
        pivot = glm::vec3(-1.85f, -4.55f, 1.08f);
    } else {
        restAngle = 28.0f;
        activeAngle = -48.0f;
        color = glm::vec3(0.25f, 0.62f, 1.0f);
        pivot = glm::vec3(1.85f, -4.55f, 1.08f);
    }

    angle = restAngle;
    angularSpeed = 720.0f;
    transform.position = pivot;
}

void Flipper::update(float dt) {
    float target = powered ? activeAngle : restAngle;
    float prev = angle;
    float diff = target - angle;
    float maxStep = angularSpeed * dt;

    if (std::abs(diff) <= maxStep) {
        angle = target;
    } else {
        angle += (diff > 0.0f ? 1.0f : -1.0f) * maxStep;
    }

    float lo = std::min(restAngle, activeAngle);
    float hi = std::max(restAngle, activeAngle);
    angle = std::clamp(angle, lo, hi);

    if (dt > 1e-6f) {
        angularVelocity = (angle - prev) / dt;
    } else {
        angularVelocity = 0.0f;
    }
}

void Flipper::rotate(float direction) {
    angle += direction * 4.0f;
    float lo = std::min(restAngle, activeAngle);
    float hi = std::max(restAngle, activeAngle);
    angle = std::clamp(angle, lo, hi);
}

void Flipper::setPowered(bool on) {
    powered = on;
}

void Flipper::setAngle(float newAngle) {
    float lo = std::min(restAngle, activeAngle);
    float hi = std::max(restAngle, activeAngle);
    angle = std::clamp(newAngle, lo, hi);
}

glm::vec2 Flipper::alongDirection() const {
    float rad = glm::radians(angle);
    if (isLeft) {
        return glm::vec2(std::cos(rad), std::sin(rad));
    }
    return glm::vec2(-std::cos(rad), -std::sin(rad));
}

glm::mat4 Flipper::getModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pivot);
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
    float offsetX = isLeft ? (length * 0.5f) : (-length * 0.5f);
    model = glm::translate(model, glm::vec3(offsetX, 0.0f, 0.0f));
    model = glm::scale(model, transform.scale);
    return model;
}

void Flipper::draw(unsigned int shaderProgram) const {
    glm::mat4 model = getModelMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, &color[0]);
    mesh.draw();
}

void Flipper::reset() {
    if (isLeft) {
        pivot = glm::vec3(-1.85f, -4.55f, 1.08f);
    } else {
        pivot = glm::vec3(1.85f, -4.55f, 1.08f);
    }
    angle = restAngle;
    angularVelocity = 0.0f;
    powered = false;
    transform.position = pivot;
}
