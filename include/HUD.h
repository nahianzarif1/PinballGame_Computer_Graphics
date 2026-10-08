#ifndef HUD_H
#define HUD_H

#include <string>
#include <glm/glm.hpp>

class PinballMachine;

class HUD {
public:
    void initialize();
    void shutdown();
    void renderConsole(int fbWidth, int fbHeight, const PinballMachine& machine,
                       bool lightsOn, bool fanOn, bool dayMode, bool pinballCam,
                       const char* shadingName,int rayWidth=0);
    void renderRoom(int fbWidth, int fbHeight, bool lightsOn, bool fanOn,
                    bool dayMode, float fanSpeed, const char* shadingName,
                    bool doorOpen=false, float doorAngle=0, double rayMilliseconds=0,
                    int rayWidth=0,bool refining=false);
    int roomSwitchAt(float x, float y, int fbWidth, int fbHeight) const;

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
