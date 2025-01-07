//
// Created by xd on 02/12/24.
//

#include "../../../headers/imgui/gui_layout.h"
#include <IconsLucide.h>

namespace ex::gui {

    void end_pin_button(ex::State &state, GuiState &gui) {
        const auto icon = gui.view.docking ? ICON_LC_PIN_OFF : ICON_LC_PIN;
        ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize(icon).x
                - ImGui::GetStyle().FramePadding.x * 3 - ImGui::GetStyle().ItemSpacing.x * 1);
        if (ex::gui::plain_button(icon)) {
            gui.view.docking = !gui.view.docking;
            gui.window.resized = true;
            logger()->debug("unpin windows: {}", gui.view.docking);
        }
        ex::gui::plain_tooltip("Unpin windows");
    }

    void show_menu_top_bar(ex::State &state, GuiState &gui) {
        if (!gui.view.show_menu)
            return;

        if (ImGui::BeginMenuBar()) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));

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
                    gui.window.exit = true;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit")) {

                if (ImGui::MenuItem(ICON_LC_UNDO " Undo", "Ctrl+Z", false, undo_history().can_undo())) {
                    undo_history().undo();
                    undo_history().apply(&state);
                }
                if (ImGui::MenuItem(ICON_LC_REDO " Redo", "Ctrl+Y", false, undo_history().can_redo())) {
                    undo_history().redo();
                    undo_history().apply(&state);
                }

                ImGui::Separator();

                if (ImGui::MenuItem(ICON_LC_SCISSORS " Cut", "Ctrl+X", false, false)) {
                    // TODO
                }
                if (ImGui::MenuItem(ICON_LC_CLIPBOARD_COPY " Copy", "Ctrl+C", false, false)) {
                    // TODO
                }
                if (ImGui::MenuItem(ICON_LC_CLIPBOARD_PASTE " Paste", "Ctrl+V", false, false)) {
                    // TODO
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                if (ImGui::MenuItem("Unpin windows", nullptr, &gui.view.docking)) {
                    gui.window.resized = true;
                    logger()->debug("unpin windows: {}", gui.view.docking);
                }
                ImGui::Separator();

                if (ImGui::MenuItem("Sidebar Left", nullptr, &gui.view.show_left)) {
                    gui.window.resized = true;
                }

                if (ImGui::MenuItem("Sidebar Right", nullptr, &gui.view.show_right)) {
                    gui.window.resized = true;
                }

                if(ImGui::MenuItem("Logs", nullptr, &gui.view.show_logs)) {
                    gui.view.expanded_logs = gui.view.show_logs;
                    gui.window.resized     = true;
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Reset layout")) {
                    gui.window.reset        = true;
                    gui.window.resized      = true;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {
                    // TODO
                }
                ImGui::EndMenu();
            }

            end_pin_button(state, gui);
            ImGui::PopStyleVar();
            ImGui::EndMenuBar();
        }
    }

}