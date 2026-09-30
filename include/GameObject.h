#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Transform.h"
#include "Mesh.h"
#include <string>

class GameObject {
public:
    Transform transform;
    Mesh mesh;
    std::string name;
    glm::vec3 color;
    float alpha{1.0f};
    float emissiveStrength{0.0f};
    float shininess{32.0f};
    float reflectivity{0.0f}; // Recursive mirror contribution in ray mode.
    float refractiveIndex{1.5f}; // Glass; transmission derives from alpha.

    GameObject(const std::string& name = "GameObject");
    virtual ~GameObject() = default;
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;
    GameObject(GameObject&&) noexcept = default;
    GameObject& operator=(GameObject&&) noexcept = default;

    virtual void update(float dt);
    virtual void draw(unsigned int shaderProgram) const;
    void setColor(const glm::vec3& newColor);
};

#endif // GAMEOBJECT_H
