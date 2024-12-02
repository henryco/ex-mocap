//
// Created by xd on 02/12/24.
//

#include "../../../headers/imgui/gui_layout.h"

namespace ex::gui {

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

        // TODO REMOVE
        ImGui::Dummy(ImVec2(0.0f, ImGui::GetContentRegionAvail().y - ImGui::CalcTextSize(ICON_LC_PLUS).y - ImGui::GetStyle().ItemSpacing.x - 14));
        // TODO REMOVE

        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();
        {
            if (ex::gui::toggle_button(ICON_LC_PLUS)) {
                // TODO
            }
            ex::gui::plain_tooltip("Add workspace");
            ImGui::SameLine(0, 10);
            if (ex::gui::toggle_button(ICON_LC_MINUS)) {
                // TODO
            }
            ex::gui::plain_tooltip("Remove workspace");
        }

        ImGui::EndChild();

        ImGui::End();
    }

}