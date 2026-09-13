#ifndef FLIPPER_H
#define FLIPPER_H

#include "GameObject.h"
#include <glm/glm.hpp>

class Flipper : public GameObject {
public:
    glm::vec3 pivot;
    float angle;
    float minAngle;
    float maxAngle;
    float angularSpeed;
    bool isLeft;
    
    Flipper(bool isLeft = true, const glm::vec3& pivot = glm::vec3(0.0f));
    
    void update(float dt) override;
    void rotate(float direction);
    void setAngle(float newAngle);
    glm::mat4 getModelMatrix() const;
    void draw(unsigned int shaderProgram) const override;
    void reset();
};

#endif // FLIPPER_H
