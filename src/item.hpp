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
	protected:
		float x, y;
		int w, h;
		GLFWwindow* window;
		std::vector<Vertex> vertices;
		unsigned int VBO, VAO, EBO;
		Shader shader_item;
		bool isVisible;

	public:
		Item(GLFWwindow* _window, const char* vertexPath, const char* fragmentPath)
        : window(_window), shader_item(vertexPath, fragmentPath) {

			glfwGetFramebufferSize(window, &w, &h);

        }

		virtual void loadVertices() = 0;
		virtual void renderSetUp() = 0;
		virtual void renderItem(bool poly) = 0;

		virtual void mouse_button_callback(GLFWwindow* /*window*/, int /*button*/, int /*action*/, int /*mods*/) {}
    	virtual void cursor_position_callback(GLFWwindow* /*window*/, double /*xpos*/, double /*ypos*/) {}

		virtual ~Item() = default;
};










