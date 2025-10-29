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

class Repere : public Item {
	protected :

		float thickness = 0.01f;

		bool isDragging = false;
		bool isVertical = false;

		double dragOffsetX = 0.0;
		double dragOffsetY = 0.0;

		double lastMouseX = 0.0;
		double lastMouseY = 0.0;
		
	public:

		double scale;
		double alpha = 0.0f;

		bool isClickOut = false;

		float lenght_x = 0.15f; 
		float lenght_y = 0.15f; 

		Repere(GLFWwindow* _window, float _x, float _y, float _scale, const char* vertexPath, const char* fragmentPath) 
		: Item(_window, vertexPath, fragmentPath), scale(_scale){

			x = _x; y = _y;

		}


		void loadVertices() override {

			x = xpos; y = ypos;

			vertices.clear();

			Vertex v;
			v.alpha = alpha;

			// Axe x (horizontal)

			v.r = 1.0f; v.g = 0.0f; v.b = 0.0f;
			v.x = x - thickness/2 * scale;
			v.y = y - thickness/2 * scale;
			vertices.push_back(v);
			v.x = x + lenght_x * scale - thickness * scale;
			v.y = y - thickness/2 * scale;
			vertices.push_back(v);
			v.x = x + lenght_x * scale - thickness * scale;
			v.y = y + thickness/2 * scale;
			vertices.push_back(v);
			v.x = x + lenght_x * scale - thickness * scale;
			v.y = y + thickness/2 * scale;
			vertices.push_back(v);
			v.x = x - thickness/2 * scale;
			v.y = y + thickness/2 * scale;
			vertices.push_back(v);
			v.x = x - thickness/2 * scale;
			v.y = y - thickness/2 * scale;
			vertices.push_back(v);

			// sommet x

			v.x = x - thickness * scale + lenght_x * scale;
			v.y = y - thickness * scale;
			vertices.push_back(v);
			v.x = x + thickness * scale + lenght_x * scale;
			v.y = y - thickness * scale;
			vertices.push_back(v);
			v.x = x + thickness * scale + lenght_x * scale;
			v.y = y + thickness * scale;
			vertices.push_back(v);
			v.x = x + thickness * scale + lenght_x * scale;
			v.y = y + thickness * scale;
			vertices.push_back(v);
			v.x = x - thickness * scale + lenght_x * scale;
			v.y = y + thickness * scale;
			vertices.push_back(v);
			v.x = x - thickness * scale + lenght_x * scale;
			v.y = y - thickness * scale;
			vertices.push_back(v);

			// Axe y (Vertical)

			v.r = 0.0f; v.g = 1.0f; v.b = 0.0f;
			v.x = x - thickness/2 * scale;
			v.y = y - thickness/2 * scale;
			vertices.push_back(v);
			v.x = x - thickness/2 * scale;
			v.y = y + lenght_y * scale - thickness * scale;
			vertices.push_back(v);
			v.x = x + thickness/2 * scale;
			v.y = y + lenght_y * scale - thickness * scale;
			vertices.push_back(v);
			v.x = x + thickness/2 * scale;
			v.y = y + lenght_y * scale - thickness * scale;
			vertices.push_back(v);
			v.x = x + thickness/2 * scale;
			v.y = y - thickness/2 * scale;
			vertices.push_back(v);
			v.x = x - thickness/2 * scale;
			v.y = y - thickness/2 * scale;
			vertices.push_back(v);

			// sommet y

			v.x = x - thickness * scale;
			v.y = y - thickness * scale + lenght_y * scale;
			vertices.push_back(v);
			v.x = x + thickness * scale;
			v.y = y - thickness * scale + lenght_y * scale;
			vertices.push_back(v);
			v.x = x + thickness * scale;
			v.y = y + thickness * scale + lenght_y * scale;
			vertices.push_back(v);
			v.x = x + thickness * scale;
			v.y = y + thickness * scale + lenght_y * scale;
			vertices.push_back(v);
			v.x = x - thickness * scale;
			v.y = y + thickness * scale + lenght_y * scale;
			vertices.push_back(v);
			v.x = x - thickness * scale;
			v.y = y - thickness * scale + lenght_y * scale;
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
		}

		bool mouse_button_callback(GLFWwindow* window, int button, int action, int mods) override {
		    double xpos, ypos;
		    glfwGetCursorPos(window, &xpos, &ypos);

		    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		    {

		    	// horizontal 
		    	if((((xpos / w) * 4.0f - 1.0f) <= (x + (lenght_x + thickness)*scale)) && ((xpos / w) * 4.0f - 1.0f) >= (x - (lenght_x + thickness)*scale)){
		    		if((1.0f - (ypos / h) * 4.0f) <= (y + thickness*scale) && (1.0f - (ypos / h) * 4.0f) >= (y - thickness*scale)){

				        isDragging = true;
				        lastMouseX = xpos;
				        isVertical = false;
				        return true;

		    		}
		    	}
		    	// vertical 
		    	if((((xpos / w) * 4.0f - 1.0f) <= (x + thickness*scale)) && ((xpos / w) * 4.0f - 1.0f) >= (x - thickness*scale)){
		    		if((1.0f - (ypos / h) * 4.0f) <= (y + (lenght_y + thickness)*scale) && (1.0f - (ypos / h) * 4.0f) >= (y - (lenght_y + thickness)*scale)){

				        isDragging = true;
				        lastMouseY = ypos;
				        isVertical = true;
				        return true;

		    		}
		    	}
		    	isClickOut = true;
		    	scale = 1.0f;
		    	alpha = 0.0f;
		    	return false;
		    }
		    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		    {
		        isDragging = false;
		        return false;
		    }
		    return false;
		}

		bool cursor_position_callback(GLFWwindow* window, double xpos, double ypos) override {
		    if (!isDragging) return false;
		    if(isVertical){
			    double dy = ypos - lastMouseY;

			    lastMouseY = ypos;

			    float ndc_dy = -4.0f * dy / h;

			    lenght_y += ndc_dy;

			    y += ndc_dy;

			    loadVertices();
			    return true;
		    } else {
		    	double dx = xpos - lastMouseX;

			    lastMouseX = xpos;

			    float ndc_dx =  4.0f * dx / w;

			    lenght_x += ndc_dx;

			    x += ndc_dx;

			    loadVertices();
			    return true;
		    }
		    return false;
		}

	    bool contains(double xpos, double ypos) override {
  	      	float ndc_x = (xpos / w) * 4.0f - 1.0f;
	        float ndc_y = 1.0f - (ypos / h) * 4.0f;
	        
	        // Zone élargie pour faciliter la sélection
	        return 	(((((xpos / w) * 4.0f - 1.0f) <= (x + (lenght_x + thickness)*scale)) && ((xpos / w) * 4.0f - 1.0f) >= (x - (lenght_x + thickness)*scale) &&
		   			(1.0f - (ypos / h) * 4.0f) <= (y + thickness*scale) && (1.0f - (ypos / h) * 4.0f) >= (y - thickness*scale)) || ((((xpos / w) * 4.0f - 1.0f) <= (x + thickness*scale)) && ((xpos / w) * 4.0f - 1.0f) >= (x - thickness*scale) &&
		    		(1.0f - (ypos / h) * 4.0f) <= (y + (lenght_y + thickness)*scale) && (1.0f - (ypos / h) * 4.0f) >= (y - (lenght_y + thickness)*scale)));
		}
};
