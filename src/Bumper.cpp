#include "Bumper.h"
#include "Ball.h"
#include "Mesh.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <cmath>

Bumper::Bumper(float radius, const glm::vec3& position)
    : radius(radius), height(0.45f), hit(false), hitTimer(0.0f) {
    name = "Bumper";
    mesh = createCylinder(radius, height, 28);
    transform.position = position;
    restColor = glm::vec3(1.0f, 0.3f, 0.3f);
    color = restColor;
}

void Bumper::update(float dt) {
    if (hitTimer > 0.0f) {
        hitTimer -= dt;
        if (hitTimer <= 0.0f) {
            hit = false;
            color = restColor;
        }
    }
}

bool Bumper::checkCollision(Ball& ball) {
    glm::vec2 bumperXY(transform.position.x, transform.position.y);
    glm::vec2 ballXY(ball.transform.position.x, ball.transform.position.y);
    glm::vec2 delta = ballXY - bumperXY;
    float distance = glm::length(delta);
    float minDistance = radius + ball.radius;

    if (distance < 1e-5f) {
        delta = glm::vec2(0.0f);
    }

    if (distance < minDistance) {
        glm::vec2 normal = distance > 1e-5f ? delta / distance : glm::vec2(0.0f, 1.0f);
        float overlap = minDistance - distance;
        ball.transform.position.x += normal.x * overlap;
        ball.transform.position.y += normal.y * overlap;

        glm::vec3 n(normal.x, normal.y, 0.0f);
        float incoming = glm::dot(ball.velocity, n);
        ball.bounce(n);
        if (hitTimer > 0.0f || incoming >= -0.05f) return false;
        ball.velocity += n * impulse;
        onHit();
        return true;
    }
    return false;
}

void Bumper::onHit() {
    hit = true;
    hitTimer = 0.18f;
    color = glm::vec3(1.0f, 1.0f, 0.45f);
}

void Bumper::draw(unsigned int shaderProgram) const {
    glm::mat4 model = transform.getModelMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, &color[0]);
    glUniform1f(glGetUniformLocation(shaderProgram, "shininess"), 48.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "objectAlpha"), 1.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "emissiveStrength"), hit ? 1.0f : 0.15f);
    glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0);
    mesh.draw();
}
