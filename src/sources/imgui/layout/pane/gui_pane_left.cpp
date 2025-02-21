//
// Created by henryco on 21/02/25.
//

#include "../../../../headers/imgui/layout/pane/gui_pane_left.h"
#include "../../../../headers/imgui/layout/pane/gui_pane.h"
#include "../../../../headers/imgui/gui_globals.h"
#include "../../../../headers/utils/gui_utils.h"

#include <imgui_internal.h>

namespace ex::gui {

    namespace pane {

        static draw_fn left_menu_draw_fn_ = nullptr;

        void set_left_menu_fn(const draw_fn &fn) {
            left_menu_draw_fn_ = fn;
        }

        const draw_fn &get_left_menu_fn() {
            return left_menu_draw_fn_;
        }

    }

    inline int dock_host_flag_(GuiState &gui) {
        return 0 | ((gui.view.docking || gui.view.first_run) ? 0 : ImGuiDockNodeFlags_NoUndocking)
#ifndef EXM_DISABLE_EXPERIMENTAL
               | ((!gui.view.docking && !gui.view.first_run) ? ImGuiDockNodeFlags_NoWindowMenuButton : 0) |
               ((gui.view.docking || gui.view.first_run) ? 0 : ImGuiDockNodeFlags_NoDocking) |
               ImGuiDockNodeFlags_DockedWindowsInFocusRoute
#endif
                ;
    }

    inline int reset_flag_(GuiState &gui, const bool standalone = false) {
        return gui.view.reset
                       ? (ImGuiWindowFlags_NoSavedSettings | (standalone ? ImGuiWindowFlags_AlwaysAutoResize : 0))
                       : 0;
    }

    void show_left_pane_header_() {
        const GuiState &gui = ex::gui::state();
        ImGui::BeginChild("InnerLeftHeader", ImVec2(gui.view.pane_width_left, 28),
                ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoDecoration);
        {
            if (pane::left_menu_draw_fn_ == nullptr)
                ImGui::Dummy(ImVec2(10, 12));
            else
                pane::left_menu_draw_fn_();
            ImGui::Separator();
        }
        ImGui::EndChild();
    }

    void show_left_dock_space_() {
        GuiState &gui = ex::gui::state();

        static bool first_time = true;

        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.f, 0.f, 0.f, .0f));
        ImGui::PushStyleColor(ImGuiCol_TabSelected, ImVec4(0.f, 0.f, 0.f, .0));
        ImGui::PushStyleColor(ImGuiCol_TabDimmedSelected, ImVec4(0.f, 0.f, 0.f, .0f));

        const int flags = (ex::gui::is_docking_empty(gui.docking.dock_left_id))
                                  ? ImGuiDockNodeFlags_NoDockingSplit
                                  : 0;

        const int aux_flags = ImGuiDockNodeFlags_PassthruCentralNode
                              | dock_host_flag_(gui);

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
        GuiState &gui = ex::gui::state();

        if (!gui.view.show_left)
            return;
        const auto old_width = gui.view.pane_width_left;

        ImGuiStyle& style = ImGui::GetStyle();
        const ImVec2 default_window_padding = style.WindowPadding;
        style.WindowPadding = ImVec2(0, 0);
        ImGui::BeginChild("LeftPane",
                          ImVec2(gui.view.pane_width_left, 0),
                          0
                                  | ImGuiChildFlags_Borders
                                  | ImGuiChildFlags_ResizeX,
                          reset_flag_(gui));
        {
            style.WindowPadding = ImVec2(0, default_window_padding.y + 1);
            show_left_pane_header_();
            style.WindowPadding = ImVec2(0, 0);
            show_left_dock_space_();
        }

        gui.view.pane_width_left = ImGui::GetWindowWidth();
        ImGui::EndChild();
        style.WindowPadding = default_window_padding;
        ImGui::SameLine();

        gui.re_size.inc_x = (!gui.re_size.inc_x) && (old_width != gui.view.pane_width_left);
    }

}