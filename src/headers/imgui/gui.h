//
// Created by henryco on 22/11/24.
//

#ifndef GUI_BOOT_H
#define GUI_BOOT_H

#include "../state.h"

// ReSharper disable once CppUnusedIncludeDirective
#include <IconsLucide.h>
#include <GLFW/glfw3.h>

namespace ex::gui {
    class GUI {

    protected:
        GLFWwindow *window;

    public:
        explicit GUI(GLFWwindow *window);

        ~GUI();

        void build(ex::State &state) const;

        void draw(float delta) const;

    protected:
        void layout(ex::State &state) const;
    };
} // namespace ex::gui

#endif // GUI_BOOT_H
