#ifndef MESH_H
#define MESH_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 texCoord{0.0f, 0.0f};
};

class Mesh {
public:
    unsigned int VAO, VBO, EBO;
    unsigned int textureID;
    bool hasTexture;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    Mesh();
    ~Mesh();
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void setupMesh();
    void draw() const;
    void clear();
    void loadTexture(const std::string& name);
};

Mesh createCube(float width = 1.0f, float height = 1.0f, float depth = 1.0f);
Mesh createPlane(float width = 1.0f, float depth = 1.0f);
Mesh createCylinder(float radius, float height, int segments);
Mesh createSphere(float radius, int latitudeSegments, int longitudeSegments);
Mesh createSpring(float radius, float height, int turns, int segments);
Mesh createFlipper(float length, float width, float height);

#endif // MESH_H
