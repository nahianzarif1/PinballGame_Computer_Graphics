#include "Plunger.h"
#include "Mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

Plunger::Plunger(const glm::vec3& position) {
    name = "Plunger";
    mesh = createCylinder(0.15f, 1.0f, 16);
    transform.position = position;
    color = glm::vec3(0.8f, 0.6f, 0.4f); // Bronze
    
    minPosition = -6.0f;
    maxPosition = -4.0f;
    speed = 3.0f;
    springCompression = 0.0f;
    
    transform.position = glm::vec3(0.0f, minPosition, 1.1f);
}

void Plunger::update(float dt) {
    // Spring compression decay
    if (springCompression > 0.0f) {
        springCompression -= dt * 2.0f;
        if (springCompression < 0.0f) springCompression = 0.0f;
    }
}

void Plunger::extend() {
    if (transform.position.y < maxPosition) {
        transform.position.y += speed * 0.016f;
    }
}

void Plunger::retract() {
    if (transform.position.y > minPosition) {
        transform.position.y -= speed * 0.016f;
        springCompression = 1.0f;
    }
}

void Plunger::reset() {
    transform.position = glm::vec3(0.0f, minPosition, 1.1f);
    springCompression = 0.0f;
}
