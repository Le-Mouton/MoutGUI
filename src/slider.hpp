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

class Slider : public Item {
	protected :

		float thickness = 0.01f;

		bool isDragging = false;
		bool isVertical = false;

		double dragOffsetX = 0.0;
		double dragOffsetY = 0.0;

		double lastMouseX = 0.0;
		double lastMouseY = 0.0;

		double x_percent;

		float cornerResolution = 10.0f;
		float cornerRadius;

		float width, height;

		double &value_change;

		Text value_text;
		std::string text;


		
	public:

		double scale;

		float xpos, ypos;
		bool isClickOut = false;

		Slider(GLFWwindow* _window, float _x, float _y, float _width, float _height, float _scale, float _cornerRadius, const char* vertexPath, const char* fragmentPath, double &_value_change) 
		: Item(_window, vertexPath, fragmentPath), cornerRadius(_cornerRadius), width(_width), height(_height), scale(_scale), value_change(_value_change), value_text(_window, (_x + (_width * 1/w) * _scale), (_y-0.01f * _scale), 0.01f * _scale, vertexPath, fragmentPath, ""){

			width = width *2.0f/w;
			height = height * 2.0f/h;

			x = _x; y = _y;
			x_percent = (- width * 1/4 + thickness) * scale;

			text = "0.0";

			value_text.text = text;
			value_text.precision = 4;
			value_text.loadVertices();

			value_text.renderSetUp();

		}


		void loadVertices() override {

			vertices.clear();

			Vertex v;

			v.r = 0.2f; v.g = 0.2f; v.b = 0.2f; v.alpha = 1.0f;

			v.x = x + width*3/4 * scale;
			v.y = y + height/2 * scale;
			vertices.push_back(v);
			v.x = x + width*3/4 * scale;
			v.y = y - height/2 * scale;
			vertices.push_back(v);
			v.x = x - width/2 * scale;
			v.y = y - height/2 * scale;
			vertices.push_back(v);
			v.x = x - width/2 * scale;
			v.y = y - height/2 * scale;
			vertices.push_back(v);
			v.x = x - width/2 * scale;
			v.y = y + height/2 * scale;
			vertices.push_back(v);
			v.x = x + width*3/4 * scale;
			v.y = y + height/2 * scale;
			vertices.push_back(v);

			v.r = 0.3f; v.g = 0.3f; v.b = 0.3f; v.alpha = 1.0f;

			v.x = x - thickness * scale - width * 1/4 * scale - thickness/2 * scale;
			v.y = y - thickness * scale - thickness/2  * scale;
			vertices.push_back(v);
			v.x = x + thickness * scale + width * 1/4 * scale + thickness/2 * scale;
			v.y = y - thickness * scale - thickness/2 * scale;
			vertices.push_back(v);
			v.x = x + thickness * scale + width * 1/4 * scale + thickness/2 * scale;
			v.y = y + thickness * scale + thickness/2 * scale;
			vertices.push_back(v);
			v.x = x + thickness * scale + width * 1/4 * scale + thickness/2 * scale;
			v.y = y + thickness * scale + thickness/2 * scale;
			vertices.push_back(v);
			v.x = x - thickness * scale - width * 1/4 * scale - thickness/2 * scale;
			v.y = y + thickness * scale + thickness/2 * scale;
			vertices.push_back(v);
			v.x = x - thickness * scale - width * 1/4 * scale - thickness/2 * scale;
			v.y = y - thickness * scale - thickness/2 * scale;
			vertices.push_back(v);

			// Axe x (horizontal)
			v.r = 1.0f; v.g = 0.0f; v.b = 0.0f; v.alpha = 1.0f;

			v.x = x - thickness * scale + x_percent;
			v.y = y - thickness * scale;
			vertices.push_back(v);
			v.x = x + thickness * scale + x_percent;
			v.y = y - thickness * scale;
			vertices.push_back(v);
			v.x = x + thickness * scale + x_percent;
			v.y = y + thickness * scale;
			vertices.push_back(v);
			v.x = x + thickness * scale + x_percent;
			v.y = y + thickness * scale;
			vertices.push_back(v);
			v.x = x - thickness * scale + x_percent;
			v.y = y + thickness * scale;
			vertices.push_back(v);
			v.x = x - thickness * scale + x_percent;
			v.y = y - thickness * scale;
			vertices.push_back(v);
			

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
	        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	        value_text.renderItem(true);
		}

		void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) override {
		    double xpos, ypos;
		    glfwGetCursorPos(window, &xpos, &ypos);

		    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		    {

		    	// horizontal 
		    	if((((xpos / w) * 4.0f - 1.0f) <= (x + x_percent + thickness*scale)) && ((xpos / w) * 4.0f - 1.0f) >= (x + x_percent - thickness*scale)){
		    		if((1.0f - (ypos / h) * 4.0f) <= (y + thickness*scale) && (1.0f - (ypos / h) * 4.0f) >= (y - thickness*scale)){
				        isDragging = true;
				        lastMouseX = xpos;
				        return;

		    		}
		    	}

		    	isClickOut = true;
		    }
		    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		    {
		        isDragging = false;
		    }
		}

		void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) override {
		    if (!isDragging) return;

		    	double dx = xpos - lastMouseX;

			    lastMouseX = xpos;

			    float ndc_dx =  4.0f * dx / w;

			    x_percent += ndc_dx;

			    x_percent = std::max((- width * 1/4 + thickness)* scale, std::min(x_percent, (width * 1/4 - thickness)* scale));

			    float a = (- width * 1/4 + thickness)* scale;
			    float b = (width * 1/4 - thickness)* scale;

			    value_change = x_percent / (b - a)  - a / (b - a);

			    text = std::to_string(value_change);
			    value_text.text = text.substr(0, 4);
			    value_text.loadVertices();
			    value_text.renderSetUp();

			    loadVertices();
		}
};
