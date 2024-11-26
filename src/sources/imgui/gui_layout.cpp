//
// Created by xd on 24/11/24.
//

#include "../../headers/imgui/gui.h"

#include <imgui.h>

namespace ex::gui {

    struct ReSize {
        bool inc_x = false;
        bool inc_y = false;
    };

    static bool window_resized = false;

    static float offset_main_h = 16.f;
    static float offset_main_v = 23.f;
    static float offset_logs_v = 4.0f;

    static float pane_width_left     = 200.0f;
    static float pane_width_right    = 200.0f;
    static float pane_height_bottom  = 200.0f;
    static float pane_height_content = 0.f;

    static float main_width  = 0.f;
    static float main_height = 0.f;
    static float main_free_x = 0.f;
    static float main_free_y = 0.f;

    static bool show_right = true;
    static bool show_left  = true;
    static bool show_logs  = true;
    static bool show_menu  = true;

    static bool expanded_logs = false;

    inline float left_offset() { return show_left ? pane_width_left : 0; }

    inline float right_offset() { return show_right ? (pane_width_right + offset_main_h) : 0; }

    inline float bottom_offset() { return show_logs ? (offset_main_v) : 0; }

    inline float logs_expanded_offset() { return expanded_logs ? (pane_height_bottom + offset_logs_v) : 0; }

    inline ImVec2 main_size() { return { main_free_x - left_offset() - right_offset(), 0 }; }

    inline ImVec2 content_size() { return { 0, main_free_y - bottom_offset() - logs_expanded_offset()}; }

    inline int menu_flag() { return show_menu ? ImGuiWindowFlags_MenuBar : 0; }

    inline int content_resize_flag() {
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

    void show_left_pane(ex::State &state, ReSize &re_size) {
        if (!show_left)
            return;
        const auto old_width = pane_width_left;
        ImGui::BeginChild("LeftPane",
                          ImVec2(pane_width_left, 0),
                          0
                          | ImGuiChildFlags_Borders
                          | ImGuiChildFlags_ResizeX);

        {
            // TODO CONTENT

            ImGuiID dock_space_id = ImGui::GetID("LeftDockSpace");
            ImGui::DockSpace(dock_space_id,
                             ImVec2(0.0f, 0.0f),
                             ImGuiDockNodeFlags_None
                                     | ImGuiDockNodeFlags_PassthruCentralNode
                                     | ImGuiDockNodeFlags_AutoHideTabBar);
        }

        pane_width_left = ImGui::GetWindowWidth();
        ImGui::EndChild();
        ImGui::SameLine();

        re_size.inc_x = (!re_size.inc_x) && (old_width != pane_width_left);
    }

    void show_right_pane(ex::State &state, ReSize &re_size) {
        if (!show_right)
            return;
        const auto old_width = pane_width_right;
        ImGui::SameLine();
        ImGui::BeginChild("RightPane",
                          ImVec2((re_size.inc_x || window_resized) ? 0 : pane_width_right, 0),
                          0
                          | ImGuiChildFlags_Borders);

        {
            // TODO CONTENT
            ImGuiID dock_space_id = ImGui::GetID("RightDockSpace");
            ImGui::DockSpace(dock_space_id,
                             ImVec2(0.0f, 0.0f),
                             ImGuiDockNodeFlags_None
                                     | ImGuiDockNodeFlags_PassthruCentralNode
                                     | ImGuiDockNodeFlags_AutoHideTabBar);
        }

        pane_width_right = ImGui::GetWindowWidth();
        ImGui::EndChild();

        re_size.inc_x = (!re_size.inc_x) && (old_width != pane_width_right);
    }

    void show_content(ex::State &state, ReSize &re_size) {
        const auto old_height = pane_height_content;

        ImGui::BeginChild("ContentBox",
                          content_size(),
                          0
                                  | ImGuiChildFlags_Borders
                                  | content_resize_flag()
                                  | ((re_size.inc_y || window_resized) ? ImGuiChildFlags_AutoResizeY : 0)
                          );
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
        pane_height_content = ImGui::GetWindowHeight();
        ImGui::EndChild();

        re_size.inc_y = (!re_size.inc_y) && (old_height != pane_height_content);
    }

    void show_logs_pane(ex::State &state, ReSize &re_size) {
        if (!show_logs)
            return;
        const auto old_height = pane_height_bottom;
        if ((expanded_logs = ImGui::CollapsingHeader("Log Output"))) {
            ImGui::BeginChild("BottomPane",
                              ImVec2(0, (re_size.inc_y || window_resized) ? 0 : pane_height_bottom),
                              0
                              | ImGuiChildFlags_Borders);

            {
                // TODO LOGS
            }

            pane_height_bottom = ImGui::GetWindowHeight();
            ImGui::EndChild();
        }
        re_size.inc_y = (!re_size.inc_y) && (old_height != pane_height_bottom);
    }

    void show_main_container(ex::State &state, ReSize &re_size) {
        const auto old_width = main_width;
        const auto old_height = main_height;

        ImGui::BeginChild("MainContainer",
                          main_size(),
                          0
                                  | (show_right ? ImGuiChildFlags_ResizeX : 0)
                                  | ((re_size.inc_x || window_resized) ? ImGuiChildFlags_AutoResizeX : 0)
                                  ,
                          0
                                  | ImGuiWindowFlags_NoScrollWithMouse
                                  | ImGuiWindowFlags_NoScrollbar
                          );
        {
            show_content(state, re_size);
            show_logs_pane(state, re_size);
        }

        main_width = ImGui::GetWindowWidth();
        main_height = ImGui::GetWindowHeight();
        ImGui::EndChild();

        re_size.inc_x = (!re_size.inc_x) && (old_width != main_width);
        re_size.inc_y = (!re_size.inc_y) && (old_height != main_height);
    }

    bool is_window_resized() {
        static ImVec2 iwr_last_size = {};
        const auto current_size = ImGui::GetContentRegionAvail();
        if (current_size.x != iwr_last_size.x || current_size.y != iwr_last_size.y) {
            iwr_last_size = current_size;
            return true;
        }
        return false;
    }

    void GUI::layout(ex::State &state) {
        ReSize re_size = {};

        ImGui::Begin("main", nullptr,ImGuiWindowFlags_NoDecoration | menu_flag());
        {
            window_resized = is_window_resized();
            main_free_x = ImGui::GetContentRegionAvail().x;
            main_free_y = ImGui::GetContentRegionAvail().y;

            show_menu_bar(state);
            show_left_pane(state, re_size);
            show_main_container(state, re_size);
            show_right_pane(state, re_size);
        }
        ImGui::End();
    }
} // namespace ex::gui
