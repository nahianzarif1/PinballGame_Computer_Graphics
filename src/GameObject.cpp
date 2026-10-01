#include "GameObject.h"
#include "Shader.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

GameObject::GameObject(const std::string& name) : name(name), color(1.0f, 1.0f, 1.0f) {}

void GameObject::update(float dt) {
    // Base update - can be overridden
}

void GameObject::draw(unsigned int shaderProgram) const {
    glm::mat4 model = transform.getModelMatrix();

    // Set uniforms directly using OpenGL
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, &color[0]);

    mesh.draw();
}

void GameObject::render(unsigned int shaderProgram) const {
    draw(shaderProgram);
}

void GameObject::setColor(const glm::vec3& newColor) {
    color = newColor;
}
