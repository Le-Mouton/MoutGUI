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
	float r, g, b, alpha;
};


class Item{
	public:
		float x, y;
		int w, h;


		GLFWwindow* window;
		std::vector<Vertex> vertices;
		unsigned int VBO, VAO, EBO;

		Shader shader_item;
		bool isVisible;
		int priority = 0;
		bool blocksPropagation = false;

		float xpos, ypos;
		float global_width, global_height;

		Item(GLFWwindow* _window, const char* vertexPath, const char* fragmentPath)
        : window(_window), shader_item(vertexPath, fragmentPath) {

			glfwGetFramebufferSize(window, &w, &h);

        }

		virtual void loadVertices() = 0;
		virtual void renderSetUp() = 0;
		virtual void renderItem(bool poly) = 0;

	    virtual bool mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	        return false;
	    }
	    
	    virtual bool cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	        return false;
	    }

	    // Vérifie si le point (xpos, ypos) est dans les limites de l'item
	    virtual bool contains(double xpos, double ypos) {
	        int w_width, w_height;
	        glfwGetWindowSize(window, &w_width, &w_height);
	        
	        float ndc_x = (xpos / w_width) * 4.0f - 1.0f;
	        float ndc_y = 1.0f - (ypos / w_height) * 4.0f;
	        
	        return (ndc_x >= xpos - global_width/2 && ndc_x <= xpos + global_width/2 &&
	                ndc_y >= ypos - global_height/2 && ndc_y <= ypos + global_height/2);
	    }

		virtual ~Item() = default;


};










