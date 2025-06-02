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

    void brackets_end(const ImVec2 &start, float indent = 0.f, float alpha = 1.0f);

    void brackets_end_half(const ImVec2 &start, float indent = 0.f, float alpha = 1.0f);

    bool begin_docked_window(const char *name, bool *p_open, bool reset = false, bool standalone = false,
                             ImGuiWindowFlags window_flags = 0);

    int window_reset_flags(bool reset, bool standalone);

    bool section_node(const char *name, bool selected = false, bool expand = false, bool collapse = false);

    bool generic_node(const char *name, bool bullet, bool selected = false, bool nested = false, bool expand = false, bool collapse = false);

    bool plain_node(const char *name, bool bullet, bool selected = false, bool nested = false, bool expand = false, bool collapse = false);

    bool show_yes_no_dialog(const char *title, const char *msg, const char *yes, const char *no);

    bool show_alert_dialog(const char *title, const char *msg, const char *ok);

    bool color_selector(const char *button, const char *label, ImVec4 *color, ImVec4 *backup_color, bool alpha = false,
                        const char *current = "Current",
                        const char *previous = "Previous");
}

#endif // EX_MOTION_GUI_UTILS_H
