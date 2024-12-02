//
// Created by xd on 02/12/24.
//

#include "../../../headers/imgui/gui_layout.h"

namespace ex::gui {

    void show_menu_top_bar(ex::State &state, GuiState &gui) {
        if (!gui.view.show_menu)
            return;

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New")) {
                    // TODO
                }
                if (ImGui::MenuItem("Open")) {
                    // TODO
                }
                if (ImGui::MenuItem("Save")) {
                    // TODO
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) {
                    gui.view.should_exit = true;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
                    // TODO
                }
                if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {
                    // TODO
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "Ctrl+X")) {
                    // TODO
                }
                if (ImGui::MenuItem("Copy", "Ctrl+C")) {
                    // TODO
                }
                if (ImGui::MenuItem("Paste", "Ctrl+V")) {
                    // TODO
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                if (ImGui::MenuItem("Unpin windows", nullptr, &gui.view.docking)) {
                    gui.window.window_resized = true;
                }
                ImGui::Separator();

                if (ImGui::MenuItem("Sidebar Left", nullptr, &gui.view.show_left)) {
                    gui.window.window_resized = true;
                }

                if (ImGui::MenuItem("Sidebar Right", nullptr, &gui.view.show_right)) {
                    gui.window.window_resized = true;
                }

                if(ImGui::MenuItem("Logs", nullptr, &gui.view.show_logs)) {
                    gui.view.expanded_logs = gui.view.show_logs;
                    gui.window.window_resized = true;
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Reset layout")) {
                    gui.window.reset_layout = true;
                    gui.window.window_resized = true;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {
                    // TODO
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }

}