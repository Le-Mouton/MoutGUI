class CallBack {
protected:
    std::vector<Item*> callback_list;
    GLFWwindow* window;

    static void mouse_cb(GLFWwindow* w, int button, int action, int mods) {
        auto* self = static_cast<CallBack*>(glfwGetWindowUserPointer(w));
        if (!self) return;
        for (auto* item : self->callback_list) {
            if (item) item->mouse_button_callback(w, button, action, mods);
        }
    }

    static void cursor_cb(GLFWwindow* w, double xpos, double ypos) {
        auto* self = static_cast<CallBack*>(glfwGetWindowUserPointer(w));
        if (!self) return;
        for (auto* item : self->callback_list) {
            if (item) item->cursor_position_callback(w, xpos, ypos);
        }
    }

public:
    CallBack(GLFWwindow* _window, const std::vector<Item*>& _callback_list)
    : window(_window), callback_list(_callback_list) {
        glfwSetWindowUserPointer(window, this);
        glfwSetMouseButtonCallback(window, CallBack::mouse_cb);
        glfwSetCursorPosCallback(window, CallBack::cursor_cb);
    }


};