//
// Created by henryco on 22/11/24.
//

#ifndef GUI_BOOT_H
#define GUI_BOOT_H

#include "../utils/log_utils.h"
#include "../state.h"

#include <GLFW/glfw3.h>

namespace ex::gui {
    class GUI {

    public:
        static inline const auto log =
            ex::logs::get_logger("im_gui");

    private:
        GLFWwindow *window;

    public:
        explicit GUI(GLFWwindow *window);

        ~GUI();

        void build(ex::State &state) const;

        void draw(float delta) const;

        bool should_close() const;
    };
} // namespace ex::gui

#endif // GUI_BOOT_H
