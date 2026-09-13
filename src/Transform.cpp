#include "Transform.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

glm::mat4 Transform::getModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);
    
    // Translate
    model = glm::translate(model, position);
    
    // Rotate (using quaternion for proper rotation order)
    glm::quat quat = glm::quat(glm::radians(rotation));
    model = model * glm::mat4_cast(quat);
    
    // Scale
    model = glm::scale(model, scale);
    
    return model;
}

void Transform::reset() {
    position = glm::vec3(0.0f, 0.0f, 0.0f);
    rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    scale = glm::vec3(1.0f, 1.0f, 1.0f);
}
