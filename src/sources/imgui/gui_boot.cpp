//
// Created by henryco on 22/11/24.
//

#include "../../src/headers/imgui/gui_boot.h"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>
#include <iostream>

namespace ex::gui {

    GLFWwindow* create_window(const char* title) {
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

    int boot(const int argc, char **argv, const char *project_path) {

        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW";
            return 1;
        }

        auto window = create_window("Example Application");
        if (!window) {
            glfwTerminate();
            return 1;
        }

        glfwMakeContextCurrent(window);

        while (!glfwWindowShouldClose(window)) {
            glClear(GL_COLOR_BUFFER_BIT);
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glfwDestroyWindow(window);
        glfwTerminate();

        return 0;
    }
}


