#include "HUD.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cctype>
#include <algorithm>

namespace {
unsigned int compileProgram(const char* vertPath, const char* fragPath) {
    auto readFile = [](const char* path) {
        std::ifstream in(path);
        std::stringstream ss;
        ss << in.rdbuf();
        return ss.str();
    };
    std::string vs = readFile(vertPath);
    std::string fs = readFile(fragPath);
    const char* vsrc = vs.c_str();
    const char* fsrc = fs.c_str();
    unsigned int v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &vsrc, nullptr);
    glCompileShader(v);
    unsigned int f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &fsrc, nullptr);
    glCompileShader(f);
    unsigned int p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    glDeleteShader(v);
    glDeleteShader(f);
    return p;
}

// 7 rows, 5 bits each (bit0 = leftmost)
const unsigned char* glyphRows(char c) {
    static const unsigned char Z[7] = {0,0,0,0,0,0,0};
    static const unsigned char G0[7] = {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E};
    static const unsigned char G1[7] = {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E};
    static const unsigned char G2[7] = {0x0E,0x11,0x01,0x06,0x08,0x10,0x1F};
    static const unsigned char G3[7] = {0x0E,0x11,0x01,0x06,0x01,0x11,0x0E};
    static const unsigned char G4[7] = {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02};
    static const unsigned char G5[7] = {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E};
    static const unsigned char G6[7] = {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E};
    static const unsigned char G7[7] = {0x1F,0x01,0x02,0x04,0x08,0x08,0x08};
    static const unsigned char G8[7] = {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E};
    static const unsigned char G9[7] = {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C};
    static const unsigned char GA[7] = {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11};
    static const unsigned char GB[7] = {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E};
    static const unsigned char GC[7] = {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E};
    static const unsigned char GD[7] = {0x1C,0x12,0x11,0x11,0x11,0x12,0x1C};
    static const unsigned char GE[7] = {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F};
    static const unsigned char GF[7] = {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10};
    static const unsigned char GG[7] = {0x0E,0x11,0x10,0x17,0x11,0x11,0x0F};
    static const unsigned char GH[7] = {0x11,0x11,0x11,0x1F,0x11,0x11,0x11};
    static const unsigned char GI[7] = {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E};
    static const unsigned char GJ[7] = {0x01,0x01,0x01,0x01,0x11,0x11,0x0E};
    static const unsigned char GK[7] = {0x11,0x12,0x14,0x18,0x14,0x12,0x11};
    static const unsigned char GL[7] = {0x10,0x10,0x10,0x10,0x10,0x10,0x1F};
    static const unsigned char GM[7] = {0x11,0x1B,0x15,0x15,0x11,0x11,0x11};
    static const unsigned char GN[7] = {0x11,0x19,0x15,0x13,0x11,0x11,0x11};
    static const unsigned char GO[7] = {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E};
    static const unsigned char GP[7] = {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10};
    static const unsigned char GQ[7] = {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D};
    static const unsigned char GR[7] = {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11};
    static const unsigned char GS[7] = {0x0E,0x11,0x10,0x0E,0x01,0x11,0x0E};
    static const unsigned char GT[7] = {0x1F,0x04,0x04,0x04,0x04,0x04,0x04};
    static const unsigned char GU[7] = {0x11,0x11,0x11,0x11,0x11,0x11,0x0E};
    static const unsigned char GV[7] = {0x11,0x11,0x11,0x11,0x11,0x0A,0x04};
    static const unsigned char GW[7] = {0x11,0x11,0x11,0x15,0x15,0x1B,0x11};
    static const unsigned char GX[7] = {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11};
    static const unsigned char GY[7] = {0x11,0x11,0x0A,0x04,0x04,0x04,0x04};
    static const unsigned char GZ[7] = {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F};
    static const unsigned char COL[7] = {0x00,0x04,0x00,0x00,0x04,0x00,0x00};
    static const unsigned char DASH[7] = {0x00,0x00,0x00,0x1F,0x00,0x00,0x00};
    static const unsigned char DOT[7] = {0x00,0x00,0x00,0x00,0x00,0x04,0x04};
    static const unsigned char SL[7] = {0x01,0x02,0x02,0x04,0x08,0x08,0x10};
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    if (c >= '0' && c <= '9') {
        const unsigned char* d[10] = {G0,G1,G2,G3,G4,G5,G6,G7,G8,G9};
        return d[c - '0'];
    }
    if (c >= 'A' && c <= 'Z') {
        const unsigned char* a[26] = {GA,GB,GC,GD,GE,GF,GG,GH,GI,GJ,GK,GL,GM,GN,GO,GP,GQ,GR,GS,GT,GU,GV,GW,GX,GY,GZ};
        return a[c - 'A'];
    }
    if (c == ':') return COL;
    if (c == '-') return DASH;
    if (c == '.') return DOT;
    if (c == '/') return SL;
    return Z;
}
}

void HUD::initialize() {
    shader = compileProgram("shaders/hud.vert", "shaders/hud.frag");
    float verts[] = {
        0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f
    };
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void HUD::shutdown() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (shader) glDeleteProgram(shader);
    vao = vbo = shader = 0;
}

void HUD::drawQuad(float x, float y, float w, float h, const glm::vec4& color) {
    glUseProgram(shader);
    glm::mat4 proj = glm::ortho(0.0f, (float)fbW, 0.0f, (float)fbH);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, &proj[0][0]);
    glUniform2f(glGetUniformLocation(shader, "pos"), x, y);
    glUniform2f(glGetUniformLocation(shader, "size"), w, h);
    glUniform4f(glGetUniformLocation(shader, "color"), color.r, color.g, color.b, color.a);
    glUniform1i(glGetUniformLocation(shader, "useTexture"), 0);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void HUD::drawGlyph(char c, float x, float y, float s, const glm::vec3& color) {
    const unsigned char* rows = glyphRows(c);
    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 5; col++) {
            if (rows[row] & (1 << (4 - col))) {
                drawQuad(x + col * s, y + (6 - row) * s, s, s, glm::vec4(color, 1.0f));
            }
        }
    }
}

void HUD::drawText(const std::string& text, float x, float y, float s, const glm::vec3& color) {
    float cx = x;
    for (char c : text) {
        if (c == ' ') {
            cx += 4.0f * s;
            continue;
        }
        drawGlyph(c, cx, y, s, color);
        cx += 6.0f * s;
    }
}

void HUD::renderConsole(int fbWidth, int fbHeight, int score, int lives, int ballNumber,
                        bool lightsOn, bool fanOn, bool dayMode, bool pinballCam,
                        bool gameOver, const char* shadingName) {
    fbW = fbWidth;
    fbH = fbHeight;
    float panelW = fbWidth * 0.27f;
    float x0 = fbWidth - panelW;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    drawQuad(x0, 0.0f, panelW, (float)fbHeight, glm::vec4(0.07f, 0.08f, 0.16f, 1.0f));
    drawQuad(x0 + 8.0f, 8.0f, panelW - 16.0f, fbHeight - 16.0f, glm::vec4(0.12f, 0.13f, 0.24f, 1.0f));

    glm::vec3 title(0.55f, 0.85f, 1.0f);
    glm::vec3 accent(0.75f, 0.55f, 1.0f);
    glm::vec3 white(0.9f, 0.92f, 1.0f);
    glm::vec3 dim(0.55f, 0.6f, 0.8f);
    glm::vec3 gold(1.0f, 0.85f, 0.35f);

    float s = std::max(2.0f, panelW / 240.0f);
    float tx = x0 + 22.0f;
    float y = fbHeight - 52.0f;

    drawText("3D PINBALL", tx, y, s * 1.05f, title);
    y -= 30.0f * s;
    drawText("SPACE CADET", tx, y, s * 1.15f, accent);
    y -= 38.0f * s;
    drawText("GAME HUB", tx, y, s, gold);

    y -= 40.0f * s;
    drawQuad(tx, y, panelW - 50.0f, 4.0f, glm::vec4(0.4f, 0.5f, 0.9f, 0.8f));

    y -= 40.0f * s;
    char buf[64];
    std::snprintf(buf, sizeof(buf), "BALL %d", ballNumber);
    drawText(buf, tx, y, s * 1.1f, white);

    y -= 46.0f * s;
    drawQuad(tx, y - 8.0f, panelW - 50.0f, 58.0f * s, glm::vec4(0.05f, 0.06f, 0.12f, 1.0f));
    drawText("PLAYER 1", tx + 8.0f, y + 28.0f * s, s * 0.85f, dim);
    std::snprintf(buf, sizeof(buf), "%06d", score);
    drawText(buf, tx + 8.0f, y + 6.0f, s * 1.35f, gold);

    y -= 80.0f * s;
    std::snprintf(buf, sizeof(buf), "LIVES %d", lives);
    drawText(buf, tx, y, s * 1.1f, lives > 0 ? white : glm::vec3(1.0f, 0.3f, 0.3f));

    y -= 36.0f * s;
    if (gameOver) {
        drawText("GAME OVER", tx, y, s * 1.15f, glm::vec3(1.0f, 0.35f, 0.4f));
        y -= 26.0f * s;
        drawText("PRESS R", tx, y, s, gold);
    } else {
        drawText("NOW PLAYING", tx, y, s, glm::vec3(0.4f, 1.0f, 0.55f));
    }

    y -= 44.0f * s;
    drawText(lightsOn ? "LIGHTS ON" : "LIGHTS OFF", tx, y, s, lightsOn ? gold : dim);
    y -= 24.0f * s;
    drawText(fanOn ? "FAN SPINNING" : "FAN OFF", tx, y, s, fanOn ? glm::vec3(0.4f, 0.85f, 1.0f) : dim);
    y -= 24.0f * s;
    drawText(dayMode ? "DAY MODE" : "NIGHT MODE", tx, y, s, dayMode ? glm::vec3(1.0f, 0.85f, 0.4f) : glm::vec3(0.45f, 0.55f, 1.0f));
    y -= 24.0f * s;
    drawText(pinballCam ? "TABLE CAM" : "HUB CAM", tx, y, s, white);
    y -= 24.0f * s;
    drawText(shadingName ? shadingName : "PHONG", tx, y, s, dim);

    y -= 40.0f * s;
    drawText("A D FLIPPERS", tx, y, s * 0.8f, dim);
    y -= 20.0f * s;
    drawText("ENTER LAUNCH", tx, y, s * 0.8f, dim);
    y -= 20.0f * s;
    drawText("1 LIGHT 2 FAN", tx, y, s * 0.8f, dim);
    y -= 20.0f * s;
    drawText("3 DAY 4 NIGHT", tx, y, s * 0.8f, dim);
    y -= 20.0f * s;
    drawText("C CAMERA  R RESET", tx, y, s * 0.8f, dim);
    y -= 20.0f * s;
    drawText("RMB LOOK WS QE", tx, y, s * 0.8f, dim);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
