//
// Created by henryco on 26/11/24.
//

#ifndef GLFW_UTILS_H
#define GLFW_UTILS_H

#include <GLFW/glfw3.h>

namespace ex::gl {

    GLFWwindow* create_window(const char* title, int *refresh_rate = nullptr);

    void set_window_title(GLFWwindow *window, const char* title);

}

#endif //GLFW_UTILS_H
