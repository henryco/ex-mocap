//
// Created by xd on 24/11/24.
//

#include "../../headers/imgui/gui_layout.h"
#include "../../headers/imgui/gui_globals.h"

// TODO REMOVE
#include <iostream>


#include "../../../extra/simpfp/include/simpfp.hpp"
// TODO REMOVE

namespace ex::gui {

    bool is_window_resized() {
        static ImVec2 iwr_last_size = {};
        const auto current_size = ImGui::GetContentRegionAvail();
        if (current_size.x != iwr_last_size.x || current_size.y != iwr_last_size.y) {
            iwr_last_size = current_size;
            return true;
        }
        return false;
    }

    void check_for_reset(GuiState &gui) {
        if (!gui.window.reset)
            return;
        gui.window.reset        = false;
        gui.window.resized      = true;
        gui.view = {};
        gui.view.reset = true;
    }

    void cleanup(GuiState &gui) {
        gui.view.first_run = false;
        if (gui.view.reset) {
            gui.view.reset = false;
        }
    }

    void process_shortcuts(ex::State &state, GuiState &) {
        const auto &[undo, redo] = key_mappings().history;
        if (ImGui::Shortcut(undo.key)) {
            if (undo_history().can_undo()) {
                undo_history().undo();
                undo_history().apply(&state);
            }
        }
        else if (ImGui::Shortcut(redo.key)) {
            if (undo_history().can_redo()) {
                undo_history().redo();
                undo_history().apply(&state);
            }
        }
    }

    bool layout::build(ex::State &state) {
        static GuiState gui;

        ReSize re_size = {};

        ImGui::GetStyle().TabRounding = 0.0f;

        ImGui::Begin("main", nullptr,
                     ImGuiWindowFlags_NoDecoration
                             | ImGuiWindowFlags_NoDocking
                             | ImGuiWindowFlags_NoBringToFrontOnFocus
                             | menu_flag(gui)
                             | reset_flag(gui));
        {
            gui.window.resized = is_window_resized();

            check_for_reset(gui);

            gui.view.main_free_x = ImGui::GetContentRegionAvail().x;
            gui.view.main_free_y = ImGui::GetContentRegionAvail().y;

            show_menu_top_bar(state, gui);
            show_left_pane(state, re_size, gui);
            show_main_container(state, re_size, gui);
            show_right_pane(state, re_size, gui);

            show_work_tree(state, gui);
            show_history_tree(state, gui);
            show_work_window(state, gui);

            process_shortcuts(state, gui);

            // TODO REMOVE
            static bool fp_open = true;
            static constexpr auto min_size = ImVec2(900, 500);

            if (!simpfp::FileDialogOpen())
                simpfp::OpenFileDialog("Select a file", nullptr);

            if (simpfp::ShowFileDialog("Select a file", &fp_open, min_size, true)) {

                if (char file[256]; simpfp::FileAccepted(file, sizeof(file))) {
                    std::cout << "accepted: " << file << '\n';
                    simpfp::CloseFileDialog();
                }

                simpfp::EndFileDialog();
            }
            // TODO REMOVE

            if (gui.view.show_debug)
                ImGui::ShowMetricsWindow(&gui.view.show_debug);
        }
        ImGui::End();

        cleanup(gui);

        return gui.window.exit;
    }
} // namespace ex::gui
