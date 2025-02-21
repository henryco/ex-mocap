//
// Created by xd on 02/12/24.
//

#ifndef EX_MOTION_GUI_LAYOUT_H
#define EX_MOTION_GUI_LAYOUT_H

#include "../../utils/gui_utils.h"
#include "../../state.h"
#include "../gui_state.h"

#include <imgui_internal.h>

#include <imgui.h>

namespace ex::gui {

    using type_id_  = std::size_t;
    using local_id_ = std::size_t;

    static constexpr float offset_main_h = 8.f;
    static constexpr float offset_main_v = 23.f;
    static constexpr float offset_logs_v = 4.0f;

    static void (*left_pane_menu)(State &state, GuiState &gui)  = nullptr;
    static void (*right_pane_menu)(State &state, GuiState &gui) = nullptr;

    namespace layout {
        /**
         * @return should exit
         */
        bool build(ex::State &state);
    }

    #define cond_node() (gui.view.reset ? ImGuiCond_Always : ImGuiCond_Once)

    inline int dock_host_flag(GuiState &gui) {
        return 0
               | ((gui.view.docking || gui.view.first_run) ? 0 : ImGuiDockNodeFlags_NoUndocking)
        #ifndef EXM_DISABLE_EXPERIMENTAL
               | ((!gui.view.docking && !gui.view.first_run) ? ImGuiDockNodeFlags_NoWindowMenuButton : 0)
               | ((gui.view.docking || gui.view.first_run) ? 0 : ImGuiDockNodeFlags_NoDocking)
               | ImGuiDockNodeFlags_DockedWindowsInFocusRoute
        #endif
                ;
    }

    inline float left_offset(GuiState &gui) { return gui.view.show_left ? (gui.view.pane_width_left + offset_main_h) : 0; }

    inline float right_offset(GuiState &gui) { return gui.view.show_right ? (gui.view.pane_width_right + offset_main_h) : 0; }

    inline float bottom_offset(GuiState &gui) { return gui.view.show_logs ? (offset_main_v) : 0; }

    inline float logs_expanded_offset(GuiState &gui) { return gui.view.expanded_logs ? (gui.view.pane_height_bottom + offset_logs_v) : 0; }

    inline ImVec2 main_size(GuiState &gui) { return { gui.view.main_free_x - left_offset(gui) - right_offset(gui), 0 }; }

    inline ImVec2 content_size(GuiState &gui) { return { 0, gui.view.main_free_y - bottom_offset(gui) - logs_expanded_offset(gui)}; }

    inline int menu_flag(GuiState &gui) { return gui.view.show_menu ? ImGuiWindowFlags_MenuBar : 0; }

    inline int reset_flag(GuiState &gui, const bool standalone = false) {
        return gui.view.reset ? (ImGuiWindowFlags_NoSavedSettings
                                 | (standalone ? ImGuiWindowFlags_AlwaysAutoResize : 0))
                              : 0;
    }

    inline bool *watch_parent_dock(GuiState &gui) {
        if (ex::gui::window_withing_dock(gui.docking.dock_left_id))
            return &gui.view.show_left;
        if (ex::gui::window_withing_dock(gui.docking.dock_right_id))
            return &gui.view.show_right;
        return nullptr;
    }

    static int content_resize_flag(GuiState &gui) {
        static bool content_resize_flag_first = true;
        if (!gui.view.expanded_logs) {
            content_resize_flag_first = true;
            return 1;
        }
        if (content_resize_flag_first) {
            content_resize_flag_first = false;
            return 1;
        }
        return ImGuiChildFlags_ResizeY;
    }

    void show_alert_dialogs();

    void show_file_dialogs();

    void show_menu_top_bar();

    void show_logs_pane();

    void show_work_tree();

    void show_history_tree();

    void show_main_container();

    void show_work_window();

}
#endif // EX_MOTION_GUI_LAYOUT_H
