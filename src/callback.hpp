#pragma once
#include <vector>
#include <algorithm>
#include <GLFW/glfw3.h>

class CallBack {
protected:
    std::vector<Item*> callback_list;
    std::vector<Item*> sorted_list;
    GLFWwindow* window;
    Item* activeItem = nullptr;
    
    void sortByPriority() {
        sorted_list = callback_list;
        std::sort(sorted_list.begin(), sorted_list.end(), 
            [](Item* a, Item* b) { 
                return a->priority > b->priority; 
            });
    }
    
    static void mouse_cb(GLFWwindow* w, int button, int action, int mods) {
        auto* self = static_cast<CallBack*>(glfwGetWindowUserPointer(w));
        if (!self) return;
        
        double xpos, ypos;
        glfwGetCursorPos(w, &xpos, &ypos);
        
        if (self->activeItem && action != GLFW_RELEASE) {
            self->activeItem->mouse_button_callback(w, button, action, mods);
            return;
        }
        
        for (auto* item : self->sorted_list) {
            if (!item) continue;
            
            bool consumed = item->mouse_button_callback(w, button, action, mods);
            
            if (action == GLFW_PRESS && consumed) {
                self->activeItem = item;
            }
            
            if (consumed) {
                break;
            }
        }
        
        if (action == GLFW_RELEASE) {
            if (self->activeItem) {
                self->activeItem->mouse_button_callback(w, button, action, mods);
            }
            self->activeItem = nullptr;
        }
    }
    
    static void cursor_cb(GLFWwindow* w, double xpos, double ypos) {
        auto* self = static_cast<CallBack*>(glfwGetWindowUserPointer(w));
        if (!self) return;
        
        if (self->activeItem) {
            self->activeItem->cursor_position_callback(w, xpos, ypos);
            return;
        }
        
        for (auto* item : self->sorted_list) {
            if (!item) continue;
            
            bool consumed = item->cursor_position_callback(w, xpos, ypos);
            if (consumed) break;
        }
    }
    
public:
    CallBack(GLFWwindow* _window, const std::vector<Item*>& _callback_list)
    : window(_window), callback_list(_callback_list) {
        sortByPriority();
        glfwSetWindowUserPointer(window, this);
        glfwSetMouseButtonCallback(window, CallBack::mouse_cb);
        glfwSetCursorPosCallback(window, CallBack::cursor_cb);
    }
    
    void addItem(Item* item) {
        callback_list.push_back(item);
        sortByPriority();
    }
    
    void removeItem(Item* item) {
        callback_list.erase(
            std::remove(callback_list.begin(), callback_list.end(), item),
            callback_list.end()
        );
        sortByPriority();
    }
    
    void refresh() {
        sortByPriority();
    }
    
    // Debug: afficher l'ordre de priorité
    void printPriorities() {
        std::cout << "Callback order (by priority):\n";
        for (auto* item : sorted_list) {
            std::cout << "  - Item at " << item << " with priority " << item->priority << "\n";
        }
    }
};