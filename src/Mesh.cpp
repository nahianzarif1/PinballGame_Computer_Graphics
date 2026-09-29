#include "Mesh.h"
#include "BitmapFont.h"
#include <glad/glad.h>
#include <cmath>
#include <utility>
#include <vector>
#include <string>
#include <algorithm>

const float PI = 3.14159265358979323846f;

Mesh::Mesh() : VAO(0), VBO(0), EBO(0), textureID(0), hasTexture(false) {}

Mesh::~Mesh() {
    clear();
}

Mesh::Mesh(Mesh&& other) noexcept
    : VAO(other.VAO), VBO(other.VBO), EBO(other.EBO),
      textureID(other.textureID), hasTexture(other.hasTexture),
      textureData(std::move(other.textureData)),
      vertices(std::move(other.vertices)), indices(std::move(other.indices)) {
    other.VAO = 0;
    other.VBO = 0;
    other.EBO = 0;
    other.textureID = 0;
    other.hasTexture = false;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        clear();
        VAO = other.VAO;
        VBO = other.VBO;
        EBO = other.EBO;
        textureID = other.textureID;
        hasTexture = other.hasTexture;
        textureData = std::move(other.textureData);
        vertices = std::move(other.vertices);
        indices = std::move(other.indices);
        other.VAO = 0;
        other.VBO = 0;
        other.EBO = 0;
        other.textureID = 0;
        other.hasTexture = false;
    }
    return *this;
}

void Mesh::setupMesh() {
    if (vertices.empty() || !glad_glGenVertexArrays) {
        return;
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // data() is well-defined for an empty vector (and OpenGL ignores the
    // pointer for a zero-byte upload).  This avoids undefined behavior from
    // taking &indices[0] when procedural meshes use glDrawArrays.
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 indices.empty() ? nullptr : indices.data(), GL_STATIC_DRAW);
    
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    
    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    
    // Color
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    
    glBindVertexArray(0);
}

void Mesh::draw() const {
    if (VAO == 0) {
        return;
    }
    glBindVertexArray(VAO);
    if (!indices.empty()) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
    }
    glBindVertexArray(0);
}

void Mesh::clear() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    if (textureID) glDeleteTextures(1, &textureID);
    VAO = 0;
    VBO = 0;
    EBO = 0;
    textureID = 0;
    hasTexture = false;
    textureData.reset();
    vertices.clear();
    indices.clear();
}

void Mesh::loadTexture(const std::string& name) {
    const int w = name == "playfield" ? 512 : 128;
    const int h = name == "playfield" ? 1024 : 128;
    std::vector<unsigned char> pixels(static_cast<size_t>(w * h * 3));

    auto put = [&](int x, int y, unsigned char r, unsigned char g, unsigned char b) {
        x = std::clamp(x, 0, w - 1);
        y = std::clamp(y, 0, h - 1);
        const int i = (y * w + x) * 3;
        pixels[i] = r;
        pixels[i + 1] = g;
        pixels[i + 2] = b;
    };

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float u = x / float(w);
            float v = y / float(h);
            unsigned char r = 180, g = 180, b = 180;

            if (name == "checker") {
                int c = ((x / 16) + (y / 16)) % 2;
                r = c ? 48 : 210;
                g = c ? 42 : 205;
                b = c ? 58 : 190;
            } else if (name == "wood") {
                float rings = std::sin((u * 8.0f + std::sin(v * 12.0f) * 0.2f) * 3.14159f);
                r = static_cast<unsigned char>(110 + 50 * rings);
                g = static_cast<unsigned char>(70 + 30 * rings);
                b = static_cast<unsigned char>(38 + 18 * rings);
            } else if (name == "playfield" || name == "stars") {
                // Navy circuit artwork, purple flipper rays and a cyan reactor.
                float dx=(u-.48f)*7.0f, dy=(v-.44f)*12.0f;
                float rad=std::sqrt(dx*dx+dy*dy);
                float vein=std::abs(std::sin(u*54+std::sin(v*34)*2.2f));
                bool trace=vein<.045f || std::abs(std::sin(v*62+std::sin(u*39)))<.035f;
                r=trace ? 46 : 15; g=trace ? 62 : 23; b=trace ? 105 : 53;
                if (rad<.95f) {
                    float glow=1-rad/.95f;
                    r=static_cast<unsigned char>(18+15*glow);
                    g=static_cast<unsigned char>(55+125*glow);
                    b=static_cast<unsigned char>(105+130*glow);
                }
                if (std::abs(rad-.9f)<.025f || std::abs(rad-1.5f)<.025f) {r=40;g=80;b=170;}
                if (v<.34f && u<.81f) {
                    float angle=std::atan2((u-.49f)*7,(v-.07f)*12);
                    bool ray=std::abs(std::sin(angle*9))<.20f;
                    if (ray) {r=100;g=42;b=154;}
                }
                if (u<.22f && v>.25f && v<.69f) {
                    r=84;g=43;b=130;
                    if (std::fmod(v*42+u*12,1.0f)<.2f) {r=126;g=76;b=174;}
                }
                if (u>.855f) {r=50;g=24;b=32;}
                if ((u<.035f || u>.965f) && std::fmod(v*23,1.0f)<.22f) {r=245;g=184;b=40;}
                int n=(x*73+y*149)%997;
                if(n<2 && v>.32f && u<.85f){r=145;g=179;b=209;}
            } else if (name == "brick") {
                int row = y / 16;
                int col = (x + (row % 2) * 16) / 32;
                bool mortar = (y % 16 < 2) || ((x + (row % 2) * 16) % 32 < 2);
                if (mortar) { r = 190; g = 180; b = 170; }
                else { r = 150 + (col * 13) % 40; g = 70; b = 55; }
            } else if (name == "fabric") {
                r = static_cast<unsigned char>(80 + 40 * std::sin(u * 40));
                g = static_cast<unsigned char>(50 + 25 * std::sin(v * 36));
                b = 90;
            } else if (name == "carpet") {
                r = static_cast<unsigned char>(90 + 20 * ((x / 8 + y / 8) % 2));
                g = 28;
                b = 42;
            } else if (name == "basketball") {
                r = 220; g = 110; b = 30;
                if (std::abs(x - 64) < 4 || std::abs(y - 64) < 4) { r = 20; g = 20; b = 20; }
                float dx = (x - 64) / 64.0f;
                float dy = (y - 64) / 64.0f;
                if (std::abs(dx * dx + dy * 0.35f) < 0.04f) { r = 20; g = 20; b = 20; }
            } else if (name == "metal") {
                r = g = b = static_cast<unsigned char>(140 + 40 * std::sin(u * 50));
            } else if (name == "concrete") {
                int n = (x * 13 + y * 29) % 23;
                r = g = b = static_cast<unsigned char>(120 + n);
            } else if (name == "neon") {
                float glow = 0.5f + 0.5f * std::sin(u * 18.0f + v * 6.0f);
                r = static_cast<unsigned char>(80 + 140 * glow);
                g = static_cast<unsigned char>(40 + 60 * glow);
                b = static_cast<unsigned char>(180 + 70 * glow);
                if ((y / 10) % 2 == 0) { r = 40; g = 20; b = 90; }
            } else if (name == "white_tile") {
                bool grout=(x%32==0 || y%32==0);
                int value=grout ? 176 : 231+(x*11+y*7)%6;
                r=value;g=value+2;b=value+4;
            } else if (name == "acoustic") {
                bool hole=(x%5==0 && y%5==0);
                r=hole ? 50 : 116;g=hole ? 57 : 128;b=hole ? 64 : 143;
            } else if (name == "upholstery") {
                int weave=((x+y)%3==0 ? 18 : 0)+(x*13+y*23)%10;
                r=210+weave;g=214+weave;b=219+weave;
            } else if (name == "electric_rug") {
                float dx=(u-.5f)*1.4f,dy=(v-.5f);
                float rad=std::sqrt(dx*dx+dy*dy);
                float bolt=std::abs(std::sin(std::atan2(dy,dx)*13+std::sin(rad*42)*2));
                bool electric=rad>.24f && rad<.54f && bolt<.12f;
                r=electric ? 115 : 10;g=electric ? 193 : 23;b=electric ? 245 : 46;
                if(std::abs(rad-.5f)<.012f) {r=55;g=131;b=219;}
            } else if (name == "arena_screen") {
                if(u<.56f) {
                    bool stripe=(y%17<2);
                    r=stripe ? 110 : 42;g=stripe ? 56 : 28;b=stripe ? 87 : 71;
                    if(v>.83f) {r=204;g=87;b=31;}
                } else {
                    float glow=std::max(0.0f,1-std::abs(u-.77f)*4);
                    r=static_cast<unsigned char>(145+90*glow);
                    g=static_cast<unsigned char>(65+110*v);
                    b=static_cast<unsigned char>(90+80*(1-v));
                    if(v<.19f) {r=28;g=41;b=142;}
                    if(std::abs(std::sin(u*45+v*7))<.16f && v>.22f && v<.72f) {r=240;g=195;b=110;}
                }
            } else if (name == "tile") {
                int cx = x / 16;
                int cy = y / 16;
                bool grout = (x % 16 < 1) || (y % 16 < 1);
                int shade = 28 + ((cx * 7 + cy * 13) % 10);
                r = grout ? 22 : static_cast<unsigned char>(95 + shade);
                g = grout ? 23 : static_cast<unsigned char>(87 + shade);
                b = grout ? 28 : static_cast<unsigned char>(80 + shade);
            } else if (name == "screen") {
                float glow = 0.45f + 0.55f * std::sin((u + v) * 22.0f);
                r = static_cast<unsigned char>(30 + 40 * glow);
                g = static_cast<unsigned char>(90 + 90 * glow);
                b = static_cast<unsigned char>(140 + 110 * glow);
                if ((y / 8) % 3 == 0) { r = 20; g = 160; b = 90; }
            } else if (name == "leaf") {
                r = static_cast<unsigned char>(30 + 20 * std::sin(u * 20.0f));
                g = static_cast<unsigned char>(90 + 70 * std::sin(v * 16.0f));
                b = 28;
            } else {
                r = 40; g = 80; b = 160;
            }
            put(x, y, r, g, b);
        }
    }

    auto data=std::make_shared<TextureData>();
    data->width=w;data->height=h;data->pixels=std::move(pixels);
    textureData=data;
    hasTexture=true;
    if (!glad_glGenTextures) return;
    if (textureID) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data->pixels.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    hasTexture = true;
}

glm::vec3 TextureData::sample(const glm::vec2& uv) const {
    if(width<=0 || height<=0 || pixels.empty())return glm::vec3(1);
    glm::vec2 p=(uv-glm::floor(uv))*glm::vec2(width,height)-glm::vec2(.5f);
    int x=static_cast<int>(std::floor(p.x)),y=static_cast<int>(std::floor(p.y));
    auto texel=[&](int a,int b) {
        a=(a%width+width)%width;b=(b%height+height)%height;
        size_t i=(static_cast<size_t>(b)*width+a)*3;
        return glm::vec3(pixels[i],pixels[i+1],pixels[i+2])/255.0f;
    };
    glm::vec2 f=p-glm::floor(p);
    return glm::mix(glm::mix(texel(x,y),texel(x+1,y),f.x),
                    glm::mix(texel(x,y+1),texel(x+1,y+1),f.x),f.y);
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
    glm::vec2 faceUV[4] = {
        glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f)
    };
    
    for (int face = 0; face < 6; face++) {
        int i0 = faceIndices[face][0];
        int i1 = faceIndices[face][1];
        int i2 = faceIndices[face][2];
        int i3 = faceIndices[face][3];
        
        mesh.vertices.push_back({positions[i0], normals[face], baseColor, faceUV[0]});
        mesh.vertices.push_back({positions[i1], normals[face], baseColor, faceUV[1]});
        mesh.vertices.push_back({positions[i2], normals[face], baseColor, faceUV[2]});
        
        mesh.vertices.push_back({positions[i0], normals[face], baseColor, faceUV[0]});
        mesh.vertices.push_back({positions[i2], normals[face], baseColor, faceUV[2]});
        mesh.vertices.push_back({positions[i3], normals[face], baseColor, faceUV[3]});
    }
    
    mesh.setupMesh();
    return mesh;
}

Mesh createPlane(float width, float depth, float textureRepeat) {
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
    
    mesh.vertices.push_back({positions[0], normal, color, glm::vec2(0.0f, 0.0f)});
    mesh.vertices.push_back({positions[1], normal, color, glm::vec2(textureRepeat, 0.0f)});
    mesh.vertices.push_back({positions[2], normal, color, glm::vec2(textureRepeat, textureRepeat)});
    
    mesh.vertices.push_back({positions[0], normal, color, glm::vec2(0.0f, 0.0f)});
    mesh.vertices.push_back({positions[2], normal, color, glm::vec2(textureRepeat, textureRepeat)});
    mesh.vertices.push_back({positions[3], normal, color, glm::vec2(0.0f, textureRepeat)});
    
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
        
        float x0 = radius * std::cos(theta0);
        float y0 = radius * std::sin(theta0);
        float x1 = radius * std::cos(theta1);
        float y1 = radius * std::sin(theta1);
        
        // Normal for side (pointing outward)
        glm::vec3 normal0 = normalize(glm::vec3(x0, y0, 0.0f));
        glm::vec3 normal1 = normalize(glm::vec3(x1, y1, 0.0f));
        
        // Two triangles per segment
        // Triangle 1
        float u0 = (float)i / segments;
        float u1 = (float)(i + 1) / segments;
        mesh.vertices.push_back({glm::vec3(x0, y0, -halfHeight), normal0, color, glm::vec2(u0, 0.0f)});
        mesh.vertices.push_back({glm::vec3(x1, y1, -halfHeight), normal1, color, glm::vec2(u1, 0.0f)});
        mesh.vertices.push_back({glm::vec3(x1, y1,  halfHeight), normal1, color, glm::vec2(u1, 1.0f)});
        
        mesh.vertices.push_back({glm::vec3(x0, y0, -halfHeight), normal0, color, glm::vec2(u0, 0.0f)});
        mesh.vertices.push_back({glm::vec3(x1, y1,  halfHeight), normal1, color, glm::vec2(u1, 1.0f)});
        mesh.vertices.push_back({glm::vec3(x0, y0,  halfHeight), normal0, color, glm::vec2(u0, 1.0f)});
    }
    
    // Top cap
    glm::vec3 topNormal(0.0f, 0.0f, 1.0f);
    for (int i = 0; i < segments; i++) {
        float theta0 = 2.0f * PI * i / segments;
        float theta1 = 2.0f * PI * (i + 1) / segments;
        
        float x0 = radius * std::cos(theta0);
        float y0 = radius * std::sin(theta0);
        float x1 = radius * std::cos(theta1);
        float y1 = radius * std::sin(theta1);
        
        mesh.vertices.push_back({glm::vec3(0.0f, 0.0f, halfHeight), topNormal, color});
        mesh.vertices.push_back({glm::vec3(x0, y0, halfHeight), topNormal, color});
        mesh.vertices.push_back({glm::vec3(x1, y1, halfHeight), topNormal, color});
    }
    
    // Bottom cap
    glm::vec3 bottomNormal(0.0f, 0.0f, -1.0f);
    for (int i = 0; i < segments; i++) {
        float theta0 = 2.0f * PI * i / segments;
        float theta1 = 2.0f * PI * (i + 1) / segments;
        
        float x0 = radius * std::cos(theta0);
        float y0 = radius * std::sin(theta0);
        float x1 = radius * std::cos(theta1);
        float y1 = radius * std::sin(theta1);
        
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
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);
        
        for (int j = 0; j <= longitudeSegments; j++) {
            float theta = 2.0f * PI * j / longitudeSegments;
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);
            
            float x = radius * cosPhi * cosTheta;
            float y = radius * cosPhi * sinTheta;
            float z = radius * sinPhi;
            
            glm::vec3 position(x, y, z);
            glm::vec3 normal = normalize(position);
            glm::vec2 uv((float)j / longitudeSegments, (float)i / latitudeSegments);
            mesh.vertices.push_back({position, normal, color, uv});
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
        
        float x0 = radius * std::cos(theta0);
        float y0 = radius * std::sin(theta0);
        float z0 = -halfHeight + height * t0;
        
        float x1 = radius * std::cos(theta1);
        float y1 = radius * std::sin(theta1);
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
            
            glm::vec3 offset0 = tubeRadius * (std::cos(alpha0) * normalize(cross(normal, tangent)) + std::sin(alpha0) * normal);
            glm::vec3 offset1 = tubeRadius * (std::cos(alpha1) * normalize(cross(normal, tangent)) + std::sin(alpha1) * normal);
            
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
                
                float r0 = capRadius * std::cos(phi0);
                float r1 = capRadius * std::cos(phi1);
                float z0 = capRadius * std::sin(phi0);
                float z1 = capRadius * std::sin(phi1);
                
                glm::vec3 p0(capX + capDir * r0 * std::cos(theta0), r0 * std::sin(theta0), z0);
                glm::vec3 p1(capX + capDir * r0 * std::cos(theta1), r0 * std::sin(theta1), z0);
                glm::vec3 p2(capX + capDir * r1 * std::cos(theta1), r1 * std::sin(theta1), z1);
                glm::vec3 p3(capX + capDir * r1 * std::cos(theta0), r1 * std::sin(theta0), z1);
                
                glm::vec3 n0 = normalize(glm::vec3(capDir * std::cos(phi0) * std::cos(theta0), std::cos(phi0) * std::sin(theta0), std::sin(phi0)));
                glm::vec3 n1 = normalize(glm::vec3(capDir * std::cos(phi0) * std::cos(theta1), std::cos(phi0) * std::sin(theta1), std::sin(phi0)));
                glm::vec3 n2 = normalize(glm::vec3(capDir * std::cos(phi1) * std::cos(theta1), std::cos(phi1) * std::sin(theta1), std::sin(phi1)));
                glm::vec3 n3 = normalize(glm::vec3(capDir * std::cos(phi1) * std::cos(theta0), std::cos(phi1) * std::sin(theta0), std::sin(phi1)));
                
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


Mesh createRing(float radius, float tubeRadius, int segments, int tubeSegments) {
    Mesh mesh;
    for (int i=0;i<=segments;++i) {
        float a=2*PI*i/segments;
        for(int j=0;j<=tubeSegments;++j) {
            float b=2*PI*j/tubeSegments;
            glm::vec3 normal(std::cos(a)*std::cos(b),std::sin(a)*std::cos(b),std::sin(b));
            glm::vec3 center(radius*std::cos(a),radius*std::sin(a),0);
            mesh.vertices.push_back({center+normal*tubeRadius,normal,glm::vec3(1),{float(i)/segments,float(j)/tubeSegments}});
        }
    }
    for(int i=0;i<segments;++i) for(int j=0;j<tubeSegments;++j) {
        unsigned a=i*(tubeSegments+1)+j,b=(i+1)*(tubeSegments+1)+j;
        for(unsigned index : {a,b,a+1,a+1,b,b+1}) mesh.indices.push_back(index);
    }
    mesh.setupMesh();return mesh;
}

Mesh createTextMesh(const std::string& text, float height) {
    Mesh mesh;float pixel=height/7,offset=0;
    for(char c : text) {
        if(c==' '){offset+=4*pixel;continue;}
        const auto* rows=BitmapFont::glyphRows(c);
        for(int row=0;row<7;++row) for(int col=0;col<5;++col) {
            if(!(rows[row] & (1<<(4-col))))continue;
            float x=offset+col*pixel,z=(6-row)*pixel;
            unsigned base=static_cast<unsigned>(mesh.vertices.size());
            for(auto pos : {glm::vec3(x,0,z),glm::vec3(x+pixel,0,z),glm::vec3(x+pixel,0,z+pixel),glm::vec3(x,0,z+pixel)})
                mesh.vertices.push_back({pos,{0,-1,0},{1,1,1},{0,0}});
            for(unsigned index : {0u,1u,2u,0u,2u,3u})mesh.indices.push_back(base+index);
        }
        offset+=6*pixel;
    }
    for(auto& vertex : mesh.vertices)vertex.position.x-=offset*.5f;
    mesh.setupMesh();return mesh;
}


Mesh createScreenMesh(float width,float height) {
    Mesh mesh;
    mesh.vertices={
        {{-width*.5f,0,-height*.5f},{0,-1,0},{1,1,1},{0,0}},
        {{ width*.5f,0,-height*.5f},{0,-1,0},{1,1,1},{1,0}},
        {{ width*.5f,0, height*.5f},{0,-1,0},{1,1,1},{1,1}},
        {{-width*.5f,0, height*.5f},{0,-1,0},{1,1,1},{0,1}}
    };
    mesh.indices={0,1,2,0,2,3};mesh.setupMesh();return mesh;
}
