//
// Created by xd on 24/11/24.
//

#include "../../headers/imgui/gui.h"

#include <imgui.h>

namespace ex::gui {

    static float offset_px = 16.f;

    static float left_pane_width    = 200.0f;
    static float right_pane_width   = 200.0f;
    static float bottom_pane_height = 150.0f;
    static float main_width         = 0.f;

    static bool show_right = true;
    static bool show_left  = true;
    static bool show_logs  = true;

    static inline float left_offset() { return show_left ? left_pane_width : 0; }

    static inline float right_offset() { return show_right ? (right_pane_width + offset_px) : 0; }

    static inline ImVec2 main_size() { return { main_width - left_offset() - right_offset(), 0 }; }

    static inline ImVec2 content_size() { return { 0, -(bottom_pane_height + 8) }; }

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
        ImGui::BeginChild("MainContainer", main_size(), true);
        {
            ImGui::BeginChild("ContentBox", content_size(), true);
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
            ImGui::EndChild();
            show_logs_pane(state);
        }
        ImGui::EndChild();
    }

    void GUI::layout(ex::State &state) {
        ImGui::Begin("main", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoScrollWithMouse);

        main_width = ImGui::GetContentRegionAvail().x;

        show_left_pane(state);
        show_main_content(state);
        show_right_pane(state);

        ImGui::End();
    }
} // namespace ex::gui
