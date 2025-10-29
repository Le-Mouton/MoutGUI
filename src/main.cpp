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
#include "scatter.hpp"
#include "plot.hpp"
#include "lineplot.hpp"
#include "heatmap.hpp"

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

double frequency = 2.0;
double amplitude = 3.0;
double phase = 0.0;

bool poly = false;
bool paused = false;

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

    // ========== PLOT 1: Signal temporel (scrolling) ==========
    LinePlot signalPlot(window, -0.5f, 0.5f, 450.0f, 280.0f,
                        "shader/shader.vs", "shader/shader.fs");
    signalPlot.setLineWidth(2.5f);
    signalPlot.setLineColor(0.2f, 0.6f, 0.9f);
    signalPlot.title->text = "Signal: sin(freq*t + phase)";
    signalPlot.xLabel->text = "Time";
    signalPlot.yLabel->text = "Amplitude";
    signalPlot.setLimits(0, 10, -5, 5);
    
    std::deque<DataPoint> signalHistory;

    signalPlot.addLineSeries(signalHistory);

    const int maxHistoryPoints = 200;

    // ========== PLOT 2: Scatter avec fit ==========
    Scatter scatterFit(window, 0.5f, 0.5f, 450.0f, 280.0f,
                            "shader/shader.vs", "shader/shader.fs");
    scatterFit.title->text = "Data + Regression";
    scatterFit.xLabel->text = "X";
    scatterFit.yLabel->text = "Y";

    HeatmapPlot heatmapPlot(window, -0.5f, -0.35f, 450, 280, "shader/shader.vs", "shader/shader.fs");
    heatmapPlot.setGridSize(40, 40);
    heatmapPlot.setColormap(HeatmapPlot::VIRIDIS);
    heatmapPlot.title->text = "Heatmap";
    heatmapPlot.loadVertices();
    heatmapPlot.renderSetUp();

    // Générer des données avec du bruit
    std::vector<DataPoint> scatterData;
    for (float x = 0; x < 10; x += 0.2f) {
        DataPoint pt;
        pt.x = x;
        pt.y = 2.0f * x + 1.0f + ((rand() % 100) / 50.0f - 1.0f) * 2.0f;
        pt.r = 0.3f; pt.g = 0.7f; pt.b = 0.3f;
        scatterData.push_back(pt);
    }
    scatterFit.setScatterData(scatterData);

    // Ligne de régression
    std::vector<DataPoint> fitLine;
    for (float x = 0; x < 10; x += 0.5f) {
        DataPoint pt;
        pt.x = x;
        pt.y = 2.0f * x + 1.0f; // Ligne théorique
        fitLine.push_back(pt);
    }
    scatterFit.addLineSeries(fitLine, 3.0f, glm::vec3(0.9f, 0.3f, 0.2f));

    std::deque<std::vector<float>> spectrogramData;
    const int W = 100, H = 50;

    HeatmapPlot spectrogram(window, 0.5f, -0.35f, 450, 280, "shader/shader.vs", "shader/shader.fs");
    spectrogram.setGridSize(H, W);
    spectrogram.setColormap(HeatmapPlot::HOT);
    spectrogram.title->text = "Spectrogram";
    spectrogram.loadVertices();
    spectrogram.renderSetUp();

    // ========== MENU DE CONTRÔLE ==========
    Slider freqSlider(window, 0.5f, 0.7f, 250.0f, 30.0f, 1.f, 8.0f,
                      "shader/shader.vs", "shader/shader.fs", frequency);
    freqSlider.priority = 100;
    
    Slider ampSlider(window, 0.5f, 0.5f, 250.0f, 30.0f, 1.f, 8.0f,
                     "shader/shader.vs", "shader/shader.fs", amplitude);
    ampSlider.priority = 100;
    
    Slider phaseSlider(window, 0.5f, 0.3f, 250.0f, 30.0f, 1.f, 8.0f,
                       "shader/shader.vs", "shader/shader.fs", phase);
    phaseSlider.priority = 100;

    Button pauseBtn(window, "Pause", 0.0f, 0.0f, 0.04f, "dark", 4.0f,
                    "shader/shader.vs", "shader/shader.fs", paused);
    pauseBtn.priority = 100;

    std::vector<Item*> menu_content = {&freqSlider, &ampSlider, &phaseSlider, &pauseBtn};
    Menu controlMenu(window, "right", "top", 180.0f, 500.0f, "dark", 4.0f,
                     "shader/shader.vs", "shader/shader.fs", menu_content);
    controlMenu.priority = 10;

    // Liste complète des items
    std::vector<Item*> item_list = {
        &freqSlider, &ampSlider, &phaseSlider, &pauseBtn,
        &controlMenu,
        &signalPlot, 
        &scatterFit , &heatmapPlot, &spectrogram
    };

    CallBack callBackManager(window, item_list);

    // Setup initial
    signalPlot.loadVertices();
    signalPlot.renderSetUp();
    
    scatterFit.loadVertices();
    scatterFit.renderSetUp();
    
    heatmapPlot.loadVertices();
    heatmapPlot.renderSetUp();
    
    spectrogram.loadVertices();
    spectrogram.renderSetUp();
    
    controlMenu.loadVertices();
    controlMenu.renderSetUp();

    glPointSize(8);

    float time = 0.0f;


    // --- Thread Render ---
    while (!glfwWindowShouldClose(window)) {

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.03f, 0.03f, 0.1f, 1.0f);
        glClearStencil(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Matrices
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // Mise à jour du signal temporel
        if (!paused) {
            DataPoint newPoint;
            newPoint.x = time;
            newPoint.y = amplitude * sin(frequency*50 * time + phase*50);
            signalHistory.push_back(newPoint);
            
            if (signalHistory.size() > maxHistoryPoints) {
                signalHistory.pop_front();
            }
            
            // après avoir poussé newPoint dans signalHistory…
            signalPlot.setSeriesData(0, signalHistory);

            // ajuste les limites pour “scroller”
            if (!signalHistory.empty()) {
                float xMin = signalHistory.front().x;
                float xMax = signalHistory.back().x;
                // garde une marge si tu veux (ex: +5%)
                signalPlot.setLimits(xMin, xMax, -5.0f, 5.0f);
            }            
            signalPlot.loadVertices();

                    std::vector<float> newColumn(H);
            for(int i=0;i<H;i++)
                newColumn[i] = exp(-pow(i*0.1 - frequency*50,2));

            spectrogram.pushColumn(newColumn);
            spectrogram.loadVertices();

            std::vector<std::vector<float>> H(40, std::vector<float>(40));
            for (int i=0;i<40;i++)
            for (int j=0;j<40;j++)
                H[i][j] = sin(sqrt(i*i+j*j)*0.15 - time);

            heatmapPlot.setGridData(H);
            heatmapPlot.loadVertices();
            time += deltaTime;
        }

        

        // Rendu
        signalPlot.renderItem(poly);
        scatterFit.renderItem(poly);
        heatmapPlot.renderItem(poly);
        spectrogram.renderItem(poly);
        controlMenu.renderItem(poly);

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
    static bool pPressed = false;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pPressed) {
        paused = !paused;
        pPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        pPressed = false;
        //std::cout << "not pause" << std::endl;

    }

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



