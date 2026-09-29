#ifndef MESH_H
#define MESH_H

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 texCoord{0.0f, 0.0f};
};

// Immutable CPU copy shared with the ray tracer; no OpenGL calls on workers.
struct TextureData {
    int width{0}, height{0};
    std::vector<unsigned char> pixels;
    glm::vec3 sample(const glm::vec2& uv) const;
};

class Mesh {
public:
    unsigned int VAO, VBO, EBO;
    unsigned int textureID;
    bool hasTexture;
    std::shared_ptr<const TextureData> textureData;
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
Mesh createPlane(float width = 1.0f, float depth = 1.0f, float textureRepeat = 4.0f);
Mesh createCylinder(float radius, float height, int segments);
Mesh createSphere(float radius, int latitudeSegments, int longitudeSegments);
Mesh createSpring(float radius, float height, int turns, int segments);
Mesh createFlipper(float length, float width, float height);
Mesh createRing(float radius, float tubeRadius, int segments = 40, int tubeSegments = 8);

#endif // MESH_H

// Text lies in X/Z and faces toward -Y.
Mesh createTextMesh(const std::string& text, float height);
Mesh createScreenMesh(float width, float height);
