#ifndef TARGET_H
#define TARGET_H

#include "GameObject.h"
#include <glm/glm.hpp>

class Target : public GameObject {
public:
    float halfW{0.28f};
    float halfD{0.08f};
    float height{0.42f};
    int points{100};
    bool down{false};
    float cooldown{0.0f};
    glm::vec3 restColor{0.95f, 0.55f, 0.15f};

    Target(const glm::vec3& position = glm::vec3(0.0f));

    void update(float dt) override;
    void draw(unsigned int shaderProgram) const override;
    bool checkCollision(class Ball& ball);
};

#endif
