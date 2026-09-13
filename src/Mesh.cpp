#include "Mesh.h"
#include <glad/glad.h>
#include <cmath>
#include <iostream>

const float PI = 3.14159265358979323846f;

Mesh::Mesh() : VAO(0), VBO(0), EBO(0) {}

Mesh::~Mesh() {
    clear();
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    
    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    
    // Color
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    
    glBindVertexArray(0);
}

void Mesh::draw() const {
    glBindVertexArray(VAO);
    if (!indices.empty()) {
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    }
    glBindVertexArray(0);
}

void Mesh::clear() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    vertices.clear();
    indices.clear();
}

// Exact mathematical geometry functions

Mesh createCube(float width, float height, float depth) {
    Mesh mesh;
    
    float hw = width / 2.0f;
    float hh = height / 2.0f;
    float hd = depth / 2.0f;
    
    // 8 vertices of a cube
    glm::vec3 positions[8] = {
        glm::vec3(-hw, -hh, -hd),
        glm::vec3( hw, -hh, -hd),
        glm::vec3( hw,  hh, -hd),
        glm::vec3(-hw,  hh, -hd),
        glm::vec3(-hw, -hh,  hd),
        glm::vec3( hw, -hh,  hd),
        glm::vec3( hw,  hh,  hd),
        glm::vec3(-hw,  hh,  hd)
    };
    
    // 6 faces with normals
    glm::vec3 normals[6] = {
        glm::vec3(0, 0, -1), // Front
        glm::vec3(0, 0,  1), // Back
        glm::vec3(-1, 0, 0), // Left
        glm::vec3( 1, 0, 0), // Right
        glm::vec3(0, -1, 0), // Bottom
        glm::vec3(0,  1, 0)  // Top
    };
    
    unsigned int faceIndices[6][4] = {
        {0, 1, 2, 3}, // Front
        {4, 7, 6, 5}, // Back
        {0, 4, 7, 3}, // Left
        {1, 5, 6, 2}, // Right
        {0, 4, 5, 1}, // Bottom
        {3, 7, 6, 2}  // Top
    };
    
    glm::vec3 baseColor(1.0f, 1.0f, 1.0f);
    
    for (int face = 0; face < 6; face++) {
        // Two triangles per face
        int i0 = faceIndices[face][0];
        int i1 = faceIndices[face][1];
        int i2 = faceIndices[face][2];
        int i3 = faceIndices[face][3];
        
        // Triangle 1
        mesh.vertices.push_back({positions[i0], normals[face], baseColor});
        mesh.vertices.push_back({positions[i1], normals[face], baseColor});
        mesh.vertices.push_back({positions[i2], normals[face], baseColor});
        
        // Triangle 2
        mesh.vertices.push_back({positions[i0], normals[face], baseColor});
        mesh.vertices.push_back({positions[i2], normals[face], baseColor});
        mesh.vertices.push_back({positions[i3], normals[face], baseColor});
    }
    
    mesh.setupMesh();
    return mesh;
}

Mesh createPlane(float width, float depth) {
    Mesh mesh;
    
    float hw = width / 2.0f;
    float hd = depth / 2.0f;
    
    glm::vec3 positions[4] = {
        glm::vec3(-hw, -hd, 0.0f),
        glm::vec3( hw, -hd, 0.0f),
        glm::vec3( hw,  hd, 0.0f),
        glm::vec3(-hw,  hd, 0.0f)
    };
    
    glm::vec3 normal(0.0f, 0.0f, 1.0f);
    glm::vec3 color(1.0f, 1.0f, 1.0f);
    
    // Triangle 1
    mesh.vertices.push_back({positions[0], normal, color});
    mesh.vertices.push_back({positions[1], normal, color});
    mesh.vertices.push_back({positions[2], normal, color});
    
    // Triangle 2
    mesh.vertices.push_back({positions[0], normal, color});
    mesh.vertices.push_back({positions[2], normal, color});
    mesh.vertices.push_back({positions[3], normal, color});
    
    mesh.setupMesh();
    return mesh;
}

Mesh createCylinder(float radius, float height, int segments) {
    Mesh mesh;
    
    float halfHeight = height / 2.0f;
    glm::vec3 color(1.0f, 1.0f, 1.0f);
    
    // Generate vertices for cylinder sides
    for (int i = 0; i < segments; i++) {
        float theta0 = 2.0f * PI * i / segments;
        float theta1 = 2.0f * PI * (i + 1) / segments;
        
        float x0 = radius * cos(theta0);
        float y0 = radius * sin(theta0);
        float x1 = radius * cos(theta1);
        float y1 = radius * sin(theta1);
        
        // Normal for side (pointing outward)
        glm::vec3 normal0 = normalize(glm::vec3(x0, y0, 0.0f));
        glm::vec3 normal1 = normalize(glm::vec3(x1, y1, 0.0f));
        
        // Two triangles per segment
        // Triangle 1
        mesh.vertices.push_back({glm::vec3(x0, y0, -halfHeight), normal0, color});
        mesh.vertices.push_back({glm::vec3(x1, y1, -halfHeight), normal1, color});
        mesh.vertices.push_back({glm::vec3(x1, y1,  halfHeight), normal1, color});
        
        // Triangle 2
        mesh.vertices.push_back({glm::vec3(x0, y0, -halfHeight), normal0, color});
        mesh.vertices.push_back({glm::vec3(x1, y1,  halfHeight), normal1, color});
        mesh.vertices.push_back({glm::vec3(x0, y0,  halfHeight), normal0, color});
    }
    
    // Top cap
    glm::vec3 topNormal(0.0f, 0.0f, 1.0f);
    for (int i = 0; i < segments; i++) {
        float theta0 = 2.0f * PI * i / segments;
        float theta1 = 2.0f * PI * (i + 1) / segments;
        
        float x0 = radius * cos(theta0);
        float y0 = radius * sin(theta0);
        float x1 = radius * cos(theta1);
        float y1 = radius * sin(theta1);
        
        mesh.vertices.push_back({glm::vec3(0.0f, 0.0f, halfHeight), topNormal, color});
        mesh.vertices.push_back({glm::vec3(x0, y0, halfHeight), topNormal, color});
        mesh.vertices.push_back({glm::vec3(x1, y1, halfHeight), topNormal, color});
    }
    
    // Bottom cap
    glm::vec3 bottomNormal(0.0f, 0.0f, -1.0f);
    for (int i = 0; i < segments; i++) {
        float theta0 = 2.0f * PI * i / segments;
        float theta1 = 2.0f * PI * (i + 1) / segments;
        
        float x0 = radius * cos(theta0);
        float y0 = radius * sin(theta0);
        float x1 = radius * cos(theta1);
        float y1 = radius * sin(theta1);
        
        mesh.vertices.push_back({glm::vec3(0.0f, 0.0f, -halfHeight), bottomNormal, color});
        mesh.vertices.push_back({glm::vec3(x1, y1, -halfHeight), bottomNormal, color});
        mesh.vertices.push_back({glm::vec3(x0, y0, -halfHeight), bottomNormal, color});
    }
    
    mesh.setupMesh();
    return mesh;
}

Mesh createSphere(float radius, int latitudeSegments, int longitudeSegments) {
    Mesh mesh;
    
    glm::vec3 color(1.0f, 1.0f, 1.0f);
    
    for (int i = 0; i <= latitudeSegments; i++) {
        float phi = -PI / 2.0f + PI * i / latitudeSegments;
        float sinPhi = sin(phi);
        float cosPhi = cos(phi);
        
        for (int j = 0; j <= longitudeSegments; j++) {
            float theta = 2.0f * PI * j / longitudeSegments;
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);
            
            float x = radius * cosPhi * cosTheta;
            float y = radius * cosPhi * sinTheta;
            float z = radius * sinPhi;
            
            glm::vec3 position(x, y, z);
            glm::vec3 normal = normalize(position);
            
            mesh.vertices.push_back({position, normal, color});
        }
    }
    
    // Generate indices for triangles
    for (int i = 0; i < latitudeSegments; i++) {
        for (int j = 0; j < longitudeSegments; j++) {
            int first = i * (longitudeSegments + 1) + j;
            int second = first + longitudeSegments + 1;
            
            mesh.indices.push_back(first);
            mesh.indices.push_back(second);
            mesh.indices.push_back(first + 1);
            
            mesh.indices.push_back(first + 1);
            mesh.indices.push_back(second);
            mesh.indices.push_back(second + 1);
        }
    }
    
    mesh.setupMesh();
    return mesh;
}

Mesh createSpring(float radius, float height, int turns, int segments) {
    Mesh mesh;
    
    glm::vec3 color(1.0f, 1.0f, 1.0f);
    float halfHeight = height / 2.0f;
    
    for (int i = 0; i < segments; i++) {
        float t0 = (float)i / (segments - 1);
        float t1 = (float)(i + 1) / (segments - 1);
        
        float theta0 = 2.0f * PI * turns * t0;
        float theta1 = 2.0f * PI * turns * t1;
        
        float x0 = radius * cos(theta0);
        float y0 = radius * sin(theta0);
        float z0 = -halfHeight + height * t0;
        
        float x1 = radius * cos(theta1);
        float y1 = radius * sin(theta1);
        float z1 = -halfHeight + height * t1;
        
        // Calculate tangent for normal
        glm::vec3 tangent(x1 - x0, y1 - y0, z1 - z0);
        glm::vec3 up(0.0f, 0.0f, 1.0f);
        glm::vec3 normal = normalize(cross(tangent, up));
        
        // Create a small tube around the helix
        float tubeRadius = radius * 0.1f;
        int tubeSegments = 8;
        
        for (int k = 0; k < tubeSegments; k++) {
            float alpha0 = 2.0f * PI * k / tubeSegments;
            float alpha1 = 2.0f * PI * (k + 1) / tubeSegments;
            
            glm::vec3 offset0 = tubeRadius * (cos(alpha0) * normalize(cross(normal, tangent)) + sin(alpha0) * normal);
            glm::vec3 offset1 = tubeRadius * (cos(alpha1) * normalize(cross(normal, tangent)) + sin(alpha1) * normal);
            
            glm::vec3 p0 = glm::vec3(x0, y0, z0) + offset0;
            glm::vec3 p1 = glm::vec3(x0, y0, z0) + offset1;
            glm::vec3 p2 = glm::vec3(x1, y1, z1) + offset1;
            glm::vec3 p3 = glm::vec3(x1, y1, z1) + offset0;
            
            glm::vec3 tubeNormal = normalize(offset0);
            
            mesh.vertices.push_back({p0, tubeNormal, color});
            mesh.vertices.push_back({p1, tubeNormal, color});
            mesh.vertices.push_back({p2, tubeNormal, color});
            
            mesh.vertices.push_back({p0, tubeNormal, color});
            mesh.vertices.push_back({p2, tubeNormal, color});
            mesh.vertices.push_back({p3, tubeNormal, color});
        }
    }
    
    mesh.setupMesh();
    return mesh;
}

Mesh createFlipper(float length, float width, float height) {
    Mesh mesh;
    
    float halfLength = length / 2.0f;
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;
    
    // Create a capsule-like shape (rounded rectangle)
    glm::vec3 color(1.0f, 1.0f, 1.0f);
    
    // Main body (cuboid)
    glm::vec3 positions[8] = {
        glm::vec3(-halfLength, -halfWidth, -halfHeight),
        glm::vec3( halfLength, -halfWidth, -halfHeight),
        glm::vec3( halfLength,  halfWidth, -halfHeight),
        glm::vec3(-halfLength,  halfWidth, -halfHeight),
        glm::vec3(-halfLength, -halfWidth,  halfHeight),
        glm::vec3( halfLength, -halfWidth,  halfHeight),
        glm::vec3( halfLength,  halfWidth,  halfHeight),
        glm::vec3(-halfLength,  halfWidth,  halfHeight)
    };
    
    glm::vec3 normals[6] = {
        glm::vec3(0, 0, -1),
        glm::vec3(0, 0,  1),
        glm::vec3(-1, 0, 0),
        glm::vec3( 1, 0, 0),
        glm::vec3(0, -1, 0),
        glm::vec3(0,  1, 0)
    };
    
    unsigned int faceIndices[6][4] = {
        {0, 1, 2, 3},
        {4, 7, 6, 5},
        {0, 4, 7, 3},
        {1, 5, 6, 2},
        {0, 4, 5, 1},
        {3, 7, 6, 2}
    };
    
    for (int face = 0; face < 6; face++) {
        int i0 = faceIndices[face][0];
        int i1 = faceIndices[face][1];
        int i2 = faceIndices[face][2];
        int i3 = faceIndices[face][3];
        
        mesh.vertices.push_back({positions[i0], normals[face], color});
        mesh.vertices.push_back({positions[i1], normals[face], color});
        mesh.vertices.push_back({positions[i2], normals[face], color});
        
        mesh.vertices.push_back({positions[i0], normals[face], color});
        mesh.vertices.push_back({positions[i2], normals[face], color});
        mesh.vertices.push_back({positions[i3], normals[face], color});
    }
    
    // Add hemisphere caps at ends
    int capSegments = 12;
    float capRadius = sqrt(halfWidth * halfWidth + halfHeight * halfHeight);
    
    for (int cap = 0; cap < 2; cap++) {
        float capX = (cap == 0) ? -halfLength : halfLength;
        float capDir = (cap == 0) ? -1.0f : 1.0f;
        
        for (int i = 0; i < capSegments; i++) {
            float phi0 = PI / 2.0f * i / capSegments;
            float phi1 = PI / 2.0f * (i + 1) / capSegments;
            
            for (int j = 0; j < capSegments; j++) {
                float theta0 = 2.0f * PI * j / capSegments;
                float theta1 = 2.0f * PI * (j + 1) / capSegments;
                
                float r0 = capRadius * cos(phi0);
                float r1 = capRadius * cos(phi1);
                float z0 = capRadius * sin(phi0);
                float z1 = capRadius * sin(phi1);
                
                glm::vec3 p0(capX + capDir * r0 * cos(theta0), r0 * sin(theta0), z0);
                glm::vec3 p1(capX + capDir * r0 * cos(theta1), r0 * sin(theta1), z0);
                glm::vec3 p2(capX + capDir * r1 * cos(theta1), r1 * sin(theta1), z1);
                glm::vec3 p3(capX + capDir * r1 * cos(theta0), r1 * sin(theta0), z1);
                
                glm::vec3 n0 = normalize(glm::vec3(capDir * cos(phi0) * cos(theta0), cos(phi0) * sin(theta0), sin(phi0)));
                glm::vec3 n1 = normalize(glm::vec3(capDir * cos(phi0) * cos(theta1), cos(phi0) * sin(theta1), sin(phi0)));
                glm::vec3 n2 = normalize(glm::vec3(capDir * cos(phi1) * cos(theta1), cos(phi1) * sin(theta1), sin(phi1)));
                glm::vec3 n3 = normalize(glm::vec3(capDir * cos(phi1) * cos(theta0), cos(phi1) * sin(theta0), sin(phi1)));
                
                mesh.vertices.push_back({p0, n0, color});
                mesh.vertices.push_back({p1, n1, color});
                mesh.vertices.push_back({p2, n2, color});
                
                mesh.vertices.push_back({p0, n0, color});
                mesh.vertices.push_back({p2, n2, color});
                mesh.vertices.push_back({p3, n3, color});
            }
        }
    }
    
    mesh.setupMesh();
    return mesh;
}
