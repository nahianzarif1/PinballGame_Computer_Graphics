#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "Camera.h"
#include "Light.h"
#include "Mesh.h"
#include <limits>
#include <vector>
#include <atomic>

class GameObject;

struct Ray {
    glm::vec3 origin, direction; // direction must be normalized.
};
struct RayMaterial {
    glm::vec3 color{1};
    float emission{0}, shininess{32}, reflection{0}, transmission{0}, ior{1.5f};
    std::shared_ptr<const TextureData> texture;
};
struct RayTriangle {
    glm::vec3 position[3], normal[3], color[3];
    glm::vec2 uv[3];
    glm::vec3 geometricNormal;
    unsigned material{0};
};
struct RayHit {
    float distance{std::numeric_limits<float>::infinity()};
    glm::vec3 barycentric{0};
    unsigned triangle{0};
};

// A self-contained scene snapshot. It never reads mutable GameObjects or GL
// resources during rendering. BVH leaves contain actual transformed triangles.
class RayScene {
public:
    std::vector<RayTriangle> triangles;
    std::vector<RayMaterial> materials;
    std::vector<PointLight> lights;
    SpotLight spot;
    glm::vec3 ambient{.1f}, background{.04f,.09f,.17f};

    void addMesh(const Mesh& mesh, const glm::mat4& world, const RayMaterial& material);
    void addObject(const GameObject& object, const glm::mat4& world);
    void buildBVH();
    bool intersect(const Ray& ray, RayHit& hit, float minimum=1e-4f,
                   float maximum=std::numeric_limits<float>::infinity()) const;
    glm::vec3 transmittance(const Ray& ray, float maximum) const;
    size_t nodeCount() const { return nodes.size(); }

private:
    struct Node {
        glm::vec3 low, high;
        unsigned begin{0}, count{0}, left{0}, right{0};
    };
    std::vector<Node> nodes;
    std::vector<unsigned> order;
    struct IntersectionTriangle { glm::vec3 origin, edge1, edge2; };
    struct BuildBounds { glm::vec3 low, high, center; };
    std::vector<IntersectionTriangle> intersections;
    std::vector<BuildBounds> bounds;
    unsigned buildNode(unsigned begin, unsigned end, unsigned depth=0);
};

struct RayImage {
    int width{0}, height{0}, samples{1};
    size_t triangleCount{0};
    double milliseconds{0};
    bool cancelled{false};
    std::vector<unsigned char> pixels; // RGB, bottom row first (OpenGL).
};

class RayTracer {
public:
    static constexpr int maxDepth=6;
    static Ray primaryRay(const Camera& camera, float x, float y, int width, int height);
    static float fresnel(float cosine, float incidentIOR, float transmittedIOR);
    static glm::vec3 trace(const RayScene& scene, const Ray& ray, int depth=maxDepth,
                          float contribution=1.0f);
    static RayImage render(const RayScene& scene, const Camera& camera, int width,
                           int height, int samples=1, unsigned workers=0,
                           const std::atomic_bool* cancel=nullptr);
};
#endif
