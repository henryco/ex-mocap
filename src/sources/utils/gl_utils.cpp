//
// Created by henryco on 26/11/24.
//

#include "../../headers/utils/gl_utils.h"

#include <iostream>

namespace ex::gl {

    GLFWwindow *create_window(const char *title) {
        GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
        if (!primaryMonitor) {
            std::cerr << "Failed to get primary monitor\n";
            return nullptr;
        }

        const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
        if (!videoMode) {
            std::cerr << "Failed to get video mode\n";
            return nullptr;
        }

        GLFWwindow* window = glfwCreateWindow(
                videoMode->width,
                videoMode->height,
                title,
                nullptr,
                nullptr);

        if (!window) {
            std::cerr << "Failed to create GLFW window\n";
            return nullptr;
        }

        int monitorX, monitorY;
        glfwGetMonitorPos(primaryMonitor, &monitorX, &monitorY);
        glfwSetWindowPos(window, monitorX, monitorY);

        return window;
    }

}