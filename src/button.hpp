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
#include "menu.hpp"

class Button : public Item{
protected:
	std::string text;
	float width, height;

	float cornerRadius;
	float cornerResolution = 10;

	std::string color;
	Text text_button;

	float scale;
	bool isDragging = false;
	bool isDoubleClicked = false;
	bool isClicked = false;

	double dragOffsetX = 0.0;
	double dragOffsetY = 0.0;

	double lastMouseX = 0.0;
	double lastMouseY = 0.0;

	double lastClickTime = 0.0;

	double animationTime = 0.0f;
	bool endAnimation = false;

	float prop_width = 40.0f; 
	float prop_height = 60.0f;
	float text_width;

	float delta_width, delta_height;
	float newdelta_width, newdelta_height;

	bool &switched_value;

public:

	Button(GLFWwindow* _window, std::string _text, float _x, float _y, float _scale, const std::string &_color, float _cornerRadius, const char* vertexPath, const char* fragmentPath, bool &_switched_value) : 
	Item(_window, vertexPath, fragmentPath), scale(_scale), color(_color), cornerRadius(_cornerRadius), text(_text), text_button(_window, _x, (_y + 100.0f * 1 / h), _scale * 0.5f, vertexPath, fragmentPath, _text), switched_value(_switched_value){
		x = _x; y = _y;

		text_button.loadVertices();

		text_width = - x + text_button.max_x + scale * 0.4f;

		width  = (prop_width  * 2.0f / w) * scale + text_width;
		height = prop_height * 2.0f / h;

		global_width = width ; global_height = height;

		
	}

		void loadVertices() override {

			x = xpos; y = ypos;

			vertices.clear();

			Vertex v;

			if(color == "dark"){
				v.r = 0.2f; v.g = 0.2f; v.b = 0.2f; v.alpha = 1.0f;
			}

			if(color == "black_dark"){
				v.r = 0.7f; v.g = 0.1f; v.b = 0.1f; v.alpha = 0.5f;
			}

			if(cornerResolution != 0){
				// top-right corner
				// right to left
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
				// right to left
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
				// right to left
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
				// right to left
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

			if(color == "dark"){
				v.r = 0.3f; v.g = 0.3f; v.b = 0.3f; v.alpha = 1.0f;
			}

			if(color == "black_dark"){
				v.r = 0.2f; v.g = 0.2f; v.b = 0.2f; v.alpha = 1.0f;
			}

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
		}

		void renderSetUp() override {

			text_button.renderSetUp();

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
	        glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.size());

	       	text_button.xpos = x - width/2.0f + scale * 0.2f;
			text_button.ypos = y - scale * 0.4f;

			text_button.loadVertices();
			text_button.renderSetUp();
	        text_button.renderItem(poly);
				
		}

		bool mouse_button_callback(GLFWwindow* window, int button, int action, int mods) override {
		    double xpos, ypos;
		    glfwGetCursorPos(window, &xpos, &ypos);

		    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		    {

		    	double now = glfwGetTime();
		        if (now - lastClickTime < 0.25) { // 250 ms
		            isDoubleClicked = !isDoubleClicked;
		        }
		        lastClickTime = now;
		    	if((((xpos / w) * 4.0f - 1.0f) <= (x + width / 2) && ((xpos / w) * 4.0f - 1.0f) >= (x - width / 2))){
		    		if((1.0f - (ypos / h) * 4.0f) <= (y + height / 2) && (1.0f - (ypos / h) * 4.0f) >= (y - height / 2)){

				        isDragging = true;
				        lastMouseX = xpos;
				        lastMouseY = ypos;
				        isClicked = true;
				        if(isClicked){
				        	isClicked = !isClicked;
				        	color = "black_dark";
				        	loadVertices();
				        	switched_value = !switched_value;
				        	return true; 
				        }

		    		} else isDoubleClicked = false; isClicked= false; color = "dark"; loadVertices(); return false;
		    	} else isDoubleClicked = false; isClicked=false; color = "dark"; loadVertices(); return false;
		    }
		    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		    {
		        isDragging = false; return false;
		    }
		    return false;
		}

		bool cursor_position_callback(GLFWwindow* window, double xpos, double ypos) override {
		    if (!isDragging) return false;
		    double dx = xpos - lastMouseX;
		    double dy = ypos - lastMouseY;

		    lastMouseX = xpos;
		    lastMouseY = ypos;

		    float ndc_dx =  4.0f * dx / w;
		    float ndc_dy = -4.0f * dy / h;

		    x += ndc_dx;
		    y += ndc_dy;

		    x = std::max(-1.0f + width/2 + cornerRadius * 2.0f / w, std::min(1.0f - width/2 - cornerRadius * 2.0f / w, x));
		    y = std::max(-1.0f + height/2 + cornerRadius * 2.0f / h, std::min(1.0f - height/2 - cornerRadius * 2.0f / h, y));

		    loadVertices();
		    return true;
		}
		bool contains(double xpos, double ypos) override {
	        float ndc_x = (xpos / w) * 4.0f - 1.0f;
	        float ndc_y = 1.0f - (ypos / h) * 4.0f;
	        
	        // Zone élargie pour faciliter la sélection
	        return (ndc_x >= x - width/2 * scale && ndc_x <= x + width/2 * scale &&
	                ndc_y >= y - height/2 * scale && ndc_y <= y + height/2 * scale);
    	}

};