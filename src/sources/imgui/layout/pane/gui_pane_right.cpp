//
// Created by henryco on 21/02/25.
//

#include "../../../../headers/imgui/layout/pane/gui_pane_right.h"
#include "../../../../headers/imgui/layout/pane/gui_pane.h"
#include "../../../../headers/imgui/gui_globals.h"
#include "../../../../headers/utils/gui_utils.h"

#include <imgui_internal.h>

namespace ex::gui {

    namespace pane {

        static draw_fn right_menu_draw_fn_ = nullptr;

        void set_right_menu_fn(const draw_fn &fn) {
            right_menu_draw_fn_ = fn;
        }

        const draw_fn &get_right_menu_fn() {
            return right_menu_draw_fn_;
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

    void show_right_pane_header_() {
        const GuiState &gui = ex::gui::state();
        ImGui::BeginChild("InnerRightHeader",
                ImVec2((gui.re_size.inc_x || gui.window.resized) ? 0 : gui.view.pane_width_right, 28),
                ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoDecoration);
        {
            ImGui::SameLine();
            if (pane::right_menu_draw_fn_ == nullptr)
                ImGui::Dummy(ImVec2(10, 12));
            else
                pane::right_menu_draw_fn_();
            ImGui::Separator();
        }
        ImGui::EndChild();

        pane::right_menu_draw_fn_ = nullptr;
    }

    void show_right_dock_space_() {
        GuiState &gui = ex::gui::state();
        static bool first_time = true;

        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.f, 0.f, 0.f, .0f));
        ImGui::PushStyleColor(ImGuiCol_TabSelected, ImVec4(0.f, 0.f, 0.f, .0));
        ImGui::PushStyleColor(ImGuiCol_TabDimmedSelected, ImVec4(0.f, 0.f, 0.f, .0f));

        const int flags = (first_time || ex::gui::is_docking_empty(gui.docking.dock_right_id))
                                  ? ImGuiDockNodeFlags_NoDockingSplit
                                  : 0;

        const int aux_flags = ImGuiDockNodeFlags_PassthruCentralNode
                              | dock_host_flag_(gui);

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

    void show_right_pane() {
        GuiState &gui = ex::gui::state();

        if (!gui.view.show_right)
            return;
        const auto old_width = gui.view.pane_width_right;
        ImGui::SameLine();
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec2 default_window_padding = style.WindowPadding;
        style.WindowPadding = ImVec2(0, 0);
        ImGui::BeginChild("RightPane",
                          ImVec2((gui.re_size.inc_x || gui.window.resized) ? 0 : gui.view.pane_width_right, 0),
                          0
                                  | ImGuiChildFlags_Borders,
                          reset_flag_(gui));
        {
            style.WindowPadding = ImVec2(0, default_window_padding.y + 1);
            show_right_pane_header_();
            style.WindowPadding = ImVec2(0, 0);
            show_right_dock_space_();
        }

        gui.view.pane_width_right = ImGui::GetWindowWidth();
        ImGui::EndChild();
        style.WindowPadding = default_window_padding;

        gui.re_size.inc_x = (!gui.re_size.inc_x) && (old_width != gui.view.pane_width_right);
    }

}