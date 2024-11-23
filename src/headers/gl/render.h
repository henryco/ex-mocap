//
// Created by xd on 23/11/24.
//

#ifndef EX_MOTION_RENDER_H
#define EX_MOTION_RENDER_H

#include <GLFW/glfw3.h>
namespace ex::gl {

    GLFWwindow* create_window(const char* title);

    class Render {

    private:
        GLFWwindow *window;

    public:
        explicit Render(GLFWwindow *window);

        ~Render();

        void loop(void (*func)(float delta));

    };
}

#endif // EX_MOTION_RENDER_H
