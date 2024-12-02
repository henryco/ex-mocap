//
// Created by xd on 02/12/24.
//

#include "../../../headers/imgui/gui_layout.h"

namespace ex::gui {

    void show_right_dock_space(ex::State &state, GuiState &gui) {
        static bool first_time = true;

        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.f, 0.f, 0.f, .0f));
        ImGui::PushStyleColor(ImGuiCol_TabSelected, ImVec4(0.f, 0.f, 0.f, .0));
        ImGui::PushStyleColor(ImGuiCol_TabDimmedSelected, ImVec4(0.f, 0.f, 0.f, .0f));

        const int flags = (first_time || ex::gui::is_docking_empty(gui.docking.dock_right_id))
                                  ? ImGuiDockNodeFlags_NoDockingSplit
                                  : 0;

        const int aux_flags = ImGuiDockNodeFlags_PassthruCentralNode
                              | dock_host_flag(gui);

        gui.docking.dock_right_id = ImGui::GetID("DockMainRight");
        ImGui::DockSpace(gui.docking.dock_right_id,
                         ImVec2(0.0f, 0.0f),
                         ImGuiDockNodeFlags_None
                                 | aux_flags
                                 | flags
        );

        if (first_time || gui.view.reset) {
            first_time = false;
            // TODO
        }

        ImGui::PopStyleColor(3);
    }

    void show_right_pane(ex::State &state, ReSize &re_size, GuiState &gui) {
        if (!gui.view.show_right)
            return;
        const auto old_width = gui.view.pane_width_right;
        ImGui::SameLine();
        ImGui::BeginChild("RightPane",
                          ImVec2((re_size.inc_x || gui.window.window_resized) ? 0 : gui.view.pane_width_right, 0),
                          0
                                  | ImGuiChildFlags_Borders,
                          reset_flag(gui));

        {
            ImGui::Text("%d", ex::gui::is_docking_empty(gui.docking.dock_right_id));
            // TODO
            ImGui::Separator();
            show_right_dock_space(state, gui);
        }

        gui.view.pane_width_right = ImGui::GetWindowWidth();
        ImGui::EndChild();

        re_size.inc_x = (!re_size.inc_x) && (old_width != gui.view.pane_width_right);
    }

}