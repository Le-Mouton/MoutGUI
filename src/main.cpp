#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.hpp"
#include <thread>
#include <chrono>
#include <atomic>
#include "menu.hpp"
#include "repere.hpp"
#include "callback.hpp"
#include "ttf-reader.hpp"
#include "text.hpp"
#include "button.hpp"
#include "slider.hpp"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1280;

// camera
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw   = -90.0f;   // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch =  0.0f;
float lastX =  800.0f / 2.0;
float lastY =  600.0 / 2.0;
float fov   =  45.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

double color0;
double color1;
double color2;

bool poly = false;

std::vector<Item*> item_list;

std::vector<Item*> menu_content;

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return 1;
    }

    // Contexte OpenGL 3.3 Core (macOS supporte jusqu'à 4.1 Core)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // requis sur macOS
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GUI", nullptr, nullptr);
    if (!window) {
        std::cerr << "Window creation failed\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Charge les fonctions OpenGL via GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        return 1;
    }

   // glEnable(GL_DEPTH_TEST);

    // Infos basiques
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";

    Slider monSlider(window, 0.5f, 0.5f, 300.0f, 40.0f, 1.f, 10.0f, 
                     "shader/shader.vs", "shader/shader.fs", color0);
    monSlider.priority = 100;  // Haute priorité
    
    Slider monSlider1(window, 0.5f, 0.3f, 300.0f, 40.0f, 1.f, 10.0f, 
                      "shader/shader.vs", "shader/shader.fs", color1);
    monSlider1.priority = 100;
    
    Slider monSlider2(window, 0.5f, 0.1f, 300.0f, 40.0f, 1.f, 10.0f, 
                      "shader/shader.vs", "shader/shader.fs", color2);
    monSlider2.priority = 100;
    
    Text monText(window, -0.8f, 0.8f, 0.1f, "shader/shader.vs", 
                 "shader/shader.fs", "Je s'appelle groot!");
    monText.priority = 50;  // Priorité moyenne
    
    Button monBoutton(window, "Clique ici", 0.0f, 0.0f, 0.05f, "dark", 4.0f, 
                      "shader/shader.vs", "shader/shader.fs", poly);
    monBoutton.priority = 100;
    
    // Créer le menu APRÈS les composants
    std::vector<Item*> menu_content = {&monSlider, &monSlider1, &monSlider2, &monBoutton};
    
    Menu monMenu(window, "right", "top", 200.0f, 1000.0f, "dark", 4.0f, 
                 "shader/shader.vs", "shader/shader.fs", menu_content);
    monMenu.priority = 10;  // Priorité basse
    
    // Liste pour les callbacks : composants AVANT le menu
    std::vector<Item*> item_list = {
        &monSlider, &monSlider1, &monSlider2, &monBoutton,  // Haute priorité
        //&monText,                                            // Moyenne
        &monMenu                                             // Basse priorité
    };

    CallBack callBackManager(window, item_list);
    
    monMenu.loadVertices();
    monText.loadVertices();
    

    monMenu.renderSetUp();
    monText.renderSetUp();


    glPointSize(8);

    // --- Thread Render ---
    while (!glfwWindowShouldClose(window)) {

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(color2, color1, color0, 1.0f);
        glClearStencil(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Matrices
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        monMenu.renderItem(poly);
        monText.renderItem(poly);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = static_cast<float>(2.5 * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        poly = !poly;

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}



