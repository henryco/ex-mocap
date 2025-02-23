//
// Created by xd on 2/23/25.
//

#include "../../../../headers/imgui/layout/display/gui_display.h"
#include "../../../../headers/imgui/gui_globals.h"
#include "../../../../headers/utils/gui_utils.h"

#include <imgui_internal.h>

#include <IconsLucide.h>

namespace ex::gui::display {

    inline int dock_host_flag_(GuiState &gui) {
        return 0 | (gui.view.docking ? 0 : (ImGuiDockNodeFlags_NoDocking | ImGuiDockNodeFlags_NoUndocking))
#ifndef EXM_DISABLE_EXPERIMENTAL
                   | ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton
#endif
                ;
    }

    void show_display() {
        GuiState &gui = ex::gui::state();
        static bool docking_empty = false;

        const int flags = docking_empty ? (ImGuiDockNodeFlags_NoDockingSplit) : 0;
        const int aux_flags = ImGuiDockNodeFlags_PassthruCentralNode | dock_host_flag_(gui);

        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.f, 0.f, 0.f, .0f));

        gui.docking.dock_main_id = ImGui::GetID("DockMainDisplay");
        ImGui::DockSpace(gui.docking.dock_main_id,
                         ImVec2(0.0f, 0.0f),
                         ImGuiDockNodeFlags_None
                                 | aux_flags
                                 | flags
        );

        docking_empty = ex::gui::is_docking_empty(gui.docking.dock_main_id);

        ImGui::PopStyleColor();

//        const auto node = ImGui::DockBuilderGetNode(gui.docking.dock_main_id);
//        ImGui::Begin("debug window");
//        ImGui::DebugNodeDockNode(node, "debug");
//        ImGui::End();

        ImGuiWindowClass window_class_1;
        window_class_1.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoCloseButton;
        ImGui::SetNextWindowClass(&window_class_1);
        ImGui::SetNextWindowDockID(gui.docking.dock_main_id, gui.view.reset ? ImGuiCond_Always : ImGuiCond_Once);
        ImGui::Begin("Window 1");
        ImGui::Text("Content for Window 1: %d", docking_empty);
        ImGui::Text(ICON_LC_SETTINGS " icon " ICON_LC_SETTINGS_2);
        ImGui::End();

        ImGuiWindowClass window_class_2;
        window_class_1.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoCloseButton;
        ImGui::SetNextWindowClass(&window_class_2);
        ImGui::SetNextWindowDockID(gui.docking.dock_main_id, gui.view.reset ? ImGuiCond_Always : ImGuiCond_Once);
        ImGui::Begin("Window 2");
        ImGui::Text("Content for Window 2: %d", docking_empty);
        ImGui::End();

    }

}