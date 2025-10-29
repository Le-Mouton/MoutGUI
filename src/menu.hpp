#pragma once
#include <vector>
#include <iostream>
#include <string>
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.hpp"
#include <thread>
#include <chrono>
#include <atomic>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.hpp"
#include "item.hpp"
#include "ttf-reader.hpp"

class Menu : public Item {
protected:
    std::string x_pos, y_pos;
    float prop_width, prop_height;
    float cornerRadius;
    int cornerResolution;
    std::string color;
    
    bool isDragging = false;
    bool isResizing = false;
    
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    
    // Zone de resize dans le coin bas-droit
    float resizeHandleSize = 20.0f; // en pixels
    
    std::vector<Item*> comp;
    float width, height;
    
    // Limites min/max
    float minWidth = 100.0f;
    float minHeight = 100.0f;

    // Curseurs GLFW
    GLFWcursor* resizeCursor = nullptr;
    GLFWcursor* normalCursor = nullptr;
    bool cursorChanged = false;

public:
    Menu(GLFWwindow* _window, const std::string & _x, const std::string & _y, float _width, float _height, 
         const std::string &_color, float _cornerRadius, const char* _vertexPath, const char* _fragmentPath, 
         std::vector<Item*> _comp) 
    : Item(_window, _vertexPath, _fragmentPath), x_pos(_x), y_pos(_y), color(_color), 
      prop_width(_width), prop_height(_height), cornerRadius(_cornerRadius), comp(_comp) {
        
        cornerResolution = 10;
        priority = 20;
        
        // Créer les curseurs
        resizeCursor = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
        normalCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        
        width  = prop_width  * 2.0f / w;
        for(int i = 0; i < comp.size(); i++){
            width = std::max(width, comp[i]->global_width + 0.04f); // Padding
        }
        height = prop_height * 2.0f / h;

        if (x_pos == "left")
            x = -1.0f + width/2 + cornerRadius * 2.0f / w;
        else if (x_pos == "right")
            x = 1.0f - width/2 - cornerRadius * 2.0f / w;
        else
            x = 0.0f;

        if (y_pos == "top")
            y = 1.0f - height/2 - cornerRadius * 2.0f / h;
        else if (y_pos == "bottom")
            y = -1.0f + height/2 + cornerRadius * 2.0f / h;
        else
            y = 0.0f;

        global_width = width; 
        global_height = height;
    }

    ~Menu() {
        if (resizeCursor) glfwDestroyCursor(resizeCursor);
        if (normalCursor) glfwDestroyCursor(normalCursor);
    }

    void loadVertices() override {
        vertices.clear();
        Vertex v;
        v.alpha = 1.0f;

        if(color == "red")
            v.r = 0.4f; v.g = 0.01f; v.b = 0.01f;
        if(color == "blue")
            v.r = 0.0f; v.g = 0.0f; v.b = 1.0f;
        if(color == "green")
            v.r = 0.0f; v.g = 1.0f; v.b = 0.0f;
        if(color == "black")
            v.r = 0.0f; v.g = 0.0f; v.b = 0.0f;
        if(color == "white")
            v.r = 1.0f; v.g = 1.0f; v.b = 1.0f;
        if(color == "dark")
            v.r = 0.20f; v.g = 0.2f; v.b = 0.2f;

        if(cornerResolution != 0){
            // top-right corner
            for(int i = cornerResolution; i >= 0; i--){
                float theta = M_PI * i/((float)cornerResolution * 2);
                v.x = (cornerRadius * cos(theta))* 2.0f / w + x + width/2;
                v.y = (cornerRadius * sin(theta))* 2.0f / h + y + height/2;
                vertices.push_back(v);
                v.x = x + width/2;
                v.y = y + height/2;
                vertices.push_back(v);
            }
            // top-left corner
            for(int i = 0; i <= cornerResolution; i++){
                float theta = - M_PI * i/((float)cornerResolution *2);
                v.x = (cornerRadius * cos(theta))* 2.0f / w + x + width/2;
                v.y = (cornerRadius * sin(theta))* 2.0f / h + y - height/2;
                vertices.push_back(v);
                v.x = x + width/2;
                v.y = y - height/2;
                vertices.push_back(v);
            }
            // bottom-left corner
            for(int i = cornerResolution; i >= 0; i--){
                float theta = M_PI * i/((float)cornerResolution * 2) + M_PI;
                v.x = (cornerRadius * cos(theta))* 2.0f / w + x - width/2;
                v.y = (cornerRadius * sin(theta))* 2.0f / h + y - height/2;
                vertices.push_back(v);
                v.x = x - width/2;
                v.y = y - height/2;
                vertices.push_back(v);
            }
            // bottom-right corner
            for(int i = 0; i <= cornerResolution; i++){
                float theta = - M_PI * i/((float)cornerResolution *2) + M_PI;
                v.x = (cornerRadius * cos(theta))* 2.0f / w + x - width/2;
                v.y = (cornerRadius * sin(theta))* 2.0f / h + y + height/2;
                vertices.push_back(v);
                v.x = x - width/2;
                v.y = y + height/2;
                vertices.push_back(v);
            }

            v.x = (cornerRadius * cos(M_PI/2)) * 2.0f / w + x + width/2;
            v.y = (cornerRadius * sin(M_PI/2)) * 2.0f / h + y + height/2;
            vertices.push_back(v);
            v.x = x + width/2;
            v.y = y + height/2;
            vertices.push_back(v);
        } else {
            v.x = x - width/2+ (cornerRadius) * 2.0f / w;
            v.y = y - height/2 - (cornerRadius) * 2.0f / h;
            vertices.push_back(v);
            v.x = x + width/2+ (cornerRadius) * 2.0f / w;
            v.y = y - height/2 - (cornerRadius) * 2.0f / h;
            vertices.push_back(v);
            v.x = x + width/2+ (cornerRadius) * 2.0f / w;
            v.y = y + height/2 + (cornerRadius) * 2.0f / h;
            vertices.push_back(v);
            v.x = x + width/2+ (cornerRadius) * 2.0f / w;
            v.y = y + height/2 + (cornerRadius) * 2.0f / h;
            vertices.push_back(v);
            v.x = x - width/2 - (cornerRadius) * 2.0f / w;
            v.y = y + height/2 + (cornerRadius) * 2.0f / h;
            vertices.push_back(v);
            v.x = x - width/2 - (cornerRadius) * 2.0f / w;
            v.y = y - height/2 - (cornerRadius) * 2.0f / h;
            vertices.push_back(v);
        }

        if(color == "dark")
            v.r = 0.3f; v.g = 0.3f; v.b = 0.3f;

        // Fond intérieur
        v.x = x + width/2;
        v.y = y + height/2;
        vertices.push_back(v);
        v.x = x + width/2;
        v.y = y - height/2;
        vertices.push_back(v);
        v.x = x - width/2;
        v.y = y - height/2;
        vertices.push_back(v);
        v.x = x - width/2;
        v.y = y - height/2;
        vertices.push_back(v);
        v.x = x - width/2;
        v.y = y + height/2;
        vertices.push_back(v);
        v.x = x + width/2;
        v.y = y + height/2;
        vertices.push_back(v);

        v.r = 0.6f; v.g = 0.6f; v.b = 0.6f; v.alpha = 1.0f;
        
        float handleSize = resizeHandleSize * 2.0f / w;
        float handleX = x + width/2 - handleSize;
        float handleY = y - height/2 + handleSize;
        
        float offset = handleSize * 0.1f;
        
        v.x = handleX + handleSize;
        v.y = handleY;
        vertices.push_back(v);
        v.x = handleX + handleSize;
        v.y = handleY - handleSize;
        vertices.push_back(v);
        v.x = handleX + handleSize;
        v.y = handleY - handleSize;
        vertices.push_back(v);
        v.x = handleX;
        v.y = handleY - handleSize;
        vertices.push_back(v);
        v.x = handleX + offset;
        v.y = handleY - handleSize;
        vertices.push_back(v);
        v.x = handleX + handleSize;
        v.y = handleY - offset;
        vertices.push_back(v);

        float currentY = y + height/2 - 0.03f;
        
        for(int i = 0; i < comp.size(); i++){
            comp[i]->xpos = x - width/2 + comp[i]->global_width/2 + 0.02f;
            comp[i]->ypos = currentY - comp[i]->global_height/2;
            
            currentY -= comp[i]->global_height + 0.015f;
            
            comp[i]->loadVertices();
        }
        
        global_width = width;
        global_height = height;
    }

    void renderSetUp() override {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        glBindVertexArray(0);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        
        for(int i = 0; i < comp.size(); i++){
            comp[i]->renderSetUp();
        }
    }

    void renderItem(bool poly) override {
        if(poly){
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);

        shader_item.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.size() - 6);
        
        glLineWidth(5.0f);
        glDrawArrays(GL_LINES, vertices.size() - 6, 6);
        glLineWidth(1.0f);

        for(int i = 0; i < comp.size(); i++){
            comp[i]->renderItem(poly);
        }
    }

    bool isInResizeHandle(double xpos, double ypos) {
        float ndc_x = (xpos / w) * 4.0f - 1.0f;
        float ndc_y = 1.0f - (ypos / h) * 4.0f;
        
        float handleSize = resizeHandleSize * 2.0f / w;
        float handleX = x + width/2 - handleSize;
        float handleY = y - height/2;
        
        return (ndc_x >= handleX && ndc_x <= handleX + handleSize &&
                ndc_y >= handleY && ndc_y <= handleY + handleSize);
    }

    bool mouse_button_callback(GLFWwindow* window, int button, int action, int mods) override {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            float ndc_x = (xpos / w) * 4.0f - 1.0f;
            float ndc_y = 1.0f - (ypos / h) * 4.0f;

            if (isInResizeHandle(xpos, ypos)) {
                isResizing = true;
                lastMouseX = xpos;
                lastMouseY = ypos;
                return true;
            }

            for (auto* child : comp) {
                if (child && child->contains(xpos, ypos)) {
                    return false; // Laisser l'enfant gérer
                }
            }

            if (ndc_x <= (x + width / 2) && ndc_x >= (x - width / 2) &&
                ndc_y <= (y + height / 2) && ndc_y >= (y - height / 2)) {

                isDragging = true;
                lastMouseX = xpos;
                lastMouseY = ypos;
                return true;
            }
        }
        else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            if (isDragging || isResizing) {
                isDragging = false;
                isResizing = false;
                return true;
            }
        }
        
        return false;
    }

    bool cursor_position_callback(GLFWwindow* window, double xpos, double ypos) override {
        if (!isResizing && !isDragging && isInResizeHandle(xpos, ypos)) {
            if (!cursorChanged) {
                glfwSetCursor(window, resizeCursor);
                cursorChanged = true;
            }
        } else if (cursorChanged && !isResizing) {
            glfwSetCursor(window, normalCursor);
            cursorChanged = false;
        }

        if (isResizing) {
            double dx = xpos - lastMouseX;
            double dy = ypos - lastMouseY;

            lastMouseX = xpos;
            lastMouseY = ypos;

            float ndc_dx = 4.0f * dx / w;
            float ndc_dy = -4.0f * dy / h;

            float newWidth = width + ndc_dx;
            float newHeight = height - ndc_dy;

            float minW = minWidth * 2.0f / w;
            float minH = minHeight * 2.0f / h;
            
            if (newWidth >= minW) {
                width = newWidth;
                prop_width = width * w / 2.0f;
            }
            
            if (newHeight >= minH) {
                height = newHeight;
                prop_height = height * h / 2.0f;
            }

            x += ndc_dx / 2.0f;
            y -= ndc_dy / 2.0f;

            loadVertices();
            return true;
        }

        if (isDragging) {
            double dx = xpos - lastMouseX;
            double dy = ypos - lastMouseY;

            lastMouseX = xpos;
            lastMouseY = ypos;

            float ndc_dx = 4.0f * dx / w;
            float ndc_dy = -4.0f * dy / h;

            x += ndc_dx;
            y += ndc_dy;

            x = std::max(-1.0f + width/2 + cornerRadius * 2.0f / w, 
                         std::min(1.0f - width/2 - cornerRadius * 2.0f / w, x));
            y = std::max(-1.0f + height/2 + cornerRadius * 2.0f / h, 
                         std::min(1.0f - height/2 - cornerRadius * 2.0f / h, y));

            loadVertices();
            return true;
        }

        return false;
    }

    bool contains(double xpos, double ypos) override {
        float ndc_x = (xpos / w) * 4.0f - 1.0f;
        float ndc_y = 1.0f - (ypos / h) * 4.0f;
        
        return (ndc_x >= x - width/2 && ndc_x <= x + width/2 &&
                ndc_y >= y - height/2 && ndc_y <= y + height/2);
    }
};