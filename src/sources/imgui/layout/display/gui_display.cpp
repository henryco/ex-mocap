//
// Created by xd on 2/23/25.
//

#include "../../../../headers/imgui/layout/display/gui_display.h"
#include "../../../../headers/imgui/gui_globals.h"
#include <IconsLucide.h>

namespace ex::gui::display {

    void show_display() {

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

    }

}