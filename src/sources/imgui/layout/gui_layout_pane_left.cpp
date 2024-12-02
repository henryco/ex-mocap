//
// Created by xd on 02/12/24.
//

#include "../../../headers/imgui/gui_layout.h"

namespace ex::gui {

    void show_left_dock_space(ex::State &state, GuiState &gui) {
        static bool first_time = true;

        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.f, 0.f, 0.f, .0f));
        ImGui::PushStyleColor(ImGuiCol_TabSelected, ImVec4(0.f, 0.f, 0.f, .0));
        ImGui::PushStyleColor(ImGuiCol_TabDimmedSelected, ImVec4(0.f, 0.f, 0.f, .0f));

        const int flags = (ex::gui::is_docking_empty(gui.docking.dock_left_id))
                                  ? ImGuiDockNodeFlags_NoDockingSplit
                                  : 0;

        const int aux_flags = ImGuiDockNodeFlags_PassthruCentralNode
                              | dock_host_flag(gui);

        gui.docking.dock_left_id = ImGui::GetID("DockMainLeft");
        ImGui::DockSpace(gui.docking.dock_left_id,
                         ImVec2(0.0f, 0.0f),
                         ImGuiDockNodeFlags_None
                                 | aux_flags
                                 | flags
        );

        if (first_time || gui.view.reset) {
            first_time = false;
            ImGui::DockBuilderRemoveNode(gui.docking.dock_left_id);
            ImGui::DockBuilderAddNode(gui.docking.dock_left_id,
                                      ImGuiDockNodeFlags_None
                                              | aux_flags
            );
            ImGui::DockBuilderSplitNode(gui.docking.dock_left_id,
                                        ImGuiDir_Up,
                                        0.7f,
                                        &gui.docking.dock_left_top_id,
                                        &gui.docking.dock_left_bot_id);

            if (const auto node_1 = ImGui::DockBuilderGetNode(gui.docking.dock_left_top_id))
                node_1->SetLocalFlags(aux_flags);

            if (const auto node_2 = ImGui::DockBuilderGetNode(gui.docking.dock_left_bot_id))
                node_2->SetLocalFlags(aux_flags);

            ImGui::DockBuilderFinish(gui.docking.dock_left_id);
        }

        ImGui::PopStyleColor(3);
    }


    void show_left_pane(ex::State &state, ReSize &re_size, GuiState &gui) {
        if (!gui.view.show_left)
            return;
        const auto old_width = gui.view.pane_width_left;
        ImGui::BeginChild("LeftPane",
                          ImVec2(gui.view.pane_width_left, 0),
                          0
                                  | ImGuiChildFlags_Borders
                                  | ImGuiChildFlags_ResizeX,
                          reset_flag(gui));

        {
            ImGui::Text("%d", ex::gui::is_docking_empty(gui.docking.dock_left_id));
            // TODO
            ImGui::Separator();
            show_left_dock_space(state, gui);
        }

        gui.view.pane_width_left = ImGui::GetWindowWidth();
        ImGui::EndChild();
        ImGui::SameLine();

        re_size.inc_x = (!re_size.inc_x) && (old_width != gui.view.pane_width_left);
    }


}