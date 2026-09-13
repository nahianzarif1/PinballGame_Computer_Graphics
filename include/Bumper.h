#ifndef BUMPER_H
#define BUMPER_H

#include "GameObject.h"
#include <glm/glm.hpp>

class Bumper : public GameObject {
public:
    float radius;
    bool hit;
    float hitTimer;
    
    Bumper(float radius = 0.4f, const glm::vec3& position = glm::vec3(0.0f));
    
    void update(float dt) override;
    void checkCollision(class Ball& ball);
    void onHit();
    bool isHit() const { return hitTimer > 0.0f; }
};

#endif // BUMPER_H
