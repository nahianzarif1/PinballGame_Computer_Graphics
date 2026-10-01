#ifndef HUD_H
#define HUD_H

#include <string>
#include <glm/glm.hpp>

class HUD {
public:
    void initialize();
    void shutdown();
    void renderConsole(int fbWidth, int fbHeight, int score, int lives, int ballNumber,
                       bool lightsOn, bool fanOn, bool dayMode, bool pinballCam,
                       bool gameOver, const char* shadingName);

private:
    unsigned int shader{0};
    unsigned int vao{0};
    unsigned int vbo{0};
    int fbW{1280};
    int fbH{720};

    void drawQuad(float x, float y, float w, float h, const glm::vec4& color);
    void drawGlyph(char c, float x, float y, float s, const glm::vec3& color);
    void drawText(const std::string& text, float x, float y, float s, const glm::vec3& color);
};

#endif
