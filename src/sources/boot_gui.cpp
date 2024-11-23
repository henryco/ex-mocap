//
// Created by henryco on 23/11/24.
//

#include <iostream>

#include "../headers/boot.h"
#include "../headers/gl/render.h"
#include "../headers/imgui/gui.h"

#include <GLFW/glfw3.h>

int ex::boot::gui(int argc, char **argv, const char *project_path) {

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW";
        return 1;
    }

    auto window = ex::gl::create_window("ExMoCap");
    if (!window) {
        glfwTerminate();
        return 1;
    }

    struct ex::State state {};
    auto render = ex::gl::Render(window);
    {
        auto gui = ex::gui::GUI(window);

        while (true) {
            gui.update_gui(state);

            if (render.render(ex::gui::GUI::draw_gui))
                break;

        }
    }

    return 0;
}
