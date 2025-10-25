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


struct Vertex{
	float x, y, z;
	float r, g, b;
};

struct Item {
	std::string x_pos, y_pos;
	float x, y;
	float width, height;
	float cornerRadius;
	int cornerResolution;

	int w, h;

	std::string color;

	unsigned int VBO, VAO, EBO;
	Shader shader_menu;

	bool isDragging = false;
	bool isDoubleClicked = false;

	double dragOffsetX = 0.0;
	double dragOffsetY = 0.0;

	double lastMouseX = 0.0;
	double lastMouseY = 0.0;

	GLFWwindow* window;

	std::vector<Vertex> vertices;

	// Vertex v_empty = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}

	// --- préparer vec(vertices)

	Item(GLFWwindow* _window, const std::string & _x, const std::string & _y, float _width, float _height, const std::string &_color, float _cornerRadius, const char* vertexPath, const char* fragmentPath) 
	: window(_window), x_pos(_x), y_pos(_y), width(_width), height(_height), color(_color), cornerRadius(_cornerRadius), shader_menu(vertexPath, fragmentPath){
		cornerResolution = 10;
		glfwGetFramebufferSize(window, &w, &h);

        glfwSetWindowUserPointer(window, this);
		glfwSetMouseButtonCallback(window, mouse_callback_static);
		glfwSetCursorPosCallback(window, cursor_callback_static);
		
		width  = width  * 2.0f / w;
		height = height * 2.0f / h;

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

	    std::cout << x << " " << y << std::endl;

	}

	void loadVertices(){

		vertices.clear();

		Vertex v;

		if(color == "red")
			v.r = 0.4f; v.g = 0.01f; v.b = 0.01f;
		if(color == "blue")
			v.r = 0.0f; v.g = 0.0f; v.b = 1.0f;
		if(color == "green")
			v.r = 0.0f; v.g = 1.0f; v.b = 0.0f;
		if(color == "black")
			v.r = 0.0f; v.g = 0.0f; v.b = 0.0f;
		if(color == "white")
			v.r = 0.0f; v.g = 0.0f; v.b = 0.0f;


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
			// v.x = x + width/2;
			// v.y = y + height/2;
			// vertices.push_back(v);
			// v.x = x + width/2 + (cornerRadius) * 2.0f / w;
			// v.y = y + height/2;
			// vertices.push_back(v);
			// v.x = x + width/2 + (cornerRadius) * 2.0f / w;
			// v.y = y + height/2 + (cornerRadius) * 2.0f / h;
			// vertices.push_back(v);
			// v.x = x + width/2 + (cornerRadius) * 2.0f / w;
			// v.y = y + height/2 + (cornerRadius) * 2.0f / h;
			// vertices.push_back(v);
			// v.x = x + width/2;
			// v.y = y + height/2 + (cornerRadius) * 2.0f / h;
			// vertices.push_back(v);
			// v.x = x + width/2;
			// v.y = y + height/2;
			// vertices.push_back(v);


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

		v.r = 0.9f; v.g = 0.9f; v.b = 0.9f;
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
		// std::cout<< v.x <<" "<< v.y <<std::endl;
	}

	static void mouse_callback_static(GLFWwindow* _window, int button, int action, int mods) {
	    Item* item = static_cast<Item*>(glfwGetWindowUserPointer(_window));
	    if (item) item->mouse_button_callback(_window, button, action, mods);
	}

	static void cursor_callback_static(GLFWwindow* _window, double xpos, double ypos) {
	    Item* item = static_cast<Item*>(glfwGetWindowUserPointer(_window));
	    if (item) item->cursor_position_callback(_window, xpos, ypos);
	}

	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
	    double xpos, ypos;
	    glfwGetCursorPos(window, &xpos, &ypos);

	    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	    {
	    	if((((xpos / w) * 4.0f - 1.0f) <= (x + width / 2) && ((xpos / w) * 4.0f - 1.0f) >= (x - width / 2))){
	    		if((1.0f - (ypos / h) * 4.0f) <= (y + height / 2) && (1.0f - (ypos / h) * 4.0f) >= (y - height / 2)){

			        isDragging = true;
			        lastMouseX = xpos;
			        lastMouseY = ypos;

	    		} else isDoubleClicked = false;
	    	} else isDoubleClicked = false;
	    }
	    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	    {
	        isDragging = false;
	    }
	}

	void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
	{
	    if (!isDragging) return;

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
	}

	void renderMenuSetUp(){

	    glGenVertexArrays(1, &VAO);
	    glGenBuffers(1, &VBO);

	    glBindVertexArray(VAO);

	    glBindBuffer(GL_ARRAY_BUFFER, VBO);
	    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);

	    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	    glEnableVertexAttribArray(0);

	    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(float)));
	    glEnableVertexAttribArray(1);

	    glBindBuffer(GL_ARRAY_BUFFER, 0); 
	    glBindVertexArray(0);
	}

	void renderMenu(bool poly){

		if(poly){
        	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		} else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);

		shader_menu.use();

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.size());
	}
};

Item createMenu(GLFWwindow* window, const std::string &_x, const std::string &_y, float _width, float _height, const std::string &color, float _cornerRadius, const char* vertexPath, const char* fragmentPath);






