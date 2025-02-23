//
// Created by xd on 2/23/25.
//

#include "../../../../headers/imgui/layout/display/gui_display.h"
#include "../../../../headers/imgui/gui_globals.h"
#include "../../../../headers/utils/gui_utils.h"

#include <imgui_internal.h>

#include <IconsLucide.h>

namespace ex::gui::display {

    ImGuiWindowClass create_display_dock_host_class_() {
        ImGuiWindowClass class_;
        class_.ClassId = 111777111;
        class_.DockingAllowUnclassed = false;
        class_.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoCloseButton;
        return class_;
    }

    inline int dock_host_flag_(GuiState &gui) {
        constexpr int docked = ImGuiDockNodeFlags_NoUndocking;
        return 0 | (gui.view.docking ? 0 : (ImGuiDockNodeFlags_NoDocking | docked))
#ifndef EXM_DISABLE_EXPERIMENTAL
                   | ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton
#endif
                ;
    }

    static const ImGuiWindowClass display_dock_host_class_ = create_display_dock_host_class_();

    void show_display() {
        GuiState &gui = ex::gui::state();
        static bool docking_empty = false;

        const int flags = docking_empty ? (ImGuiDockNodeFlags_NoDockingSplit) : 0;
        const int aux_flags = ImGuiDockNodeFlags_PassthruCentralNode | dock_host_flag_(gui);

        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.f, 0.f, 0.f, .0f));

        gui.docking.dock_main_id = ImGui::GetID("DockMainDisplay");
        ImGui::DockSpace(
            gui.docking.dock_main_id,
            ImVec2(0.0f, 0.0f),
            ImGuiDockNodeFlags_None | aux_flags | flags,
            &display_dock_host_class_);

        docking_empty = ex::gui::is_docking_empty(gui.docking.dock_main_id);

        ImGui::PopStyleColor();

        ImGui::SetNextWindowClass(&display_dock_host_class_);
        ImGui::SetNextWindowDockID(gui.docking.dock_main_id, gui.view.reset ? ImGuiCond_Always : ImGuiCond_Once);
        ImGui::Begin("Window 1");
        ImGui::Text("Content for Window 1: %d", docking_empty);
        ImGui::Text(ICON_LC_SETTINGS " icon " ICON_LC_SETTINGS_2);
        ImGui::End();

        ImGui::SetNextWindowClass(&display_dock_host_class_);
        ImGui::SetNextWindowDockID(gui.docking.dock_main_id, gui.view.reset ? ImGuiCond_Always : ImGuiCond_Once);
        ImGui::Begin("Window 2");
        ImGui::Text("Content for Window 2: %d", docking_empty);
        ImGui::End();

    }

}