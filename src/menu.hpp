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
#include "repere.hpp"
#include "ttf-reader.hpp"


class Menu : public Item {
	protected :
		std::string x_pos, y_pos;

		float width, height;
		float prop_width, prop_height;

		float delta_width, delta_height;
		float newdelta_width, newdelta_height;

		float cornerRadius;
		int cornerResolution;

		std::string color;

		bool isDragging = false;
		bool isDoubleClicked = false;

		double dragOffsetX = 0.0;
		double dragOffsetY = 0.0;

		double lastMouseX = 0.0;
		double lastMouseY = 0.0;

		double lastClickTime = 0.0;

		double animationTime = 0.0f;
		bool endAnimation = false;

		Repere editSize;

	public:
		Menu(GLFWwindow* _window, const std::string & _x, const std::string & _y, float _width, float _height, const std::string &_color, float _cornerRadius, const char* _vertexPath, const char* _fragmentPath) 
		: Item(_window, _vertexPath, _fragmentPath), x_pos(_x), y_pos(_y), color(_color), prop_width(_width), prop_height(_height), cornerRadius(_cornerRadius), editSize(window, 0.0f, 0.0f, 1.0f, _vertexPath, _fragmentPath){
			
			cornerResolution = 10;
			
			width  = prop_width  * 2.0f / w;
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
		}
		void loadVertices() override {

			vertices.clear();

			if(!isDoubleClicked)

				glfwGetFramebufferSize(window, &w, &h);
				width  = prop_width  * 2.0f / w;
				height = prop_height * 2.0f / h;

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
				v.r = 0.0f; v.g = 0.0f; v.b = 0.0f;
			if(color == "dark")
				v.r = 0.20f; v.g = 0.2f; v.b = 0.2f;



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
			if(color == "dark")
				v.r = 0.3f; v.g = 0.3f; v.b = 0.3f;

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
	        glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.size());

	        if(isDoubleClicked){
	        	editSize.xpos = x;
        		editSize.ypos = y;
	        	editSize.renderItem(false);


	        	double now = glfwGetTime();
		        if (now - animationTime < 0.35) { // 250 ms
		          	if(editSize.scale <= 1.2f)
				    	editSize.scale += 0.01f;
		        }
		        if (now - animationTime < 0.35 && endAnimation) { // 250 ms
		          	if(editSize.alpha <= 0.5f)
				    	editSize.alpha += 0.01f;
		        } else endAnimation = true;
		        animationTime = now;

		        editSize.loadVertices();


			   	newdelta_width = width - editSize.lenght_x;
		        newdelta_height = height - editSize.lenght_y;

		        if((delta_width - newdelta_width) * 1.7f + width>= 0.01f)
		        	prop_width += ((delta_width - newdelta_width) * 1.7f) * w/2.0f;
		        else
		        	width = 0.01f * w/2.0f;
		        if((delta_height - newdelta_height) * 1.7f + height >= 0.01f)
		        	prop_height += ((delta_height - newdelta_height) * 1.7f) * h/2.0f;
		        else 
		        	height = 0.01f * h/2.0f;

	        	if(editSize.isClickOut){
	        		isDoubleClicked = false; editSize.isClickOut = false; editSize.lenght_x = 0.15f; editSize.lenght_y = 0.15f;
	        	}
	        	loadVertices();
	        }
				
		}

		void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) override {
		    double xpos, ypos;
		    glfwGetCursorPos(window, &xpos, &ypos);

		    if (isDoubleClicked) {
		        editSize.mouse_button_callback(window, button, action, mods);
		        return;
		    }

		   	delta_width = width - editSize.lenght_x;
	        delta_height = height - editSize.lenght_y;

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

				        if(isDoubleClicked){

				        	editSize.xpos = x;
        					editSize.ypos = y;

							editSize.loadVertices();
							editSize.renderSetUp();
							editSize.renderItem(false);

				        }

		    		} else isDoubleClicked = false;
		    	} else isDoubleClicked = false;
		    }
		    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		    {
		        isDragging = false;
		    }
		}

		void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) override {

		    if (isDoubleClicked) {
		        editSize.cursor_position_callback(window, xpos, ypos);
		        return;
		    }

		    if (!isDragging) return;
		    double dx = xpos - lastMouseX;
		    double dy = ypos - lastMouseY;

		    lastMouseX = xpos;
		    lastMouseY = ypos;

		    float ndc_dx =  4.0f * dx / w;
		    float ndc_dy = -4.0f * dy / h;

		    x += ndc_dx;
		    y += ndc_dy;

		    //std::cout << x << " " << y << std::endl;

		    x = std::max(-1.0f + width/2 + cornerRadius * 2.0f / w, std::min(1.0f - width/2 - cornerRadius * 2.0f / w, x));
		    y = std::max(-1.0f + height/2 + cornerRadius * 2.0f / h, std::min(1.0f - height/2 - cornerRadius * 2.0f / h, y));

		    loadVertices();
		}

};





