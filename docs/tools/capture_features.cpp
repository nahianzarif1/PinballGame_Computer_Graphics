#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include "Shader.h"
#include "GameHub.h"
#include "RayTraceView.h"

constexpr int WIDTH=1600, HEIGHT=1000;

void aim(Camera& camera, glm::vec3 position, glm::vec3 target, float fov=56) {
    glm::vec3 direction=glm::normalize(target-position);
    camera.position=position;
    camera.yaw=glm::degrees(std::atan2(direction.y,direction.x));
    camera.pitch=glm::degrees(std::asin(direction.z));
    camera.zoom=fov;
    camera.processMouseMovement(0,0);
}
void place(PinballMachine& machine, glm::vec2 position, glm::vec2 velocity) {
    machine.ballState=BallState::IN_PLAY;
    machine.ballSaveTimer=0;
    machine.ball.transform.position=glm::vec3(position,machine.playfield.calculateZ(position.y)+machine.ball.radius);
    machine.ball.velocity=glm::vec3(velocity,0);
}
void advance(GameHub& hub, float seconds) {
    for(int i=0;i<static_cast<int>(seconds*60);++i) hub.update(1.0f/60);
}
void completeBank(PinballMachine& machine) {
    machine.resetGame();
    for(auto& target: machine.targets) {
        place(machine,{target.transform.position.x,target.transform.position.y-.2f},{0,5});
        machine.checkCollisions();
    }
    for(auto& target:machine.targets) target.update(.22f);
}

int main(int argc,char** argv) {
    if(argc<2 || argc>3) return 1;
    bool animationsOnly=argc==3 && std::string(argv[2])=="--animations-only";
    bool scoringOnly=argc==3 && std::string(argv[2])=="--scoring-only";
    auto output=std::filesystem::absolute(argv[1]);
    std::filesystem::create_directories(output);
    if(!glfwInit()) return 2;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
    glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
    glfwWindowHint(GLFW_SAMPLES,4);
#ifdef __APPLE__
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER,GLFW_FALSE);
#endif
    GLFWwindow* window=glfwCreateWindow(WIDTH,HEIGHT,"README feature captures",nullptr,nullptr);
    if(!window) return 3;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return 4;
    {
        Shader flat("shaders/flat.vert","shaders/flat.frag");
        Shader gouraud("shaders/gouraud.vert","shaders/gouraud.frag");
        Shader phong("shaders/phong.vert","shaders/phong.frag");
        RayTraceView rays;
        unsigned texture=0;
        glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);
        unsigned char white[3]={255,255,255};
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,1,1,0,GL_RGB,GL_UNSIGNED_BYTE,white);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        std::vector<unsigned char> pixels(WIDTH*HEIGHT*3);
        auto save=[&](const std::string& name,GameHub& hub,bool raytrace=false) {
            int viewWidth=static_cast<int>(WIDTH*.73f);
            if(raytrace) {
                hub.fanOn=false;hub.fanAngularSpeed=0;hub.update(0);
                hub.rayTracing=true;
                rays.capture(hub,viewWidth,HEIGHT,1168,2);
                hub.rayFrameWidth=rays.width();
                hub.rayFrameMilliseconds=rays.milliseconds();hub.rayRefining=false;
            }
            glEnable(GL_DEPTH_TEST);glEnable(GL_MULTISAMPLE);
            glDepthMask(GL_TRUE);glDisable(GL_BLEND);
            auto sky=hub.skyColor();glClearColor(sky.r,sky.g,sky.b,1);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            glViewport(0,0,viewWidth,HEIGHT);
            Shader* shader=hub.machine.getShadingMode()==ShadingMode::FLAT?&flat:
                hub.machine.getShadingMode()==ShadingMode::GOURAUD?&gouraud:&phong;
            shader->use();
            shader->setMat4("view",hub.activeCamera().getViewMatrix());
            shader->setMat4("projection",hub.activeCamera().getProjectionMatrix(float(viewWidth)/HEIGHT));
            if(raytrace) rays.draw();
            else {glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,texture);hub.render(shader->ID);}
            glViewport(0,0,WIDTH,HEIGHT);hub.renderHUD(WIDTH,HEIGHT);
            glPixelStorei(GL_PACK_ALIGNMENT,1);
            glReadPixels(0,0,WIDTH,HEIGHT,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
            std::ofstream file(output/(name+".ppm"),std::ios::binary);
            file<<"P6\n"<<WIDTH<<" "<<HEIGHT<<"\n255\n";
            for(int y=HEIGHT-1;y>=0;--y) file.write(reinterpret_cast<const char*>(pixels.data())+size_t(y)*WIDTH*3,WIDTH*3);
            glfwSwapBuffers(window);glfwPollEvents();
            std::cout<<name<<" | score="<<hub.machine.score<<" bonus="<<hub.machine.bonus
                <<" multiplier="<<hub.machine.multiplier<<" lives="<<hub.machine.lives<<std::endl;
        };
        auto shot=[&](const char* name,const std::function<void(GameHub&)>& configure={},bool raytrace=false) {
            GameHub hub;advance(hub,2);
            if(configure) configure(hub);
            save(name,hub,raytrace);
        };
        if(!animationsOnly && !scoringOnly) {
        shot("room-overview");
        shot("table-ready",[](auto& h){h.pinballCam=true;});
        shot("plunger-charged",[](auto& h){h.pinballCam=true;h.machine.setPlungerPulling(true);advance(h,.65f);});
        shot("active-gameplay",[](auto& h){h.pinballCam=true;h.machine.launchFromLane(18);advance(h,1.55f);});
        shot("flippers-powered",[](auto& h){h.pinballCam=true;h.machine.setLeftFlipperPowered(true);h.machine.setRightFlipperPowered(true);advance(h,.16f);});
        shot("bumper-hit",[](auto& h){h.pinballCam=true;auto& b=h.machine.bumpers[0];place(h.machine,{b.transform.position.x,b.transform.position.y-b.radius-h.machine.ball.radius+.01f},{0,6});h.machine.checkCollisions();});
        shot("target-bank",[](auto& h){h.pinballCam=true;completeBank(h.machine);});
        shot("drain-bonus",[](auto& h){h.pinballCam=true;completeBank(h.machine);place(h.machine,{0,-7},{0,-5});h.machine.checkCollisions();});
        shot("ball-save",[](auto& h){h.pinballCam=true;h.machine.launchFromLane(18);h.machine.ball.transform.position.y=-7;h.machine.ball.transform.position.x=0;h.machine.checkCollisions();});
        shot("game-over",[](auto& h){h.pinballCam=true;completeBank(h.machine);for(int i=0;i<3;++i){place(h.machine,{0,-7},{0,-5});h.machine.checkCollisions();}});
        shot("restart-session-best",[](auto& h){h.pinballCam=true;completeBank(h.machine);place(h.machine,{0,-7},{0,-5});h.machine.checkCollisions();h.machine.resetGame();});
        shot("lights-off",[](auto& h){h.toggleLights();h.update(0);});
        shot("day-mode",[](auto& h){h.setDayMode(true);h.update(0);});
        shot("fan-running",[](auto& h){aim(h.hubCamera,{1.5f,1.4f,2.8f},{2.3f,4.1f,5.07f},58);});
        shot("fan-stopped",[](auto& h){h.toggleFan();advance(h,2);aim(h.hubCamera,{1.5f,1.4f,2.8f},{2.3f,4.1f,5.07f},58);});
        shot("fridge-closed",[](auto& h){aim(h.hubCamera,{-2.9f,3,2.2f},{-4.66f,5.6f,1.7f},60);});
        shot("fridge-open",[](auto& h){h.toggleDoor();advance(h,1.1f);aim(h.hubCamera,{-2.9f,3,2.2f},{-4.66f,5.6f,1.7f},60);});
        shot("shading-flat",[](auto& h){h.setShadingMode(ShadingMode::FLAT);});
        shot("shading-gouraud",[](auto& h){h.setShadingMode(ShadingMode::GOURAUD);});
        shot("shading-phong",[](auto& h){h.setShadingMode(ShadingMode::PHONG);});
        shot("camera-exploration",[](auto& h){aim(h.hubCamera,{0,-5.6f,2.65f},{-1.0f,1.0f,1.8f},65);});
        shot("room-raytrace",{},true);
        shot("glass-raytrace",[](auto& h){aim(h.hubCamera,{-2.9f,3.2f,2.1f},{-4.66f,5.75f,1.7f},56);},true);
        shot("table-raytrace",[](auto& h){h.pinballCam=true;place(h.machine,{-.1f,-.65f},{0,0});},true);
        }
        if(!animationsOnly) {
            shot("orbit-hit",[](auto& h){h.pinballCam=true;place(h.machine,{-2.9f,1.6f},{0,8});h.machine.checkCollisions();place(h.machine,{-1,4.5f},{0,8});h.machine.checkCollisions();if(h.machine.score!=250 || h.machine.bonus!=100)throw std::runtime_error("Orbit capture did not produce the intended event");});
            shot("slingshot-hit",[](auto& h){h.pinballCam=true;place(h.machine,{-1.50f,-3.15f},{-5,-2});h.machine.checkCollisions();if(h.machine.score!=10)throw std::runtime_error("Slingshot capture did not produce the intended event");});
        }
        if(!scoringOnly) {
        shot("wall-switches",[](auto& h){aim(h.hubCamera,{3.5f,3.1f,2.4f},{5.0f,6.54f,2.15f},52);h.interactSwitches(h.hubCamera.position,glm::normalize(glm::vec3(5.35f,6.54f,2.15f)-h.hubCamera.position));advance(h,2);});
        {
            GameHub h;aim(h.hubCamera,{-2.9f,3,2.2f},{-4.66f,5.6f,1.7f},60);h.toggleDoor();
            for(int i=0;i<80;++i){if(i==14 || i==22 || i==45)h.toggleDoor();save("door-frame-"+std::to_string(i),h);advance(h,.05f);}
        }
        {
            GameHub h;aim(h.hubCamera,{1.5f,1.4f,2.8f},{2.3f,4.1f,5.07f},58);
            for(int i=0;i<80;++i){if(i==45)h.toggleFan();save("fan-frame-"+std::to_string(i),h);advance(h,.05f);}
        }
        }
        glDeleteTextures(1,&texture);
    }
    glfwDestroyWindow(window);glfwTerminate();return 0;
}
