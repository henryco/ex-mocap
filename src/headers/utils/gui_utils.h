//
// Created by xd on 27/11/24.
//

#ifndef EX_MOTION_GUI_UTILS_H
#define EX_MOTION_GUI_UTILS_H

namespace ex::gui {

    bool plain_close_button();

    bool plain_button(const char *label, const bool &state);

    bool plain_button(const char *label);

    bool toggle_button(const char *label);

    bool toggle_button(const char *label, const bool &state);

    bool plain_tooltip(const char *label);
}

#endif // EX_MOTION_GUI_UTILS_H
