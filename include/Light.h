#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

struct PointLight {
    glm::vec3 position{0.0f, 0.0f, 0.0f};

    glm::vec3 ambient{0.1f, 0.1f, 0.1f};
    glm::vec3 diffuse{1.0f, 1.0f, 1.0f};
    glm::vec3 specular{1.0f, 1.0f, 1.0f};

    float intensity{1.0f};

    float constant{1.0f};
    float linear{0.09f};
    float quadratic{0.032f};

    bool enabled{true};

    void reset() {
        intensity = 1.0f;
        enabled = true;
    }

    void setColor(const glm::vec3& color) {
        ambient = color * 0.25f;
        diffuse = color;
        specular = glm::vec3(1.0f);
    }
};

struct SpotLight {
    glm::vec3 position{0.0f, 0.0f, 7.8f};
    glm::vec3 direction{0.0f, 0.0f, -1.0f};

    glm::vec3 ambient{0.05f, 0.05f, 0.04f};
    glm::vec3 diffuse{1.0f, 0.95f, 0.8f};
    glm::vec3 specular{1.0f, 1.0f, 1.0f};

    float intensity{1.6f};
    float cutOff{38.0f};
    float outerCutOff{48.0f};
    float exponent{20.0f};

    float constant{1.0f};
    float linear{0.045f};
    float quadratic{0.0075f};

    bool enabled{true};
};

#endif // LIGHT_H
