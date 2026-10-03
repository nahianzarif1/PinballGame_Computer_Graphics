#ifndef PLUNGER_H
#define PLUNGER_H

#include "GameObject.h"
#include <glm/glm.hpp>

class Plunger : public GameObject {
public:
    float minPosition;
    float maxPosition;
    float speed;
    float springCompression;
    bool pulling{false};

    Plunger(const glm::vec3& position = glm::vec3(0.0f));

    void update(float dt) override;
    void draw(unsigned int shaderProgram) const override;
    void setPulling(bool on);
    float releaseLaunchSpeed();
    void reset();
};

#endif // PLUNGER_H
