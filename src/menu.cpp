#include "menu.hpp"

Item createMenu(GLFWwindow* window, const std::string &x, const std::string &y, float width, float height, const std::string &color, float cornerRadius, const char* vertexPath, const char* fragmentPath){
	Item menu(window, x, y, width, height, color, cornerRadius, vertexPath, fragmentPath);

	return menu;
}