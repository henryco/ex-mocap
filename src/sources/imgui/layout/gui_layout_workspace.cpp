//
// Created by xd on 02/12/24.
//

#include "../../../headers/imgui/gui_layout.h"

namespace ex::gui {

    void workspace_tree(ex::State &state, GuiState &gui) {
        ImGui::BeginChild(
                "WorkSpaceTree",
                ImVec2(0.0f, ImGui::GetContentRegionAvail().y - ImGui::CalcTextSize(ICON_LC_PLUS).y - ImGui::GetStyle().ItemSpacing.x - 10),
                0);
        // TODO
        ImGui::Text("test");
        ImGui::EndChild();
    }

    void workspace_buttons(ex::State &state, GuiState &gui) {
        if (ex::gui::toggle_button(ICON_LC_PLUS)) {
            logger()->debug("Add workspace");
            // TODO
        }
        ex::gui::plain_tooltip("Add workspace");
        ImGui::SameLine(0, 10);
        if (ex::gui::toggle_button(ICON_LC_MINUS)) {
            logger()->debug("Remove workspace");
            // TODO
        }
        ex::gui::plain_tooltip("Remove workspace");
    }

    void show_workspace_switcher(ex::State &state, GuiState &gui) {
        static bool *watch = nullptr;
        if (watch != nullptr && !(*watch))
            return;

        static bool docked = false;
        ex::gui::next_auto_hide(docked || gui.view.docking);
        ImGui::SetNextWindowDockID(gui.docking.dock_left_bot_id, cond_node());
        ImGui::Begin("Workspace", nullptr, reset_flag(gui) | ImGuiWindowFlags_NoCollapse);
        docked = ex::gui::window_is_multi_docked();
        watch = watch_parent_dock(gui);

        ImGui::BeginChild("WorkSpaceBox", ImVec2(0, 0), 0, ImGuiWindowFlags_NoDecoration);
        workspace_tree(state, gui);
//        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();
        workspace_buttons(state, gui);
        ImGui::EndChild();

        ImGui::End();
    }

}