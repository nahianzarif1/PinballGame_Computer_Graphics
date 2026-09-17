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
