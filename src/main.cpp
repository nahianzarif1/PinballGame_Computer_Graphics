#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <algorithm>
#include <cstdio>

#include "Shader.h"
#include "GameHub.h"

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

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GameHub Pinball", NULL, NULL);
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
        std::cout << "Shaders loaded successfully" << std::endl;
    } catch (...) {
        std::cout << "Failed to load shaders. Make sure shaders/ is in the build folder." << std::endl;
        return -1;
    }

    hub = new GameHub();

    std::cout << "\n========================================\n";
    std::cout << "   GAME HUB PINBALL\n";
    std::cout << "========================================\n";
    std::cout << "PINBALL:\n";
    std::cout << "  A / D     Left / right flipper\n";
    std::cout << "  ENTER     Hold to charge, release to launch\n";
    std::cout << "  SPACE     Reset ball and launch\n";
    std::cout << "  R         Reset complete game\n";
    std::cout << "  C         Toggle pinball / hub camera\n";
    std::cout << "ROOM:\n";
    std::cout << "  W / S     Walk forward / back\n";
    std::cout << "  Arrows    Strafe\n";
    std::cout << "  Q / E     Up / down\n";
    std::cout << "  RMB       Look around\n";
    std::cout << "ENVIRONMENT:\n";
    std::cout << "  1         Room lights on/off\n";
    std::cout << "  2         Fan on/off\n";
    std::cout << "  3 / 4     Day / night\n";
    std::cout << "  F5/F6/F7  Flat / Gouraud / Phong\n";
    std::cout << "  ESC       Exit\n";
    std::cout << "========================================\n\n";

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        hub->update(deltaTime);

        glm::vec3 sky = hub->skyColor();
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

        hub->render(currentShader->ID);

        glViewport(0, 0, fbw, std::max(fbh, 1));
        hub->renderHUD(fbw, fbh);

        char title[128];
        std::snprintf(title, sizeof(title), "GameHub Pinball  |  %s  |  Score %06d  |  %d FPS",
                      hub->pinballCam ? "Table Cam" : "Hub Cam",
                      hub->machine.score,
                      std::max(1, static_cast<int>(1.0f / std::max(deltaTime, 0.0001f))));
        glfwSetWindowTitle(window, title);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete hub;
    delete phongShader;
    delete gouraudShader;
    delete flatShader;
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
    hub->machine.setPlungerPulling(pull);

    Camera& cam = hub->activeCamera();
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cam.processKeyboard(0, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cam.processKeyboard(1, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) cam.processKeyboard(2, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) cam.processKeyboard(3, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) cam.processKeyboard(4, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) cam.processKeyboard(5, deltaTime);

    static bool k1 = false, k2 = false, k3 = false, k4 = false, kC = false, kR = false, kSpace = false;
    static bool f5 = false, f6 = false, f7 = false;

    if (edge(window, GLFW_KEY_1, k1)) hub->toggleLights();
    if (edge(window, GLFW_KEY_2, k2)) hub->toggleFan();
    if (edge(window, GLFW_KEY_3, k3)) hub->setDayMode(true);
    if (edge(window, GLFW_KEY_4, k4)) hub->setDayMode(false);
    if (edge(window, GLFW_KEY_C, kC)) hub->toggleCamera();
    if (edge(window, GLFW_KEY_R, kR)) hub->resetAll();
    if (edge(window, GLFW_KEY_SPACE, kSpace)) hub->machine.launchFromLane(18.0f);
    if (edge(window, GLFW_KEY_F5, f5)) hub->machine.setShadingMode(ShadingMode::FLAT);
    if (edge(window, GLFW_KEY_F6, f6)) hub->machine.setShadingMode(ShadingMode::GOURAUD);
    if (edge(window, GLFW_KEY_F7, f7)) hub->machine.setShadingMode(ShadingMode::PHONG);
}

void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, std::max(width, 1), std::max(height, 1));
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int) {
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
