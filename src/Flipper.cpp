#include "Flipper.h"
#include "Mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>

Flipper::Flipper(bool isLeft, const glm::vec3& pivotParam) 
    : isLeft(isLeft), angle(0.0f) {
    name = isLeft ? "LeftFlipper" : "RightFlipper";
    mesh = createFlipper(1.5f, 0.25f, 0.15f);
    
    if (isLeft) {
        minAngle = -30.0f;
        maxAngle = 30.0f;
        color = glm::vec3(0.2f, 0.6f, 1.0f); // Blue
        pivot = glm::vec3(-1.5f, -4.0f, 1.3f);
    } else {
        minAngle = -30.0f;
        maxAngle = 30.0f;
        color = glm::vec3(0.2f, 0.6f, 1.0f); // Blue
        pivot = glm::vec3(1.5f, -4.0f, 1.3f);
    }
    
    angularSpeed = 180.0f; // Degrees per second
    transform.position = pivot;
}

void Flipper::update(float dt) {
    // Update is handled by rotation control
}

void Flipper::rotate(float direction) {
    angle += direction * angularSpeed * 0.016f; // Assume ~60fps
    
    if (angle < minAngle) angle = minAngle;
    if (angle > maxAngle) angle = maxAngle;
}

void Flipper::setAngle(float newAngle) {
    angle = newAngle;
    if (angle < minAngle) angle = minAngle;
    if (angle > maxAngle) angle = maxAngle;
}

glm::mat4 Flipper::getModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);
    
    // Translate to pivot
    model = glm::translate(model, pivot);
    
    // Rotate around Z axis
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
    
    // Scale
    model = glm::scale(model, transform.scale);
    
    return model;
}

void Flipper::reset() {
    angle = 0.0f;
}
