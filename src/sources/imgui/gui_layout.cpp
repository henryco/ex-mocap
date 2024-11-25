//
// Created by xd on 24/11/24.
//

#include "../../headers/imgui/gui.h"

#include <imgui.h>

namespace ex::gui {

    static float offset_main_h = 16.f;
    static float offset_main_v = 23.f;

    static float left_pane_width    = 200.0f;
    static float right_pane_width   = 200.0f;
    static float main_width         = 0.f;

    static bool show_right = true;
    static bool show_left  = true;
    static bool show_logs  = true;
    static bool show_menu  = true;

    static bool expanded_logs = false;

    static inline float left_offset() { return show_left ? left_pane_width : 0; }

    static inline float right_offset() { return show_right ? (right_pane_width + offset_main_h) : 0; }

    static inline float bottom_offset() { return show_logs ? (offset_main_v) : 0; }

    static inline float logs_expanded_offset() { return expanded_logs ? 200 : 0; }

    static inline ImVec2 main_size() { return { main_width - left_offset() - right_offset(), 0 }; }

    static inline ImVec2 content_size() { return { 0, -(bottom_offset() + logs_expanded_offset())}; }

    static inline int menu_flag() { return show_menu ? ImGuiWindowFlags_MenuBar : 0; }

    static inline int content_resize_flag() {
        static bool content_resize_flag_first = true;
        if (!expanded_logs) {
            content_resize_flag_first = true;
            return 1;
        }
        if (content_resize_flag_first) {
            content_resize_flag_first = false;
            return 1;
        }
        return ImGuiChildFlags_ResizeY;
    }

    void show_menu_bar(ex::State &state) {
        if (!show_menu)
            return;

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New")) { /* Handle "New" action */ }
                if (ImGui::MenuItem("Open")) { /* Handle "Open" action */ }
                if (ImGui::MenuItem("Save")) { /* Handle "Save" action */ }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) { /* Handle "Exit" action */ }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", "Ctrl+Z")) { /* Handle "Undo" action */ }
                if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {
                    /* Disabled item example */
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "Ctrl+X")) { /* Handle "Cut" action */ }
                if (ImGui::MenuItem("Copy", "Ctrl+C")) { /* Handle "Copy" action */ }
                if (ImGui::MenuItem("Paste", "Ctrl+V")) { /* Handle "Paste" action */ }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) { /* Handle "About" action */ }
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }

    void show_left_pane(ex::State &state) {
        if (!show_left)
            return;
        ImGui::BeginChild("LeftPane", ImVec2(left_pane_width, 0),
                          ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
        ImGui::Text("Left Pane (e.g., File Explorer)");
        ImGui::Button("Sample Button");
        ImGui::EndChild();
//        left_pane_width = ImGui::GetWindowWidth();
        ImGui::SameLine();
    }

    void show_right_pane(ex::State &state) {
        if (!show_right)
            return;
        ImGui::SameLine();
        ImGui::BeginChild("RightPane", ImVec2(0, 0), ImGuiChildFlags_Borders);
        ImGui::Text("Right Pane (e.g., Properties)");
        ImGui::EndChild();
//        right_pane_width = ImGui::GetWindowWidth();
    }

    void show_content(ex::State &state) {
        ImGui::BeginChild("ContentBox", content_size(),
                          ImGuiChildFlags_Borders |
                                  content_resize_flag());
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
    }

    void show_logs_pane(ex::State &state) {
        if (!show_logs)
            return;

        if ((expanded_logs = ImGui::CollapsingHeader("Log Output"))) {
            ImGui::BeginChild("BottomPane", ImVec2(0, 0), true);
            ImGui::Text("This is a log message.");
            ImGui::Text("Another log message.");
            ImGui::EndChild();
        }

    }

    void show_main_container(ex::State &state) {
        ImGui::BeginChild("MainContainer", main_size(),
                          0 | (show_right ? ImGuiChildFlags_ResizeX : 0),
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        {
            show_content(state);
            show_logs_pane(state);
        }
        ImGui::EndChild();
    }

    void GUI::layout(ex::State &state) {
        ImGui::Begin("main", nullptr,ImGuiWindowFlags_NoDecoration | menu_flag());
        {
            main_width = ImGui::GetContentRegionAvail().x;
            show_menu_bar(state);
            show_left_pane(state);
            show_main_container(state);
            show_right_pane(state);
        }
        ImGui::End();
    }
} // namespace ex::gui
