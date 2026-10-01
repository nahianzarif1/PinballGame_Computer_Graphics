#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>
#include <algorithm>

#include "Shader.h"
#include "Camera.h"
#include "PinballMachine.h"
#include "Mesh.h"

// Window settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Global state
PinballMachine* machine = nullptr;
Shader* phongShader = nullptr;
Shader* gouraudShader = nullptr;
Shader* flatShader = nullptr;
float lastFrame = 0.0f;
float deltaTime = 0.0f;

// Control state
bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void renderUI(GLFWwindow* window);

int main() {
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Interactive Pinball Machine", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    // Capture mouse for camera control
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Load GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    
    // Load shaders
    try {
        phongShader = new Shader("shaders/phong.vert", "shaders/phong.frag");
        gouraudShader = new Shader("shaders/gouraud.vert", "shaders/gouraud.frag");
        flatShader = new Shader("shaders/flat.vert", "shaders/flat.frag");
        std::cout << "Shaders loaded successfully" << std::endl;
    } catch (...) {
        std::cout << "Failed to load shaders. Make sure shaders directory is in the build folder." << std::endl;
        return -1;
    }
    
    // Test simple rendering
    std::cout << "Testing basic rendering..." << std::endl;
    
    // Create pinball machine
    machine = new PinballMachine();
    
    glClearColor(0.07f, 0.08f, 0.12f, 1.0f);
    
    // Disable backface culling for debugging
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    // glFrontFace(GL_CCW);
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "   3D INTERACTIVE PINBALL MACHINE" << std::endl;
    std::cout << "   GAME HUB EDITION" << std::endl;
    std::cout << "========================================\n" << std::endl;
    std::cout << "CONTROLS:" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "OBJECT CONTROL:" << std::endl;
    std::cout << "  TAB    - Select next object" << std::endl;
    std::cout << "  W/S    - Move forward/backward" << std::endl;
    std::cout << "  A/D    - Move left/right" << std::endl;
    std::cout << "  Q/E    - Move up/down" << std::endl;
    std::cout << "  Z/X    - Nudge flipper angle" << std::endl;
    std::cout << "  R      - Reset selected object" << std::endl;
    std::cout << "\nPINBALL:" << std::endl;
    std::cout << "  LEFT ARROW  - Left flipper" << std::endl;
    std::cout << "  RIGHT ARROW - Right flipper" << std::endl;
    std::cout << "  SPACE / DOWN - Hold to pull plunger, release to launch" << std::endl;
    std::cout << "\nLIGHT CONTROL:" << std::endl;
    std::cout << "  F1/F2/F3 - Select Light 1/2/3" << std::endl;
    std::cout << "  W/S/A/D/Q/E - Move selected light" << std::endl;
    std::cout << "  +/-    - Adjust light intensity" << std::endl;
    std::cout << "  R      - Reset selected light" << std::endl;
    std::cout << "\nROOM CONTROLS:" << std::endl;
    std::cout << "  L      - Toggle room lights" << std::endl;
    std::cout << "  F      - Toggle ceiling fan" << std::endl;
    std::cout << "  1/2/3  - Switch light switches" << std::endl;
    std::cout << "  [ / ]  - Adjust ambient light" << std::endl;
    std::cout << "  { / }  - Adjust diffuse light" << std::endl;
    std::cout << "  < / >  - Adjust specular light" << std::endl;
    std::cout << "\nSHADING:" << std::endl;
    std::cout << "  1      - Flat shading" << std::endl;
    std::cout << "  2      - Gouraud shading" << std::endl;
    std::cout << "  3      - Phong shading" << std::endl;
    std::cout << "\nCAMERA:" << std::endl;
    std::cout << "  Mouse  - Look around" << std::endl;
    std::cout << "  Scroll - Zoom" << std::endl;
    std::cout << "\nOTHER:" << std::endl;
    std::cout << "  ESC    - Exit" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // Main game loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // Process input
        processInput(window);
        
        // Update machine
        machine->update(deltaTime);
        
        // Render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Select shader based on shading mode
        Shader* currentShader;
        switch (machine->getShadingMode()) {
            case ShadingMode::FLAT:
                currentShader = flatShader;
                break;
            case ShadingMode::GOURAUD:
                currentShader = gouraudShader;
                break;
            case ShadingMode::PHONG:
            default:
                currentShader = phongShader;
                break;
        }
        
        currentShader->use();
        
        // Set view and projection matrices
        glm::mat4 view = machine->camera.getViewMatrix();
        int fbw = SCR_WIDTH, fbh = SCR_HEIGHT;
        glfwGetFramebufferSize(window, &fbw, &fbh);
        glm::mat4 projection = machine->camera.getProjectionMatrix((float)fbw / (float)std::max(fbh, 1));
        
        currentShader->setMat4("view", view);
        currentShader->setMat4("projection", projection);
        
        machine->render(currentShader->ID);
        
        // Render UI info
        renderUI(window);
        
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Cleanup
    delete machine;
    delete phongShader;
    delete gouraudShader;
    delete flatShader;
    
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    bool leftFlip = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
    bool rightFlip = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS ||
                     glfwGetKey(window, GLFW_KEY_SLASH) == GLFW_PRESS ||
                     glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS;
    bool pullPlunger = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ||
                       glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;
    machine->setLeftFlipperPowered(leftFlip);
    machine->setRightFlipperPowered(rightFlip);
    machine->setPlungerPulling(pullPlunger);
    
    // Camera movement (WASD + QE for up/down)
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        // Shift held - camera movement
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            machine->camera.processKeyboard(0, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            machine->camera.processKeyboard(1, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            machine->camera.processKeyboard(2, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            machine->camera.processKeyboard(3, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            machine->camera.processKeyboard(4, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            machine->camera.processKeyboard(5, deltaTime);
    } else {
        // No shift - object/light control
        glm::vec3 movement(0.0f);
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            movement.y = 1.0f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            movement.y = -1.0f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            movement.x = -1.0f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            movement.x = 1.0f;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            movement.z = 1.0f;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            movement.z = -1.0f;
        
        if (movement != glm::vec3(0.0f)) {
            if (machine->selectionMode == SelectionMode::OBJECT) {
                machine->moveSelectedObject(movement);
            } else {
                machine->moveSelectedLight(movement);
            }
        }
        
        // Rotation for flippers
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
            machine->rotateSelectedObject(-1.0f);
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
            machine->rotateSelectedObject(1.0f);
        
        // Light intensity
        if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS)
            machine->adjustLightIntensity(1.0f);
        if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
            machine->adjustLightIntensity(-1.0f);
        
        // Reset
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            if (machine->selectionMode == SelectionMode::OBJECT) {
                machine->resetSelectedObject();
            } else {
                machine->resetSelectedLight();
            }
        }
    }
    
    // Object selection (TAB)
    static bool tabPressed = false;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !tabPressed) {
        machine->selectNextObject();
        tabPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE) {
        tabPressed = false;
    }
    
    // Light selection (F1, F2, F3)
    static bool f1Pressed = false;
    static bool f2Pressed = false;
    static bool f3Pressed = false;
    
    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS && !f1Pressed) {
        machine->selectLight(0);
        f1Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_RELEASE) {
        f1Pressed = false;
    }
    
    if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS && !f2Pressed) {
        machine->selectLight(1);
        f2Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_RELEASE) {
        f2Pressed = false;
    }
    
    if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS && !f3Pressed) {
        machine->selectLight(2);
        f3Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_RELEASE) {
        f3Pressed = false;
    }
    
    // Shading mode (1, 2, 3)
    static bool key1Pressed = false;
    static bool key2Pressed = false;
    static bool key3Pressed = false;

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !key1Pressed) {
        machine->setShadingMode(ShadingMode::FLAT);
        key1Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE) {
        key1Pressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !key2Pressed) {
        machine->setShadingMode(ShadingMode::GOURAUD);
        key2Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE) {
        key2Pressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !key3Pressed) {
        machine->setShadingMode(ShadingMode::PHONG);
        key3Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE) {
        key3Pressed = false;
    }

    // Room controls
    static bool keyLPressed = false;
    static bool keyFPressed = false;

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !keyLPressed) {
        machine->toggleRoomLights();
        keyLPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE) {
        keyLPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !keyFPressed) {
        machine->toggleRoomFan();
        keyFPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
        keyFPressed = false;
    }

    // Light intensity controls
    static float ambientIntensity = 0.1f;
    static float diffuseIntensity = 0.8f;
    static float specularIntensity = 1.0f;

    if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS) {
        ambientIntensity = std::max(0.0f, ambientIntensity - 0.01f);
        machine->setRoomAmbient(ambientIntensity);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS) {
        ambientIntensity = std::min(1.0f, ambientIntensity + 0.01f);
        machine->setRoomAmbient(ambientIntensity);
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        diffuseIntensity = std::max(0.0f, diffuseIntensity - 0.01f);
        machine->setRoomDiffuse(diffuseIntensity);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        diffuseIntensity = std::min(1.0f, diffuseIntensity + 0.01f);
        machine->setRoomDiffuse(diffuseIntensity);
    }

    if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS) {
        specularIntensity = std::max(0.0f, specularIntensity - 0.01f);
        machine->setRoomSpecular(specularIntensity);
    }
    if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) {
        specularIntensity = std::min(1.0f, specularIntensity + 0.01f);
        machine->setRoomSpecular(specularIntensity);
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    
    lastX = xpos;
    lastY = ypos;
    
    machine->camera.processMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    machine->camera.processMouseScroll(yoffset);
}

void renderUI(GLFWwindow* window) {
    // Simple console-based UI info (could be extended with text rendering)
    static int frameCount = 0;
    static float lastUIUpdate = 0.0f;
    
    frameCount++;
    float currentTime = glfwGetTime();
    
    if (currentTime - lastUIUpdate >= 1.0f) {
        std::string modeStr;
        switch (machine->getShadingMode()) {
            case ShadingMode::FLAT: modeStr = "FLAT"; break;
            case ShadingMode::GOURAUD: modeStr = "GOURAUD"; break;
            case ShadingMode::PHONG: modeStr = "PHONG"; break;
        }
        
        std::string selectionStr;
        if (machine->selectionMode == SelectionMode::OBJECT) {
            const char* objectNames[] = {"Ball", "Bumper 1", "Bumper 2", "Bumper 3", "Left Flipper", "Right Flipper", "Plunger"};
            selectionStr = "OBJECT: " + std::string(objectNames[machine->selectedObjectIndex]);
        } else {
            selectionStr = "LIGHT: " + std::to_string(machine->selectedLightIndex + 1);
        }
        
        glfwSetWindowTitle(window, ("3D Pinball - " + modeStr + " - " + selectionStr + " - " + std::to_string(frameCount) + " FPS").c_str());
        
        frameCount = 0;
        lastUIUpdate = currentTime;
    }
}
