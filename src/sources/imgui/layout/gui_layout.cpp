//
// Created by xd on 24/11/24.
//

#include "../../../headers/imgui/layout/gui_layout.h"
#include "../../../headers/imgui/layout/pane/gui_pane_left.h"
#include "../../../headers/imgui/layout/pane/gui_pane_right.h"
#include "../../../headers/imgui/actions/gui_actions.h"
#include "../../../headers/imgui/actions/gui_actions_registry.h"
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

    void check_for_window_closed_(State &state) {
        if (!glfwWindowShouldClose(state.gl.window))
            return;
        glfwSetWindowShouldClose(state.gl.window, GLFW_FALSE);
        actions::exit_action();
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

    void register_default_shortcuts_() {
        static const auto &[file, history, work_tree, layout] = key_mappings();
        actions::registry::set_action(file.create.key, &actions::new_action);
        actions::registry::set_action(file.open.key, &actions::open_action);
        actions::registry::set_action(file.save.key, &actions::save_action);
        actions::registry::set_action(file.exit.key, &actions::exit_action);
        actions::registry::set_action(file.save_as.key, &actions::save_as_action);
        actions::registry::set_action(history.undo.key, &actions::undo_action);
        actions::registry::set_action(history.redo.key, &actions::redo_action);
        actions::registry::set_action(layout.pin.key, &actions::pin_unpin_layout_action);
    }

    void process_shortcuts_() {
        static int size     = 0;
        const auto registry = actions::registry::list_actions(&size);

        for (int i = 0; i < size; i++) {
            if (ImGui::Shortcut(registry[i].key, ImGuiInputFlags_RouteGlobal)) {
                if (registry[i].fn != nullptr)
                    registry[i].fn();
                return;
            }
        }
    }

    bool layout::build(ex::State &state) {
        GuiState &gui = ex::gui::state();

        gui.view.show_fps = state.metrics.timing_enabled;
        gui.re_size       = {};

        ImGui::GetStyle().TabRounding = 0.0f;

        ImGui::Begin("main", nullptr,
                     ImGuiWindowFlags_NoDecoration
                             | ImGuiWindowFlags_NoDocking
                             | ImGuiWindowFlags_NoBringToFrontOnFocus
                             | menu_flag(gui)
                             | reset_flag(gui));
        {
            gui.window.resized = is_window_resized_();

            check_for_window_closed_(state);
            check_for_reset_(gui);

            gui.view.main_free_x = ImGui::GetContentRegionAvail().x;
            gui.view.main_free_y = ImGui::GetContentRegionAvail().y;

            register_default_shortcuts_();

            show_alert_dialogs();
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
