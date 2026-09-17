#ifndef FLIPPER_H
#define FLIPPER_H

#include "GameObject.h"
#include <glm/glm.hpp>

class Flipper : public GameObject {
public:
    glm::vec3 pivot;
    float angle;
    float restAngle;
    float activeAngle;
    float angularSpeed;
    float angularVelocity;
    float length;
    float width;
    float height;
    bool isLeft;
    bool powered;

    Flipper(bool isLeft = true, const glm::vec3& pivot = glm::vec3(0.0f));

    void update(float dt) override;
    void rotate(float direction);
    void setPowered(bool on);
    void setAngle(float newAngle);
    glm::mat4 getModelMatrix() const;
    void draw(unsigned int shaderProgram) const override;
    void reset();
    glm::vec2 alongDirection() const;
};

#endif // FLIPPER_H
