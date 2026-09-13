#include "Bumper.h"
#include "Ball.h"
#include "Mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <cmath>

Bumper::Bumper(float radius, const glm::vec3& position) 
    : radius(radius), hit(false), hitTimer(0.0f) {
    name = "Bumper";
    mesh = createCylinder(radius, 0.6f, 24);
    transform.position = position;
    color = glm::vec3(1.0f, 0.3f, 0.3f); // Red bumper
}

void Bumper::update(float dt) {
    if (hitTimer > 0.0f) {
        hitTimer -= dt;
        if (hitTimer <= 0.0f) {
            hit = false;
            color = glm::vec3(1.0f, 0.3f, 0.3f); // Return to red
        }
    }
}

void Bumper::checkCollision(Ball& ball) {
    glm::vec3 bumperCenter = transform.position;
    glm::vec3 ballCenter = ball.transform.position;
    
    float distance = glm::length(ballCenter - bumperCenter);
    float minDistance = radius + ball.radius;
    
    if (distance < minDistance) {
        // Collision detected
        glm::vec3 normal = glm::normalize(ballCenter - bumperCenter);
        
        // Move ball out of collision
        float overlap = minDistance - distance;
        ball.transform.position += normal * overlap;
        
        // Bounce
        ball.bounce(normal);
        
        onHit();
    }
}

void Bumper::onHit() {
    hit = true;
    hitTimer = 0.2f; // Flash for 0.2 seconds
    color = glm::vec3(1.0f, 1.0f, 0.3f); // Flash yellow
}
