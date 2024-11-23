//
// Created by henryco on 22/11/24.
//

#ifndef GUI_BOOT_H
#define GUI_BOOT_H

#include <GLFW/glfw3.h>
#include "../state.h"

namespace ex::gui {
    class GUI {

    private:
        GLFWwindow *window;

    public:
        explicit GUI(GLFWwindow *window);

        ~GUI();

        void draw_gui();

        void update_gui(ex::State &state);
    };

    int boot(int argc, char **argv, const char *project_path);
} // namespace ex::gui

#endif // GUI_BOOT_H
