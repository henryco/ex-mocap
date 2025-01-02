//
// Created by xd on 27/11/24.
//

#ifndef EX_MOTION_GUI_UTILS_H
#define EX_MOTION_GUI_UTILS_H

#include <imgui.h>

namespace ex::gui {

    void next_no_close();

    void next_auto_hide(bool header);

    bool window_withing_dock(const ImGuiID &id);

    bool window_is_multi_docked();

    bool window_is_docked();

    bool is_docking_empty(const ImGuiID &id);

    bool plain_close_button();

    bool plain_button(const char *label, const bool &state);

    bool plain_button(const char *label);

    bool toggle_button(const char *label);

    bool toggle_button(const char *label, const bool &state);

    bool plain_tooltip(const char *label);

    ImVec2 brackets_start(float indent = 0.f);

    void brackets_end(const ImVec2 &start, float indent = 0.f);

    void brackets_end_half(const ImVec2 &start, float indent = 0.f);
}

#endif // EX_MOTION_GUI_UTILS_H
