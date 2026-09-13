#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

struct PointLight {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    
    glm::vec3 ambient{0.1f, 0.1f, 0.1f};
    glm::vec3 diffuse{1.0f, 1.0f, 1.0f};
    glm::vec3 specular{1.0f, 1.0f, 1.0f};
    
    float intensity{1.0f};
    
    // Attenuation coefficients
    float constant{1.0f};
    float linear{0.09f};
    float quadratic{0.032f};
    
    bool enabled{true};
    
    void reset();
    void setColor(const glm::vec3& color);
};

#endif // LIGHT_H
