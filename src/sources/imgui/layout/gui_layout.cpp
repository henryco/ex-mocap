//
// Created by xd on 24/11/24.
//

#include "../../../headers/imgui/layout/gui_layout.h"
#include "../../../headers/imgui/gui_actions.h"
#include "../../../headers/imgui/gui_globals.h"

namespace ex::gui {

    bool is_window_resized_() {
        static ImVec2 iwr_last_size = {};
        const auto current_size = ImGui::GetContentRegionAvail();
        if (current_size.x != iwr_last_size.x || current_size.y != iwr_last_size.y) {
            iwr_last_size = current_size;
            return true;
        }
        return false;
    }

    void check_for_reset_(GuiState &gui) {
        if (!gui.window.reset)
            return;
        gui.window.reset        = false;
        gui.window.resized      = true;
        gui.view = {};
        gui.view.reset = true;
    }

    void cleanup_(GuiState &gui) {
        gui.view.first_run = false;
        if (gui.view.reset) {
            gui.view.reset = false;
        }
    }

    void process_shortcuts_() {
        static int size     = 0;
        const auto registry = actions::list_actions(&size);

        for (int i = 0; i < size; i++) {
            if (ImGui::Shortcut(registry[i].key)) {
                if (registry[i].fn != nullptr)
                    registry[i].fn();
                return;
            }
        }
    }

    bool layout::build(ex::State &) {
        GuiState &gui = ex::gui::state();
        gui.re_size = {};

        ImGui::GetStyle().TabRounding = 0.0f;

        ImGui::Begin("main", nullptr,
                     ImGuiWindowFlags_NoDecoration
                             | ImGuiWindowFlags_NoDocking
                             | ImGuiWindowFlags_NoBringToFrontOnFocus
                             | menu_flag(gui)
                             | reset_flag(gui));
        {
            gui.window.resized = is_window_resized_();

            check_for_reset_(gui);

            gui.view.main_free_x = ImGui::GetContentRegionAvail().x;
            gui.view.main_free_y = ImGui::GetContentRegionAvail().y;

            show_file_dialogs();
            show_menu_top_bar();
            show_left_pane();
            show_main_container();
            show_right_pane();

            show_work_tree();
            show_history_tree();
            show_work_window();

            process_shortcuts_();

            if (gui.view.show_debug)
                ImGui::ShowMetricsWindow(&gui.view.show_debug);
        }
        ImGui::End();

        cleanup_(gui);

        return gui.window.exit;
    }
} // namespace ex::gui
