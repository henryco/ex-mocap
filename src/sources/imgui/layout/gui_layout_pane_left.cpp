//
// Created by xd on 02/12/24.
//

#include "../../../headers/imgui/gui_layout.h"
#include "../../../headers/imgui/gui_globals.h"
#include "../../../headers/logic/logic_globals.h"

namespace ex::gui {

    void show_left_pane_header() {
        State &state = ex::logic::globals::state();
        GuiState &gui = ex::gui::state();
        ImGui::BeginChild("InnerLeftHeader", ImVec2(gui.view.pane_width_left, 28),
                ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoDecoration);
        {
            if (left_pane_menu == nullptr) ImGui::Dummy(ImVec2(10, 12));
            else left_pane_menu(state, gui);
            ImGui::Separator();
        }
        ImGui::EndChild();
    }

    void show_left_dock_space() {
        GuiState &gui = ex::gui::state();

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


    void show_left_pane() {
        State &state = ex::logic::globals::state();
        GuiState &gui = ex::gui::state();

        if (!gui.view.show_left)
            return;
        const auto old_width = gui.view.pane_width_left;

        ImGuiStyle& style = ImGui::GetStyle();
        ImVec2 default_window_padding = style.WindowPadding;
        style.WindowPadding = ImVec2(0, 0);
        ImGui::BeginChild("LeftPane",
                          ImVec2(gui.view.pane_width_left, 0),
                          0
                                  | ImGuiChildFlags_Borders
                                  | ImGuiChildFlags_ResizeX,
                          reset_flag(gui));
        {
            style.WindowPadding = ImVec2(0, default_window_padding.y + 1);
            show_left_pane_header();
            style.WindowPadding = ImVec2(0, 0);
            show_left_dock_space();
        }

        gui.view.pane_width_left = ImGui::GetWindowWidth();
        ImGui::EndChild();
        style.WindowPadding = default_window_padding;
        ImGui::SameLine();

        gui.re_size.inc_x = (!gui.re_size.inc_x) && (old_width != gui.view.pane_width_left);
    }


}