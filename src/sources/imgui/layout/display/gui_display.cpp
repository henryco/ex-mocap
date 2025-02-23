//
// Created by xd on 2/23/25.
//

#include "../../../../headers/imgui/layout/display/gui_display.h"
#include "../../../../headers/imgui/gui_globals.h"
#include "../../../../headers/utils/gui_utils.h"

#include <imgui_internal.h>

#include <IconsLucide.h>
#include <iostream>

namespace ex::gui::display {

//    inline int dock_host_flag_(GuiState &gui) {
//        return 0 | ((gui.view.docking || gui.view.first_run) ? 0 : ImGuiDockNodeFlags_NoUndocking)
//#ifndef EXM_DISABLE_EXPERIMENTAL
//               | ((gui.view.docking || gui.view.first_run) ? 0 : ImGuiDockNodeFlags_NoDocking) |
//                   ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton |
//                   ImGuiDockNodeFlags_NoDockingOverOther | ImGuiDockNodeFlags_NoDockingSplitOther
//#endif
//                ;
//    }

    inline int dock_host_flag_(GuiState &gui) {
        return 0 //| ImGuiDockNodeFlags_NoDocking | ImGuiDockNodeFlags_NoUndocking
#ifndef EXM_DISABLE_EXPERIMENTAL
                   | ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton
//                   | ImGuiDockNodeFlags_NoDockingOverOther | ImGuiDockNodeFlags_NoDockingSplitOther
#endif
                ;
    }

    void show_display() {
        GuiState &gui = ex::gui::state();
        static bool first_time = true;
        static bool docking_empty = false;

        std::cout << "empty: " << docking_empty << '\n';

        const int flags = (first_time || ex::gui::is_docking_empty(gui.docking.dock_main_id))
                                  ? 0
                                  : 0;

        const int aux_flags = ImGuiDockNodeFlags_PassthruCentralNode
                              | dock_host_flag_(gui);

        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.f, 0.f, 0.f, .0f));

        gui.docking.dock_main_id = ImGui::GetID("DockMainDisplay");
        ImGui::DockSpace(gui.docking.dock_main_id,
                         ImVec2(0.0f, 0.0f),
                         ImGuiDockNodeFlags_None
                                 | aux_flags
                                 | flags
        );

        if (first_time || gui.view.reset) {
            first_time = false;
            // TODO
        }

        docking_empty = ex::gui::is_docking_empty(gui.docking.dock_main_id);

        ImGui::PopStyleColor();

        const auto node = ImGui::DockBuilderGetNode(gui.docking.dock_main_id);
        ImGui::Begin("debug window");
        ImGui::DebugNodeDockNode(node, "debug");
        ImGui::End();

        ImGuiWindowClass window_class;
        window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoCloseButton
         | ImGuiDockNodeFlags_NoDockingOverOther | ImGuiDockNodeFlags_NoDockingOverMe;

        ImGui::SetNextWindowClass(&window_class);
        ImGui::SetNextWindowDockID(gui.docking.dock_main_id, gui.view.reset ? ImGuiCond_Always : ImGuiCond_Once);
        ImGui::Begin("Window 1");
        ImGui::Text("Content for Window 1");
        ImGui::Text(ICON_LC_SETTINGS " icon " ICON_LC_SETTINGS_2);
        ImGui::End();

        ImGui::SetNextWindowClass(&window_class);
        ImGui::SetNextWindowDockID(gui.docking.dock_main_id, gui.view.reset ? ImGuiCond_Always : ImGuiCond_Once);
        ImGui::Begin("Window 2");
        ImGui::Text("Content for Window 2");
        ImGui::End();

    }

}