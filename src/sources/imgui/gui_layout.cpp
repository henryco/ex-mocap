//
// Created by xd on 24/11/24.
//

#include "../../headers/imgui/gui.h"

#include <imgui.h>

namespace ex::gui {

    static float left_pane_width  = 200.0f;
    static float right_pane_width = 200.0f;
    static float bottom_pane_height = 150.0f;
    static float main_width = 0.f;

    static bool show_right = true;
    static bool show_left = true;
    static bool show_logs = true;

    void show_left_pane(ex::State &state) {
        if (!show_left)
            return;
        ImGui::BeginChild("LeftPane", ImVec2(left_pane_width, 0), true);
        ImGui::Text("Left Pane (e.g., File Explorer)");
        ImGui::Button("Sample Button");
        ImGui::EndChild();
        ImGui::SameLine();
    }

    void show_right_pane(ex::State &state) {
        if (!show_right)
            return;
        ImGui::SameLine();
        ImGui::BeginChild("RightPane", ImVec2(right_pane_width, 0), true);
        ImGui::Text("Right Pane (e.g., Properties)");
        ImGui::EndChild();
    }

    void show_logs_pane(ex::State &state) {
        if (!show_logs)
            return;
        ImGui::BeginChild("BottomPane", ImVec2(0, bottom_pane_height), true);
        ImGui::Text("Log Output:");
        ImGui::Separator();
        ImGui::Text("This is a log message.");
        ImGui::Text("Another log message.");
        ImGui::EndChild();
    }

    void show_main_content(ex::State &state) {
        ImGui::BeginChild("MainContainer",
                          ImVec2(
                                  main_width - (show_left ? left_pane_width : 0) - (show_right ? (right_pane_width + 16) : 0),
                                  -bottom_pane_height),
                          true);

        if (ImGui::BeginTabBar("MainTabs")) {
            if (ImGui::BeginTabItem("Tab 1")) {
                ImGui::Text("Content for Tab 1");
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Tab 2")) {
                ImGui::Text("Content for Tab 2");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        show_logs_pane(state);
        ImGui::EndChild();
    }

    void GUI::layout(ex::State &state) {
        ImGui::Begin("main", nullptr,
                     ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoScrollWithMouse);

        main_width = ImGui::GetContentRegionAvail().x;

        show_left_pane(state);
        show_main_content(state);
        show_right_pane(state);

        ImGui::End();
    }
}