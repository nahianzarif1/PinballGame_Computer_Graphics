#include "Ball.h"
#include "Mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

Ball::Ball(float radius) : radius(radius), restitution(0.9f) {
    name = "Ball";
    mesh = createSphere(radius, 16, 24);
    color = glm::vec3(0.95f, 0.95f, 0.95f); // Silver ball
    transform.position = glm::vec3(0.0f, -4.0f, 1.5f);
}

void Ball::update(float dt) {
    transform.position += velocity * dt;
    
    // Simple gravity
    velocity.z -= 9.8f * dt;
    
    // Keep ball above ground
    if (transform.position.z < radius) {
        transform.position.z = radius;
        velocity.z = -velocity.z * restitution;
    }
}

void Ball::applyForce(const glm::vec3& force) {
    velocity += force;
}

void Ball::bounce(const glm::vec3& normal) {
    // V' = V - 2(V·N)N
    float dotProduct = glm::dot(velocity, normal);
    velocity = velocity - 2.0f * dotProduct * normal;
    velocity *= restitution; // Energy loss
}

void Ball::reset() {
    transform.position = glm::vec3(0.0f, -4.0f, 1.5f);
    velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
}
