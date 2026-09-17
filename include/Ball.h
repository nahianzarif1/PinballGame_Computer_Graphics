#ifndef BALL_H
#define BALL_H

#include "GameObject.h"
#include <glm/glm.hpp>

class Ball : public GameObject {
public:
    glm::vec3 velocity{0.0f, 0.0f, 0.0f};
    float radius;
    float restitution;
    float maxSpeed{22.0f};

    Ball(float radius = 0.25f);

    void update(float dt) override;
    void draw(unsigned int shaderProgram) const override;
    void applyForce(const glm::vec3& force);
    void bounce(const glm::vec3& normal);
    void reset();
    void sitOnPlayfield(float surfaceZ);
};

#endif // BALL_H
