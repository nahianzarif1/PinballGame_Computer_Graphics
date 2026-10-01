#include "GameObject.h"
#include "Shader.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

GameObject::GameObject(const std::string& name) : name(name), color(1.0f, 1.0f, 1.0f), alpha(1.0f) {}

void GameObject::update(float dt) {
    (void)dt;
}

void GameObject::draw(unsigned int shaderProgram) const {
    glm::mat4 model = transform.getModelMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, &color[0]);
    glUniform1f(glGetUniformLocation(shaderProgram, "objectAlpha"), alpha);
    glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), mesh.hasTexture ? 1 : 0);
    if (mesh.hasTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh.textureID);
        glUniform1i(glGetUniformLocation(shaderProgram, "textureSampler"), 0);
    }
    mesh.draw();
}

void GameObject::setColor(const glm::vec3& newColor) {
    color = newColor;
}
