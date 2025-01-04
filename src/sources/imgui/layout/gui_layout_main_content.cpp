//
// Created by xd on 02/12/24.
//

#include "../../../headers/imgui/gui_layout.h"
#include <IconsLucide.h>

namespace ex::gui {

    void show_content(ex::State &state, ReSize &re_size, GuiState &gui) {
        const auto old_height = gui.view.pane_height_content;

        ImGui::BeginChild("ContentBox",
                          content_size(gui),
                          0
                                  | ImGuiChildFlags_Borders
                                  | content_resize_flag(gui)
                                  | ((re_size.inc_y || gui.window.resized) ? ImGuiChildFlags_AutoResizeY : 0),
                          reset_flag(gui)
        );
        if (ImGui::BeginTabBar("MainTabs")) {
            if (ImGui::BeginTabItem("Tab 1")) {
                ImGui::Text("Content for Tab 1");
                ImGui::Text(ICON_LC_SETTINGS " icon " ICON_LC_SETTINGS_2);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Tab 2")) {
                ImGui::Text("Content for Tab 2");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        gui.view.pane_height_content = ImGui::GetWindowHeight();
        ImGui::EndChild();

        re_size.inc_y = (!re_size.inc_y) && (old_height != gui.view.pane_height_content);
    }

    void show_main_container(ex::State &state, ReSize &re_size, GuiState &gui) {
        const auto old_width = gui.view.main_width;
        const auto old_height = gui.view.main_height;

        ImGui::BeginChild("MainContainer",
                          main_size(gui),
                          0
                                  | (gui.view.show_right ? ImGuiChildFlags_ResizeX : 0)
                                  | ((re_size.inc_x || gui.window.resized) ? ImGuiChildFlags_AutoResizeX : 0)
                                  ,
                          0
                                  | ImGuiWindowFlags_NoScrollWithMouse
                                  | ImGuiWindowFlags_NoScrollbar
                                  | reset_flag(gui)
        );
        {
            show_content(state, re_size, gui);
            show_logs_pane(state, re_size, gui);
        }

        gui.view.main_width = ImGui::GetWindowWidth();
        gui.view.main_height = ImGui::GetWindowHeight();
        ImGui::EndChild();

        re_size.inc_x = (!re_size.inc_x) && (old_width != gui.view.main_width);
        re_size.inc_y = (!re_size.inc_y) && (old_height != gui.view.main_height);
    }

}
