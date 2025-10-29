#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "item.hpp"
#include "text.hpp"

struct DataPoint {
    float x, y;
    float r = 1.0f, g = 1.0f, b = 1.0f;
};

struct LineSeries {
    std::vector<Vertex> vertices;
    GLuint VAO, VBO;
    float lineWidth;
    glm::vec3 color;
    bool initialized = false;
};

class Plot : public Item {
public:

	bool isDragging = false;
	bool isResizing = false;

	double lastMouseX = 0.0;
	double lastMouseY = 0.0;

	float resizeHandleSize = 20.0f; // en pixels

	GLFWcursor* resizeCursor = nullptr;
	GLFWcursor* normalCursor = nullptr;
	bool cursorChanged = false;

	float minWidth = 120.0f;
	float minHeight = 120.0f;

    float plot_width, plot_height;
    float padding = 40.0f;
    
    float xMin, xMax, yMin, yMax;
    bool autoScale = true;
    
    std::string backgroundColor = "white";
    float gridAlpha = 0.2f;
    bool showGrid = true;
    bool showAxes = true;
    
    Text* xLabel = nullptr;
    Text* yLabel = nullptr;
    Text* title = nullptr;

    std::vector<Text*> xTicksLabels;
    std::vector<Text*> yTicksLabels;

    int numTicksX = 6;
    int numTicksY = 6;
    
    std::vector<DataPoint> dataPoints;
    
    std::vector<Vertex> backgroundVertices;
    std::vector<Vertex> gridVertices;
    std::vector<Vertex> axesVertices;
    
    GLuint backgroundVAO, backgroundVBO;
    GLuint gridVAO, gridVBO;
    GLuint axesVAO, axesVBO;

    const char* shader_v;
    const char* shader_f;

    Plot(GLFWwindow* _window, float _x, float _y, float _width, float _height, 
         const char* vertexPath, const char* fragmentPath)
    : Item(_window, vertexPath, fragmentPath), shader_v(vertexPath), shader_f(fragmentPath){

        resizeCursor = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
        normalCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        
        x = _x;
        y = _y;
        plot_width = _width * 2.0f / w;
        plot_height = _height * 2.0f / h;
        
        global_width = plot_width;
        global_height = plot_height;
        
        priority = 10;
        
        xMin = 0.0f; xMax = 10.0f;
        yMin = 0.0f; yMax = 10.0f;

        shader_v = vertexPath;
        shader_f = fragmentPath;
        
        title = new Text(_window, x, y + plot_height/2 + 0.03f, 0.015f, 
                        vertexPath, fragmentPath, "Plot");
        xLabel = new Text(_window, x, y - plot_height/2 - 0.05f, 0.012f, 
                         vertexPath, fragmentPath, "X Axis");
        yLabel = new Text(_window, x - plot_width/2 - 0.08f, y, 0.012f, 
                         vertexPath, fragmentPath, "Y Axis");
        title->precision = 2; xLabel->precision = 2; yLabel->precision = 2;
    }
    
    virtual ~Plot() {
        if (title) delete title;
        if (xLabel) delete xLabel;
        if (yLabel) delete yLabel;
        
        glDeleteVertexArrays(1, &backgroundVAO);
        glDeleteBuffers(1, &backgroundVBO);
        glDeleteVertexArrays(1, &gridVAO);
        glDeleteBuffers(1, &gridVBO);
        glDeleteVertexArrays(1, &axesVAO);
        glDeleteBuffers(1, &axesVBO);
    }

    static float niceRound(float value) {
        float expv = powf(10.0f, floorf(log10f(value)));
        float f = value / expv;
        float nf;
        if (f < 1.5f) nf = 1.0f;
        else if (f < 3.0f) nf = 2.0f;
        else if (f < 7.0f) nf = 5.0f;
        else nf = 10.0f;
        return nf * expv;
    }
    
    void setData(const std::vector<DataPoint>& data) {
        dataPoints = data;
        if (autoScale) {
            computeDataBounds();
        }
    }
    
    void addPoint(float x, float y, float r = 1.0f, float g = 1.0f, float b = 1.0f) {
        DataPoint pt;
        pt.x = x; pt.y = y;
        pt.r = r; pt.g = g; pt.b = b;
        dataPoints.push_back(pt);
        
        if (autoScale) {
            computeDataBounds();
        }
    }
    
    void clearData() {
        dataPoints.clear();
    }
    
    // Définir les limites 
    void setLimits(float xmin, float xmax, float ymin, float ymax) {
        xMin = xmin; xMax = xmax;
        yMin = ymin; yMax = ymax;
        autoScale = false;
    }
    
    virtual void computeDataBounds() {
        if (dataPoints.empty()) return;
        
        xMin = xMax = dataPoints[0].x;
        yMin = yMax = dataPoints[0].y;
        
        for (const auto& pt : dataPoints) {
            xMin = std::min(xMin, pt.x);
            xMax = std::max(xMax, pt.x);
            yMin = std::min(yMin, pt.y);
            yMax = std::max(yMax, pt.y);
        }
        
        float xMargin = (xMax - xMin) * 0.05f;
        float yMargin = (yMax - yMin) * 0.05f;
        xMin -= xMargin; xMax += xMargin;
        yMin -= yMargin; yMax += yMargin;
    }
    
    glm::vec2 dataToNDC(float dataX, float dataY) {
        float paddingNDC_x = padding * 2.0f / w;
        float paddingNDC_y = padding * 2.0f / h;
        
        float plotAreaWidth = plot_width - 2 * paddingNDC_x;
        float plotAreaHeight = plot_height - 2 * paddingNDC_y;
        
		float dx = xMax - xMin;
		float dy = yMax - yMin;
		if (dx == 0.0f) dx = 1e-6f;
		if (dy == 0.0f) dy = 1e-6f;

		float normalizedX = (dataX - xMin) / dx;
		float normalizedY = (dataY - yMin) / dy;
        
        float ndcX = x - plot_width/2 + paddingNDC_x + normalizedX * plotAreaWidth;
        float ndcY = y - plot_height/2 + paddingNDC_y + normalizedY * plotAreaHeight;
        
        return glm::vec2(ndcX, ndcY);
    }
    
    void loadBackgroundVertices() {
        backgroundVertices.clear();
        Vertex v;
        v.alpha = 1.0f;
        
        if (backgroundColor == "white") {
            v.r = 1.0f; v.g = 1.0f; v.b = 1.0f;
        } else if (backgroundColor == "light_gray") {
            v.r = 0.95f; v.g = 0.95f; v.b = 0.95f;
        } else if (backgroundColor == "dark") {
            v.r = 0.15f; v.g = 0.15f; v.b = 0.15f;
        }
        
        v.x = x - plot_width/2;
        v.y = y - plot_height/2;
        backgroundVertices.push_back(v);
        v.x = x + plot_width/2;
        v.y = y - plot_height/2;
        backgroundVertices.push_back(v);
        v.x = x + plot_width/2;
        v.y = y + plot_height/2;
        backgroundVertices.push_back(v);
        v.x = x + plot_width/2;
        v.y = y + plot_height/2;
        backgroundVertices.push_back(v);
        v.x = x - plot_width/2;
        v.y = y + plot_height/2;
        backgroundVertices.push_back(v);
        v.x = x - plot_width/2;
        v.y = y - plot_height/2;
        backgroundVertices.push_back(v);
    }
    
    void loadGridVertices() {
        gridVertices.clear();
        if (!showGrid) return;
        
        Vertex v;
        v.r = 0.7f; v.g = 0.7f; v.b = 0.7f;
        v.alpha = gridAlpha;
        
        float paddingNDC_x = padding * 2.0f / w;
        float paddingNDC_y = padding * 2.0f / h;
        
        float plotLeft = x - plot_width/2 + paddingNDC_x;
        float plotRight = x + plot_width/2 - paddingNDC_x;
        float plotBottom = y - plot_height/2 + paddingNDC_y;
        float plotTop = y + plot_height/2 - paddingNDC_y;
        
        int numVerticalLines = 10;
        for (int i = 0; i <= numVerticalLines; i++) {
            float t = i / (float)numVerticalLines;
            float xPos = plotLeft + t * (plotRight - plotLeft);
            
            v.x = xPos; v.y = plotBottom;
            gridVertices.push_back(v);
            v.x = xPos; v.y = plotTop;
            gridVertices.push_back(v);
        }
        
        int numHorizontalLines = 10;
        for (int i = 0; i <= numHorizontalLines; i++) {
            float t = i / (float)numHorizontalLines;
            float yPos = plotBottom + t * (plotTop - plotBottom);
            
            v.x = plotLeft; v.y = yPos;
            gridVertices.push_back(v);
            v.x = plotRight; v.y = yPos;
            gridVertices.push_back(v);
        }

		v.r = 1.0f; v.g = 0.0f; v.b = 0.0f; v.alpha = 1.0f;

		float hsx = resizeHandleSize * 2.0f / w;
		float hsy = resizeHandleSize * 2.0f / h;

		float handleX = x + plot_width/2 - hsx;
		float handleY = y - plot_height/2 + hsy;

		v.x = handleX;       v.y = handleY;       axesVertices.push_back(v);
		v.x = handleX+hsx;   v.y = handleY;       axesVertices.push_back(v);
		v.x = handleX+hsx;   v.y = handleY-hsy;   axesVertices.push_back(v);
		v.x = handleX;       v.y = handleY;       axesVertices.push_back(v);
		v.x = handleX+hsx;   v.y = handleY-hsy;   axesVertices.push_back(v);
		v.x = handleX;       v.y = handleY-hsy;   axesVertices.push_back(v);
    }
    
    void loadAxesVertices() {
        axesVertices.clear();
        for (auto* t : xTicksLabels) delete t;
        for (auto* t : yTicksLabels) delete t;
        xTicksLabels.clear();
        yTicksLabels.clear();



        if (!showAxes) return;

        float paddingNDC_x = padding * 2.0f / w;
        float paddingNDC_y = padding * 2.0f / h;

        float plotLeft   = x - plot_width/2 + paddingNDC_x;
        float plotRight  = x + plot_width/2 - paddingNDC_x;
        float plotBottom = y - plot_height/2 + paddingNDC_y;
        float plotTop    = y + plot_height/2 - paddingNDC_y;

        Vertex v; 
        v.alpha = 1.0f; 
        v.r = 0.0f; v.g = 0.0f; v.b = 0.0f;

        auto pushLine = [&](float x1, float y1, float x2, float y2){
            v.x = x1; v.y = y1; axesVertices.push_back(v);
            v.x = x2; v.y = y2; axesVertices.push_back(v);
        };

        pushLine(plotLeft, plotBottom, plotRight, plotBottom);
        pushLine(plotRight, plotBottom, plotRight, plotTop);
        pushLine(plotRight, plotTop, plotLeft, plotTop);
        pushLine(plotLeft, plotTop, plotLeft, plotBottom);

        // === CALCUL DYNAMIQUE ====
        float dx = niceRound((xMax - xMin) / (numTicksX - 1));
        float dy = niceRound((yMax - yMin) / (numTicksY - 1));

        for (float xv = ceil(xMin / dx) * dx; xv <= xMax; xv += dx) {
            glm::vec2 p = dataToNDC(xv, yMin);

            pushLine(p.x, plotBottom, p.x, plotBottom + (paddingNDC_y * 0.4f));

            auto* t = new Text(window, p.x, plotBottom - paddingNDC_y * 0.6f, 0.012f, shader_v, shader_f, std::to_string((int)xv));
            t->precision = 1; t->color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            xTicksLabels.push_back(t);
            t->loadVertices();
            t->renderSetUp();
        }

        for (float yv = ceil(yMin / dy) * dy; yv <= yMax; yv += dy) {
            glm::vec2 p = dataToNDC(xMin, yv);

            pushLine(plotLeft, p.y, plotLeft + (paddingNDC_x * 0.4f), p.y);

            auto* t = new Text(window, plotLeft - paddingNDC_x * 0.8f, p.y, 0.012f, shader_v, shader_f, std::to_string((int)yv));
            yTicksLabels.push_back(t); t->color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            t->loadVertices();
            t->renderSetUp();
        }

        // === HANDLE (inchangé) ===
        float hsx = resizeHandleSize * 2.0f / w;
        float hsy = resizeHandleSize * 2.0f / h;
        float handleX = x + plot_width/2 - hsx;
        float handleY = y - plot_height/2 + hsy;

        pushLine(handleX, handleY, handleX + hsx, handleY);
        pushLine(handleX + hsx, handleY, handleX + hsx, handleY - hsy);
        pushLine(handleX + hsx, handleY - hsy, handleX, handleY - hsy);
        pushLine(handleX, handleY - hsy, handleX, handleY);
    }
        
    void loadVertices() override {
        loadBackgroundVertices();
        loadGridVertices();
        loadAxesVertices();
    }
    
    void renderSetUp() override {
        // Background
        glGenVertexArrays(1, &backgroundVAO);
        glGenBuffers(1, &backgroundVBO);
        glBindVertexArray(backgroundVAO);
        glBindBuffer(GL_ARRAY_BUFFER, backgroundVBO);
        glBufferData(GL_ARRAY_BUFFER, backgroundVertices.size() * sizeof(Vertex), 
                     backgroundVertices.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        
        // Grid
        glGenVertexArrays(1, &gridVAO);
        glGenBuffers(1, &gridVBO);
        glBindVertexArray(gridVAO);
        glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
        glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(Vertex), 
                     gridVertices.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        
        // Axes
        glGenVertexArrays(1, &axesVAO);
        glGenBuffers(1, &axesVBO);
        glBindVertexArray(axesVAO);
        glBindBuffer(GL_ARRAY_BUFFER, axesVBO);
        glBufferData(GL_ARRAY_BUFFER, axesVertices.size() * sizeof(Vertex), 
                     axesVertices.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        
        if (title) {
            title->loadVertices();
            title->renderSetUp();
        }
        if (xLabel) {
            xLabel->loadVertices();
            xLabel->renderSetUp();
        }
        if (yLabel) {
            yLabel->loadVertices();
            yLabel->renderSetUp();
        }
    }
    
	void renderItem(bool poly) override {
	    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	    shader_item.use();

	    // Background
	    glBindVertexArray(backgroundVAO);
	    glBindBuffer(GL_ARRAY_BUFFER, backgroundVBO);
	    glBufferData(GL_ARRAY_BUFFER, backgroundVertices.size()*sizeof(Vertex),
	                 backgroundVertices.data(), GL_DYNAMIC_DRAW);
	    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)backgroundVertices.size());

	    // Grid
	    if (showGrid && !gridVertices.empty()) {
	        glBindVertexArray(gridVAO);
	        glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
	        glBufferData(GL_ARRAY_BUFFER, gridVertices.size()*sizeof(Vertex),
	                     gridVertices.data(), GL_DYNAMIC_DRAW);
	        glLineWidth(1.0f);
	        glDrawArrays(GL_LINES, 0, (GLsizei)gridVertices.size());
	    }

	    // Axes + handle
	    if (showAxes && !axesVertices.empty()) {
	        glBindVertexArray(axesVAO);
	        glBindBuffer(GL_ARRAY_BUFFER, axesVBO);
	        glBufferData(GL_ARRAY_BUFFER, axesVertices.size()*sizeof(Vertex),
	                     axesVertices.data(), GL_DYNAMIC_DRAW);
	        glLineWidth(2.0f);
	        glDrawArrays(GL_LINES, 0, (GLsizei)axesVertices.size());
	    }

        for (auto* t : xTicksLabels) t->renderItem(false);
        for (auto* t : yTicksLabels) t->renderItem(false);

	    if (title) title->renderItem(false);
	    if (xLabel) xLabel->renderItem(false);
	    if (yLabel) yLabel->renderItem(false);
	}
    
	bool mouse_button_callback(GLFWwindow* window, int button, int action, int mods) override {



	    double xpos, ypos;
	    glfwGetCursorPos(window, &xpos, &ypos);

	    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

	        if (isInResizeHandle(xpos, ypos)) {
	            isResizing = true;
	            lastMouseX = xpos;
	            lastMouseY = ypos;
	            return true;
	        }

	        if (contains(xpos, ypos)) {
	            isDragging = true;
	            lastMouseX = xpos;
	            lastMouseY = ypos;
	            return true;
	        }

        }
	    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
	        isDragging = false;
	        isResizing = false;
	        if (title) { title->xpos = x; title->ypos = y + plot_height/2 + 0.03f; title->loadVertices(); title->renderSetUp();}
			if (xLabel){ xLabel->xpos = x; xLabel->ypos = y - plot_height/2 - 0.05f; xLabel->loadVertices(); xLabel->renderSetUp();}
			if (yLabel){ yLabel->xpos = x - plot_width/2 - 0.08f; yLabel->ypos = y; yLabel->loadVertices(); yLabel->renderSetUp();}
	        return true;

	    }

	    return false;
	}

    
	bool cursor_position_callback(GLFWwindow* window, double xpos, double ypos) override {

	    // Changement curseur
	    if (!isDragging && !isResizing && isInResizeHandle(xpos, ypos)) {
	        glfwSetCursor(window, resizeCursor);
	        cursorChanged = true;
	    } 
	    else if (cursorChanged && !isResizing) {
	        glfwSetCursor(window, normalCursor);
	        cursorChanged = false;
	       	if (title) { title->xpos = x; title->ypos = y + plot_height/2 + 0.03f; title->loadVertices(); title->renderSetUp();}
			if (xLabel){ xLabel->xpos = x; xLabel->ypos = y - plot_height/2 - 0.05f; xLabel->loadVertices(); xLabel->renderSetUp();}
			if (yLabel){ yLabel->xpos = x - plot_width/2 - 0.08f; yLabel->ypos = y; yLabel->loadVertices(); yLabel->renderSetUp();}
	    }

	    // RESIZE
	    if (isResizing) {
	        float dx = xpos - lastMouseX;
	        float dy = ypos - lastMouseY;

	        lastMouseX = xpos;
	        lastMouseY = ypos;

	        float ndc_dx = 2.0f * dx / w;
	        float ndc_dy = -2.0f * dy / h;

	        float newW = plot_width + ndc_dx;
	        float newH = plot_height - ndc_dy;

	        float minW = minWidth * 2.0f / w;
	        float minH = minHeight * 2.0f / h;

	        if (newW > minW)  plot_width = newW;
	        if (newH > minH)  plot_height = newH;

	        x += ndc_dx / 2.0f;
	        y -= ndc_dy / 2.0f;

	        loadVertices();
	        return true;
	    }

	    // DRAG
	    if (isDragging) {
	        float dx = xpos - lastMouseX;
	        float dy = ypos - lastMouseY;
	        lastMouseX = xpos;
	        lastMouseY = ypos;

	        float ndc_dx = 2.0f * dx / w;
	        float ndc_dy = -2.0f * dy / h;

	        x += ndc_dx;
	        y += ndc_dy;

	        loadVertices();
	        return true;
	    }

	    return false;
	}
    
	bool contains(double xpos, double ypos) override {
	    float ndc_x = (float(xpos) / w) * 4.0f - 1.0f;
	    float ndc_y = 1.0f - (float(ypos) / h) * 4.0f;
	    return (ndc_x >= x - plot_width/2 && ndc_x <= x + plot_width/2 &&
	            ndc_y >= y - plot_height/2 && ndc_y <= y + plot_height/2);
	}

    bool isInResizeHandle(double xpos, double ypos) {
	    float ndc_x = (float(xpos) / w) * 4.0f - 1.0f;
	    float ndc_y = 1.0f - (float(ypos) / h) * 4.0f;

	    float hsx = resizeHandleSize * 2.0f / w;
	    float hsy = resizeHandleSize * 2.0f / h;

	    float handleX = x + plot_width/2 - hsx;
	    float handleY = y - plot_height/2 + hsy;

	    return (ndc_x >= handleX && ndc_x <= handleX+hsx &&
	            ndc_y <= handleY && ndc_y >= handleY-hsy);
	}
};