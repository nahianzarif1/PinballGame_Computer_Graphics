#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

#include "Shader.h"
#include "GameHub.h"
#include "RayTraceView.h"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

GameHub* hub = nullptr;
Shader* phongShader = nullptr;
Shader* gouraudShader = nullptr;
Shader* flatShader = nullptr;
float lastFrame = 0.0f;
float deltaTime = 0.0f;
bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool looking = false;
RayTraceView* rayView=nullptr;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

static bool edge(GLFWwindow* window, int key, bool& latch) {
    bool down = glfwGetKey(window, key) == GLFW_PRESS;
    bool fired = down && !latch;
    latch = down;
    return fired;
}

int main(int argc, char** argv) {
    std::string capturePath;
    int captureFrames=60;
    bool roomView=false, tableView=false, autoLaunch=false, lightsOff=false, fanOff=false;
    bool rayTracing=false,doorOpen=false,dayMode=false,liveCapture=false;
    int rayWidth=480,raySamples=4;
    bool rayWidthSpecified=false,raySamplesSpecified=false;
    ShadingMode captureShading=ShadingMode::PHONG;
    for(int i=1;i<argc;++i) {
        std::string arg=argv[i];
        if(arg=="--capture" && i+1<argc) capturePath=argv[++i];
        else if(arg=="--frames" && i+1<argc) captureFrames=std::max(1,std::atoi(argv[++i]));
        else if(arg=="--room") roomView=true;
        else if(arg=="--launch") autoLaunch=true;
        else if(arg=="--table") tableView=true;
        else if(arg=="--lights-off") lightsOff=true;
        else if(arg=="--fan-off") fanOff=true;
        else if(arg=="--raytrace")rayTracing=true;
        else if(arg=="--capture-live")liveCapture=true;
        else if(arg=="--door-open")doorOpen=true;
        else if(arg=="--day")dayMode=true;
        else if(arg=="--ray-width" && i+1<argc){rayWidth=std::clamp(std::atoi(argv[++i]),64,1920);rayWidthSpecified=true;}
        else if(arg=="--ray-samples" && i+1<argc){raySamples=std::clamp(std::atoi(argv[++i]),1,64);raySamplesSpecified=true;}
        else if(arg=="--shading" && i+1<argc) {
            std::string mode=argv[++i];captureShading=mode=="flat" ? ShadingMode::FLAT : mode=="gouraud" ? ShadingMode::GOURAUD : ShadingMode::PHONG;
            if(mode=="raytrace")rayTracing=true;
        }
    }
    // Find assets beside the executable, so double-clicking or launching from
    // another directory works as well as starting in the build folder.
    auto assets=std::filesystem::absolute(argv[0]).parent_path();
    if(std::filesystem::exists(assets/"shaders")) std::filesystem::current_path(assets);
    if (!glfwInit()) { std::cerr<<"GLFW initialization failed\n";return 1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    if(!capturePath.empty())glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Technology Gaming Lounge", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    try {
        phongShader = new Shader("shaders/phong.vert", "shaders/phong.frag");
        gouraudShader = new Shader("shaders/gouraud.vert", "shaders/gouraud.frag");
        flatShader = new Shader("shaders/flat.vert", "shaders/flat.frag");
        rayView = new RayTraceView();
        std::cout << "Shaders loaded successfully" << std::endl;
    } catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        delete phongShader;delete gouraudShader;delete flatShader;
        glfwDestroyWindow(window);glfwTerminate();
        return -1;
    }

    hub = new GameHub();
    if(roomView) hub->pinballCam=false;
    if(tableView || autoLaunch) hub->pinballCam=true;
    if(lightsOff) hub->toggleLights();
    if(fanOff) hub->toggleFan();
    if(doorOpen)hub->toggleDoor();
    hub->setDayMode(dayMode);
    hub->rayTracing=rayTracing;
    hub->machine.setShadingMode(captureShading);
    if(autoLaunch) hub->machine.launchFromLane(18);
    int renderedFrames=0;
    bool wasRayTracing=false;
    unsigned int whiteTexture=0;
    glGenTextures(1,&whiteTexture);glBindTexture(GL_TEXTURE_2D,whiteTexture);
    const unsigned char whitePixel[3]={255,255,255};
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,1,1,0,GL_RGB,GL_UNSIGNED_BYTE,whitePixel);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    std::cout << "\n========================================\n";
    std::cout << "   GAME HUB PINBALL\n";
    std::cout << "========================================\n";
    std::cout << "PINBALL:\n";
    std::cout << "  A / D     Left / right flipper\n";
    std::cout << "  ENTER     Hold to charge, release to launch\n";
    std::cout << "  SPACE     Launch ready ball\n";
    std::cout << "  R         Reset complete game\n";
    std::cout << "  C         Toggle pinball / hub camera\n";
    std::cout << "ROOM:\n";
    std::cout << "  W / S     Walk forward / back\n";
    std::cout << "  Arrows    Strafe\n";
    std::cout << "  Q / E     Up / down\n";
    std::cout << "  RMB       Look around\n";
    std::cout << "  F / LMB   Use wall switch\n";
    std::cout << "  H         Restore reference room view\n";
    std::cout << "ENVIRONMENT:\n";
    std::cout << "  1         Room lights on/off\n";
    std::cout << "  2         Ceiling fan on/off\n";
    std::cout << "  3 / 4     Day / night\n";
    std::cout << "  B / G / P Flat / Gouraud / Phong\n";
    std::cout << "  T         Ray tracing on/off (CPU BVH, shadows, reflection, refraction)\n";
    std::cout << "  O         Open/close refrigerator door\n";
    std::cout << "  ESC       Exit\n";
    std::cout << "========================================\n\n";

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = std::min(currentFrame - lastFrame, 0.1f);
        if(!capturePath.empty()) deltaTime=1.0f/60;
        lastFrame = currentFrame;

        processInput(window);
        hub->update(deltaTime);
        if(hub->rayTracing!=wasRayTracing){rayView->invalidate();wasRayTracing=hub->rayTracing;}

        glm::vec3 sky = hub->skyColor();
        // Reset critical 3D render state every frame.  The HUD and transparent
        // passes temporarily alter GL state, so doing this here guarantees the
        // next 3D frame always starts from a known, visible configuration.
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glClearColor(sky.r, sky.g, sky.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Shader* currentShader = phongShader;
        switch (hub->machine.getShadingMode()) {
            case ShadingMode::FLAT: currentShader = flatShader; break;
            case ShadingMode::GOURAUD: currentShader = gouraudShader; break;
            default: currentShader = phongShader; break;
        }
        currentShader->use();

        int fbw = SCR_WIDTH, fbh = SCR_HEIGHT;
        glfwGetFramebufferSize(window, &fbw, &fbh);
        int viewW = std::max(1, static_cast<int>(fbw * 0.73f));
        glViewport(0, 0, viewW, std::max(fbh, 1));

        glm::mat4 view = hub->activeCamera().getViewMatrix();
        glm::mat4 projection = hub->activeCamera().getProjectionMatrix((float)viewW / (float)std::max(fbh, 1));
        currentShader->setMat4("view", view);
        currentShader->setMat4("projection", projection);

        bool rayReady=false;
        if(hub->rayTracing) {
            if(!capturePath.empty() && !liveCapture) {
                if(renderedFrames+1>=captureFrames) {
                    rayView->capture(*hub,viewW,std::max(fbh,1),rayWidth,raySamples);rayReady=true;
                }
            } else rayReady=rayView->update(*hub,viewW,std::max(fbh,1),rayWidthSpecified ? rayWidth : 0,
                                            raySamplesSpecified ? raySamples : 0);
            hub->rayFrameMilliseconds=rayView->milliseconds();
            hub->rayFrameWidth=rayView->width();hub->rayRefining=rayView->refining();
        }
        if(rayReady)rayView->draw();
        else {
            glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,whiteTexture);
            hub->render(currentShader->ID);
        }

        glViewport(0, 0, fbw, std::max(fbh, 1));
        hub->renderHUD(fbw, fbh);

        char title[128];
        std::snprintf(title, sizeof(title), "GameHub Pinball  |  %s  |  %s  |  Score %06d  |  %d FPS",
                      hub->pinballCam ? "Table Cam" : "Technology Lounge",
                      hub->rayTracing ? (rayReady ? (rayView->refining()?"Ray tracing: refining":"Ray tracing") : "Ray tracing: rendering") : "Raster",
                      hub->machine.score,
                      std::max(1, static_cast<int>(1.0f / std::max(deltaTime, 0.0001f))));
        glfwSetWindowTitle(window, title);

        if(!capturePath.empty() && ++renderedFrames>=captureFrames &&
           (!hub->rayTracing || !liveCapture || (rayReady&&!rayView->refining()))) {
            std::vector<unsigned char> pixels(static_cast<size_t>(fbw)*fbh*3);
            glPixelStorei(GL_PACK_ALIGNMENT,1);
            glReadPixels(0,0,fbw,fbh,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
            std::ofstream out(capturePath,std::ios::binary);
            out<<"P6\n"<<fbw<<" "<<fbh<<"\n255\n";
            for(int y=fbh-1;y>=0;--y) out.write(reinterpret_cast<const char*>(pixels.data()+static_cast<size_t>(y)*fbw*3),fbw*3);
            std::cout<<"Captured frame: "<<capturePath<<" | score "<<hub->machine.score<<" | state "<<static_cast<int>(hub->machine.ballState)
                     <<" | renderer "<<(rayReady ? "ray tracing" : "raster")<<" | ray frame "<<hub->rayFrameMilliseconds<<" ms\n";
            if(rayReady)std::cout<<"Ray image detail: "<<rayView->width()<<" pixels wide, "<<rayView->samples()<<" samples/pixel\n";
            glfwSetWindowShouldClose(window,true);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteTextures(1,&whiteTexture);
    delete hub;
    delete phongShader;
    delete gouraudShader;
    delete flatShader;
    delete rayView;rayView=nullptr;
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    bool leftFlip = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    bool rightFlip = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    bool pull = glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_KP_ENTER) == GLFW_PRESS;
    hub->machine.setLeftFlipperPowered(leftFlip);
    hub->machine.setRightFlipperPowered(rightFlip);
    if(pull && hub->machine.ballState==BallState::READY && !hub->machine.gameOver) hub->pinballCam=true;
    hub->machine.setPlungerPulling(pull);

    Camera& cam = hub->activeCamera();
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cam.processKeyboard(0, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cam.processKeyboard(1, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) cam.processKeyboard(2, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) cam.processKeyboard(3, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) cam.processKeyboard(4, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) cam.processKeyboard(5, deltaTime);

    static bool k1 = false, k2 = false, k3 = false, k4 = false, kC = false, kR = false, kSpace = false;
    static bool kB = false, kG = false, kP = false, kT=false, kO=false, kF=false, kH=false;

    if (edge(window, GLFW_KEY_1, k1)) hub->toggleLights();
    if (edge(window, GLFW_KEY_2, k2)) hub->toggleFan();
    if (edge(window, GLFW_KEY_O, kO)) hub->toggleDoor();
    if (edge(window, GLFW_KEY_T, kT)) hub->toggleRayTracing();
    if (edge(window, GLFW_KEY_3, k3)) hub->setDayMode(true);
    if (edge(window, GLFW_KEY_4, k4)) hub->setDayMode(false);
    if (edge(window, GLFW_KEY_C, kC)) hub->toggleCamera();
    if (edge(window, GLFW_KEY_F, kF)) hub->interactSwitches(cam.position,cam.front);
    if (edge(window, GLFW_KEY_H, kH)) {hub->resetRoomCamera();hub->pinballCam=false;}
    if (edge(window, GLFW_KEY_R, kR)) hub->resetAll();
    if (edge(window, GLFW_KEY_SPACE, kSpace)) {
        if(hub->machine.ballState==BallState::READY && !hub->machine.gameOver) hub->pinballCam=true;
        hub->machine.launchFromLane(18.0f);
    }
    if (edge(window, GLFW_KEY_B, kB)) hub->setShadingMode(ShadingMode::FLAT);
    if (edge(window, GLFW_KEY_G, kG)) hub->setShadingMode(ShadingMode::GOURAUD);
    if (edge(window, GLFW_KEY_P, kP)) hub->setShadingMode(ShadingMode::PHONG);
}

void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, std::max(width, 1), std::max(height, 1));
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int) {
    if(button==GLFW_MOUSE_BUTTON_LEFT && action==GLFW_PRESS && hub && !hub->pinballCam && !looking) {
        double mouseX=0,mouseY=0;int width=1,height=1,fbw=1,fbh=1;
        glfwGetCursorPos(window,&mouseX,&mouseY);glfwGetWindowSize(window,&width,&height);glfwGetFramebufferSize(window,&fbw,&fbh);
        float x=static_cast<float>(mouseX)*fbw/std::max(1,width),y=static_cast<float>(mouseY)*fbh/std::max(1,height);
        int control=hub->hud.roomSwitchAt(x,fbh-y,fbw,fbh);
        if(control==1)hub->toggleLights();
        else if(control==2)hub->toggleFan();
        else if(control>=3 && control<=5)hub->setShadingMode(static_cast<ShadingMode>(control-3));
        else if(control==6){hub->toggleRayTracing();rayView->invalidate();}
        else if(control==7)hub->toggleDoor();
        else if(x<fbw*.73f) {
            glm::vec4 clip(2*x/(fbw*.73f)-1,1-2*y/std::max(1,fbh),-1,1);
            auto& camera=hub->activeCamera();
            glm::vec4 eye=glm::inverse(camera.getProjectionMatrix(fbw*.73f/std::max(1,fbh)))*clip;
            eye.z=-1;eye.w=0;
            glm::vec3 direction=glm::normalize(glm::vec3(glm::inverse(camera.getViewMatrix())*eye));
            hub->interactSwitches(camera.position,direction);
        }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        looking = (action == GLFW_PRESS);
        glfwSetInputMode(window, GLFW_CURSOR, looking ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        firstMouse = true;
    }
}

void mouse_callback(GLFWwindow*, double xpos, double ypos) {
    if (!looking || !hub) {
        firstMouse = true;
        return;
    }
    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }
    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos);
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);
    hub->activeCamera().processMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow*, double, double yoffset) {
    if (hub) {
        hub->activeCamera().processMouseScroll(static_cast<float>(yoffset));
    }
}
