#ifndef BUMPER_H
#define BUMPER_H

#include "GameObject.h"
#include <glm/glm.hpp>

class Bumper : public GameObject {
public:
    float radius;
    float height;
    bool hit;
    float hitTimer;
    glm::vec3 restColor;
    float impulse{8.5f};
    int points{50};

    Bumper(float radius = 0.4f, const glm::vec3& position = glm::vec3(0.0f));

    void update(float dt) override;
    void draw(unsigned int shaderProgram) const override;
    bool checkCollision(class Ball& ball);
    void onHit();
    bool isHit() const { return hitTimer > 0.0f; }
};

#endif
