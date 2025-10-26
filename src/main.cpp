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

bool poly = false;

std::vector<Item*> item_list;

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

    // 1️⃣ Charger le glyphe
    GlyphData g = loadTTFGlyph("fonts/arial.ttf", 24);
    auto vertices = buildFilledGlyph(g, 1.0f/2048.0f, 24);

    // 3️⃣ Envoi à OpenGL
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);


    Menu monMenu(window, "right", "center", 100.0f, 1000.0f, "dark", 4.0f, "shader/shader.vs", "shader/shader.fs"); item_list.push_back(&monMenu);
    Menu monMenu2(window, "right", "top", 100.0f, 1000.0f, "dark", 4.0f, "shader/shader.vs", "shader/shader.fs"); item_list.push_back(&monMenu2);

    CallBack callBackManager(window, item_list);
    
    monMenu.loadVertices();
    monMenu2.loadVertices();
    monMenu.renderSetUp();
    monMenu2.renderSetUp();


    // std::vector<float> vertices;
    // for(auto&p:points){
    //     vertices.push_back(p.x / 1024.0f);
    //     vertices.push_back(p.y / 1024.0f);
    // }
    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);
    // glDrawArrays(GL_LINE_LOOP, 0, vertices.size()/2);

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
        glClearColor(0.1f, 0.12f, 0.18f, 1.0f);
        glClearStencil(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Matrices
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        monMenu.renderItem(poly);
        monMenu2.renderItem(poly);

        
        // glEnable(GL_STENCIL_TEST);

        // // étape 1 — inversion du stencil pour chaque contour
        // glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        // glDepthMask(GL_FALSE);
        // glStencilFunc(GL_ALWAYS, 0, 1);
        // glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);

        // glBindVertexArray(VAO);
        // glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        // // // étape 2 — dessiner uniquement les zones impaires
        // // glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        // // glDepthMask(GL_TRUE);
        // // glStencilFunc(GL_NOTEQUAL, 0, 1);
        // // glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        //glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        // glDisable(GL_STENCIL_TEST);

            glEnable(GL_STENCIL_TEST);

            // 🟣 Étape 1 — écriture du stencil (toggle bit à chaque passage)
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glDepthMask(GL_FALSE);
            glStencilFunc(GL_ALWAYS, 0, 1);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);

            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, vertices.size());

            // 🟢 Étape 2 — affichage final uniquement sur les pixels où stencil ≠ 0
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glDepthMask(GL_TRUE);
            glStencilFunc(GL_NOTEQUAL, 0, 1);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

            glDrawArrays(GL_TRIANGLES, 0, vertices.size());
            glDisable(GL_STENCIL_TEST);

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



