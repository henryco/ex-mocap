//
// Created by xd on 23/11/24.
//

#include "../../src/headers/gl/render.h"
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

    Render::Render(GLFWwindow *window) {
        this->window = window;
        glfwMakeContextCurrent(this->window);
    }

    Render::~Render() {
        glfwDestroyWindow(this->window);
        glfwTerminate();
    }

    bool Render::render(void (*func)(float delta)) {
        if (glfwWindowShouldClose(window))
            return true;

        glfwPollEvents();

        int display_w, display_h;
        glfwGetFramebufferSize(this->window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        if (func != nullptr)
            func(0.f);

        glfwSwapBuffers(this->window);

        return false;
    }
}


