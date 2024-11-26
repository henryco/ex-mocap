//
// Created by henryco on 22/11/24.
//

#ifndef GUI_BOOT_H
#define GUI_BOOT_H

#include "../state.h"

#include <IconsLucide.h>
#include <GLFW/glfw3.h>

namespace ex::gui {
    class GUI {

    private:
        GLFWwindow *window;

    public:
        explicit GUI(GLFWwindow *window);

        ~GUI();

        void update_gui(ex::State &state) const;

        static void draw_gui(float delta);

    protected:
        static void layout(ex::State &state);
    };
} // namespace ex::gui

#endif // GUI_BOOT_H
