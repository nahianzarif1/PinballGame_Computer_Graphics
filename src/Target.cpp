#include "Target.h"
#include "Ball.h"
#include "Mesh.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

Target::Target(const glm::vec3& position) {
    name = "Target";
    mesh = createCube(halfW * 2.0f, halfD * 2.0f, height);
    transform.position = position;
    restColor = glm::vec3(0.95f, 0.55f, 0.15f);
    color = restColor;
}

void Target::update(float dt) {
    if (cooldown > 0.0f) {
        cooldown -= dt;
        if (cooldown <= 0.0f) {
            down = false;
            color = restColor;
        }
    }
}

bool Target::checkCollision(Ball& ball) {
    if (down) {
        return false;
    }
    glm::vec2 c(transform.position.x, transform.position.y);
    glm::vec2 p(ball.transform.position.x, ball.transform.position.y);
    glm::vec2 d = p - c;
    float nx = std::clamp(d.x, -halfW, halfW);
    float ny = std::clamp(d.y, -halfD, halfD);
    glm::vec2 closest = c + glm::vec2(nx, ny);
    glm::vec2 delta = p - closest;
    float dist = glm::length(delta);
    if (dist >= ball.radius) {
        return false;
    }
    glm::vec2 n = (dist > 1e-5f) ? (delta / dist) : glm::vec2(0.0f, -1.0f);
    float overlap = ball.radius - std::max(dist, 1e-5f);
    ball.transform.position.x += n.x * overlap;
    ball.transform.position.y += n.y * overlap;
    ball.bounce(glm::vec3(n.x, n.y, 0.0f));
    down = true;
    cooldown = 1.6f;
    color = glm::vec3(0.25f, 1.0f, 0.45f);
    return true;
}

void Target::draw(unsigned int shaderProgram) const {
    glm::mat4 model = transform.getModelMatrix();
    if (down) {
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.18f));
        model = glm::rotate(model, glm::radians(70.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    }
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, &color[0]);
    glUniform1f(glGetUniformLocation(shaderProgram, "objectAlpha"), 1.0f);
    glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0);
    mesh.draw();
}
