#include "HUD.h"
#include "BitmapFont.h"
#include "PinballMachine.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cctype>
#include <algorithm>
#include <cmath>

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


}

void HUD::initialize() {
    if (!glad_glCreateProgram) return;
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
    const unsigned char* rows = BitmapFont::glyphRows(c);
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

void HUD::renderConsole(int fbWidth, int fbHeight, const PinballMachine& m,
                        bool lightsOn, bool fanOn, bool dayMode, bool pinballCam,
                        const char* shadingName,int rayWidth) {
    fbW=fbWidth; fbH=fbHeight;
    float panelW=fbWidth*.27f, x0=fbWidth-panelW;
    // Lay out a 320 x 720 console, scaling to both window dimensions.
    float scale=std::min(panelW/320.0f,fbHeight/720.0f);
    float ox=x0+(panelW-320*scale)*.5f, oy=(fbHeight-720*scale)*.5f;
    glDisable(GL_DEPTH_TEST);glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    drawQuad(x0,0,panelW,fbHeight,{.025f,.032f,.06f,1});
    const glm::vec3 cyan(.2f,.83f,1), purple(.69f,.48f,1), gold(1,.76f,.25f);
    const glm::vec3 white(.88f,.93f,1), dim(.43f,.5f,.65f), green(.28f,1,.65f);
    auto quad=[&](float x,float y,float w,float h,glm::vec4 c){drawQuad(ox+x*scale,oy+y*scale,w*scale,h*scale,c);};
    auto text=[&](const std::string& t,float x,float y,float size,glm::vec3 c){drawText(t,ox+x*scale,oy+y*scale,size*scale,c);};
    quad(0,0,2,720,{.2f,.8f,1,1});
    quad(22,679,24,3,{.65f,.45f,1,1});text("THE GAME HUB",22,649,3.0f,white);
    text("NEON CIRCUIT / PINBALL",22,627,1.7f,cyan);
    quad(22,492,276,113,{.055f,.07f,.115f,1});
    text("PLAYER 01",36,579,1.65f,dim);
    char buf[96];std::snprintf(buf,sizeof(buf),"%08d",m.score);
    text(buf,36,526,4.55f,gold);
    std::snprintf(buf,sizeof(buf),"SESSION BEST %08d",m.highScore);
    text(buf,36,501,1.55f,dim);
    std::snprintf(buf,sizeof(buf),"BALL %d / 3",m.ballNumber);text(buf,22,466,2,white);
    for(int i=0;i<3;++i)quad(226+i*24,465,15,15,i<m.lives ? glm::vec4(cyan,1) : glm::vec4(.12f,.16f,.23f,1));
    quad(22,408,132,40,{.06f,.055f,.12f,1});quad(164,408,134,40,{.045f,.09f,.11f,1});
    std::snprintf(buf,sizeof(buf),"%dX MULTIPLIER",m.multiplier);text(buf,30,422,1.5f,purple);
    std::snprintf(buf,sizeof(buf),"BONUS %d",m.bonus*m.multiplier);text(buf,172,422,1.5f,cyan);
    const char* status=m.gameOver ? "GAME OVER - PRESS R" :
        m.ballState==BallState::READY ? "SPACE TO LAUNCH" : m.ballState==BallState::SHOOTER ? "LAUNCHING" : "BALL IN PLAY";
    text(status,22,378,1.9f,m.gameOver ? glm::vec3(1,.3f,.4f) : green);
    if(m.ballSaveTimer>0 && m.ballState!=BallState::READY) {
        std::snprintf(buf,sizeof(buf),"BALL SAVE %.0f SEC",std::ceil(m.ballSaveTimer));text(buf,22,355,1.55f,gold);
    } else text(m.eventTimer>0 ? m.lastEvent : "COMPLETE THE TARGET BANK",22,355,1.5f,dim);
    if(m.eventTimer>0 && m.lastPoints>0) {
        std::snprintf(buf,sizeof(buf),"%s  +%d",m.lastEvent.c_str(),m.lastPoints);text(buf,22,334,1.35f,gold);
    }
    text("TARGET BANK",22,308,1.6f,dim);
    for(size_t i=0;i<m.targets.size();++i)quad(22+i*70,289,60,7,m.targets[i].down ? glm::vec4(green,1) : glm::vec4(.14f,.17f,.25f,1));
    text("BANK 1000 / ORBIT 250",22,265,1.45f,purple);
    text("BUMPERS 50-150 / TARGETS 100",22,244,1.35f,dim);
    quad(22,222,276,1,{.16f,.2f,.3f,1});
    text("CONTROLS",22,198,1.55f,cyan);
    text("A / D     FLIPPERS",22,173,1.6f,white);
    text("SPACE     QUICK LAUNCH",22,151,1.6f,white);
    text("ENTER     HOLD AND RELEASE",22,129,1.45f,white);
    text("C ROOM / TABLE    R RESTART",22,107,1.45f,dim);
    text("RMB LOOK  WS WALK  QE HEIGHT",22,86,1.35f,dim);
    std::string settings=std::string(lightsOn ? "LIGHT ON" : "LIGHT OFF")+" / "+(fanOn ? "FAN ON" : "FAN OFF");
    text(settings,22,58,1.35f,dim);
    std::string renderName=shadingName;
    if(renderName=="RAY TRACE")renderName=rayWidth>0?"RAY "+std::to_string(rayWidth)+"PX":"RAY RENDERING";
    text(std::string(dayMode ? "DAY" : "NIGHT")+" / "+(pinballCam ? "TABLE" : "ROOM")+" / "+renderName,22,38,1.35f,dim);
    text("B FLAT / G GOURAUD / P PHONG",22,18,1.25f,dim);
    text("T RAY TRACE / O FRIDGE DOOR",22,72,1.25f,cyan);
    glDisable(GL_BLEND);glEnable(GL_DEPTH_TEST);
}


int HUD::roomSwitchAt(float x, float y, int width, int height) const {
    if(width<=0 || height<=0)return 0;
    float panelWidth=width*.27f,scale=std::min(panelWidth/320.0f,height/720.0f);
    float ox=width-panelWidth+(panelWidth-320*scale)*.5f,oy=(height-720*scale)*.5f;
    x=(x-ox)/scale;y=(y-oy)/scale;
    if(x<22 || x>298)return 0;
    if(y>=466 && y<=544)return 1;
    if(y>=354 && y<=432)return 2;
    if(y>=228 && y<=257)for(int i=0;i<3;++i)if(x>=22+i*94 && x<=110+i*94)return 3+i;
    if(y>=176 && y<=201)return 6;
    if(y>=137 && y<=166)return 7;
    return 0;
}
void HUD::renderRoom(int width,int height,bool lightsOn,bool fanOn,bool dayMode,float fanSpeed,const char* shadingName,
                     bool doorOpen,float doorAngle,double rayMilliseconds,int rayWidth,bool refining) {
    fbW=width;fbH=height;
    float panelWidth=width*.27f,scale=std::min(panelWidth/320.0f,height/720.0f);
    float ox=width-panelWidth+(panelWidth-320*scale)*.5f,oy=(height-720*scale)*.5f;
    glDisable(GL_DEPTH_TEST);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    const glm::vec3 white(.9f,.95f,1),dim(.47f,.57f,.7f),blue(.24f,.62f,1),green(.27f,1,.66f);
    auto quad=[&](float x,float y,float w,float h,glm::vec4 color){drawQuad(ox+x*scale,oy+y*scale,w*scale,h*scale,color);};
    auto text=[&](const std::string& t,float x,float y,float size,glm::vec3 color){drawText(t,ox+x*scale,oy+y*scale,size*scale,color);};
    drawQuad(width-panelWidth,0,panelWidth,height,{.023f,.036f,.065f,1});quad(0,0,2,720,{.19f,.49f,1,1});
    quad(22,680,30,3,{.18f,.48f,1,1});text("TECHNOLOGY",22,644,3.35f,white);
    text("GAMING LOUNGE",22,614,2.0f,blue);
    text("ROOM CONTROLS",22,568,1.65f,dim);
    auto card=[&](float y,const char* name,const char* key,bool on) {
        quad(22,y,276,78,{.055f,.083f,.13f,1});text(name,36,y+48,1.75f,white);
        text(key,36,y+22,1.4f,dim);
        quad(227,y+25,54,25,on ? glm::vec4(.08f,.35f,.58f,1) : glm::vec4(.13f,.17f,.23f,1));
        quad(on ? 255 : 232,y+29,18,17,on ? glm::vec4(blue,1) : glm::vec4(dim,1));
    };
    card(466,"ROOM LIGHTS","CLICK / KEY 1",lightsOn);
    text(lightsOn ? "PENDANTS AND LED WALL ON" : "PENDANTS AND LED WALL OFF",22,443,1.45f,lightsOn ? blue : dim);
    card(354,"CEILING FAN","CLICK / KEY 2",fanOn);
    char buffer[80];std::snprintf(buffer,sizeof(buffer),"%s / %.0f RPM",fanOn ? "RUNNING" : fanSpeed>1 ? "COASTING" : "STOPPED",fanSpeed/6);
    text(buffer,22,331,1.55f,fanOn ? green : dim);
    quad(22,302,276,1,{.15f,.23f,.34f,1});text("SHADING",22,273,1.65f,dim);
    const char* modes[]={"FLAT","GOURAUD","PHONG"};
    const char* modeKeys[]={"B","G","P"};
    for(int i=0;i<3;++i) {
        bool selected=std::string(shadingName)==modes[i];
        quad(22+i*94,228,88,29,selected ? glm::vec4(.08f,.24f,.43f,1) : glm::vec4(.055f,.083f,.13f,1));
        text(modes[i],28+i*94,239,1.2f,selected ? blue : dim);
        text(modeKeys[i],54+i*94,208,1.25f,dim);
    }
    bool tracing=std::string(shadingName)=="RAY TRACE";
    quad(22,176,276,25,tracing ? glm::vec4(.08f,.24f,.43f,1) : glm::vec4(.055f,.083f,.13f,1));
    text(tracing ? "RAY TRACING ON / T" : "ENABLE RAY TRACING / T",30,185,1.35f,tracing ? blue : white);
    quad(22,137,276,29,{.055f,.083f,.13f,1});
    text(doorOpen ? "CLOSE FRIDGE / O" : "OPEN FRIDGE / O",30,148,1.4f,white);
    if(doorAngle>0 && doorAngle<100)text(doorOpen ? "OPENING" : "CLOSING",222,148,1.1f,green);
    if(tracing) {
        std::snprintf(buffer,sizeof(buffer),"%s %dPX / %.0f MS",refining?"REFINE":"RAY",rayWidth,rayMilliseconds);
        text(rayMilliseconds>0 ? buffer : "RENDERING FIRST RAY FRAME",22,117,1.3f,blue);
    } else text(dayMode ? "DAY / KEY 4 NIGHT" : "NIGHT / KEY 3 DAY",22,117,1.4f,dim);
    text("CLICK / F SWITCHES AND DOOR",22,94,1.35f,dim);
    text("WS WALK / ARROWS STRAFE",22,72,1.4f,dim);
    text("RMB LOOK / QE HEIGHT",22,50,1.4f,dim);
    text("H RESTORE / 3 DAY / 4 NIGHT",22,32,1.25f,dim);
    text("C PINBALL / SPACE LAUNCH",22,15,1.45f,blue);
    float cx=width*.73f*.5f,cy=height*.5f;
    drawQuad(cx-5,cy-.5f,10,1,{.6f,.8f,1,.5f});drawQuad(cx-.5f,cy-5,1,10,{.6f,.8f,1,.5f});
    glDisable(GL_BLEND);glEnable(GL_DEPTH_TEST);
}
