#ifndef PLAYFIELD_H
#define PLAYFIELD_H

#include "GameObject.h"
#include <glm/glm.hpp>

class Playfield : public GameObject {
public:
    float width;
    float length;
    float tiltAngle; // In degrees
    float baseHeight; // z0
    
    Playfield(float width = 7.0f, float length = 12.0f, float tiltAngle = 8.0f);
    
    float calculateZ(float y) const;
    glm::vec3 localToWorld(const glm::vec3& localPos) const;
    void update(float dt) override;
};

#endif // PLAYFIELD_H
