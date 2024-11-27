//
// Created by xd on 24/11/24.
//

#include "../../headers/imgui/gui.h"
#include "../../headers/utils/gui_utils.h"

#include <imgui.h>

namespace ex::gui {

    typedef struct View {
        float pane_width_left     = 350.0f;
        float pane_width_right    = 350.0f;
        float pane_height_bottom  = 200.0f;
        float pane_height_content = 0.f;

        float main_width  = 0.f;
        float main_height = 0.f;
        float main_free_x = 0.f;
        float main_free_y = 0.f;

        bool show_right = true;
        bool show_left  = true;
        bool show_logs  = true;
        bool show_menu  = true;

        bool highlight_logs = true;
        bool expanded_logs = false;
        bool follow_logs = false;
        bool should_exit = false;
    } View;

    typedef struct ReSize {
        bool inc_x = false;
        bool inc_y = false;
    } ReSize;

    static constexpr float offset_main_h = 8.f;
    static constexpr float offset_main_v = 23.f;
    static constexpr float offset_logs_v = 4.0f;

    static bool window_resized = false;
    static bool reset_layout = false;
    static View view = View{};

    inline float left_offset() { return view.show_left ? (view.pane_width_left + offset_main_h) : 0; }

    inline float right_offset() { return view.show_right ? (view.pane_width_right + offset_main_h) : 0; }

    inline float bottom_offset() { return view.show_logs ? (offset_main_v) : 0; }

    inline float logs_expanded_offset() { return view.expanded_logs ? (view.pane_height_bottom + offset_logs_v) : 0; }

    inline ImVec2 main_size() { return { view.main_free_x - left_offset() - right_offset(), 0 }; }

    inline ImVec2 content_size() { return { 0, view.main_free_y - bottom_offset() - logs_expanded_offset()}; }

    inline int menu_flag() { return view.show_menu ? ImGuiWindowFlags_MenuBar : 0; }

    inline int content_resize_flag() {
        static bool content_resize_flag_first = true;
        if (!view.expanded_logs) {
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
        if (!view.show_menu)
            return;

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New")) {
                    // TODO
                }
                if (ImGui::MenuItem("Open")) {
                    // TODO
                }
                if (ImGui::MenuItem("Save")) {
                    // TODO
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) {
                    view.should_exit = true;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
                    // TODO
                }
                if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {
                    // TODO
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "Ctrl+X")) {
                    // TODO
                }
                if (ImGui::MenuItem("Copy", "Ctrl+C")) {
                    // TODO
                }
                if (ImGui::MenuItem("Paste", "Ctrl+V")) {
                    // TODO
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                if (ImGui::MenuItem("Sidebar Left", nullptr, &view.show_left)) {
                    window_resized = true;
                }

                if (ImGui::MenuItem("Sidebar Right", nullptr, &view.show_right)) {
                    window_resized = true;
                }

                if(ImGui::MenuItem("Logs", nullptr, &view.show_logs)) {
                    view.expanded_logs = view.show_logs;
                    window_resized = true;
                }

                ImGui::Separator();
                if (ImGui::MenuItem("Reset layout")) {
                    reset_layout = true;
                    window_resized = true;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {
                    // TODO
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }

    void show_left_pane(ex::State &state, ReSize &re_size) {
        if (!view.show_left)
            return;
        const auto old_width = view.pane_width_left;
        ImGui::BeginChild("LeftPane",
                          ImVec2(view.pane_width_left, 0),
                          0
                          | ImGuiChildFlags_Borders
                          | ImGuiChildFlags_ResizeX);

        {
            ImGuiID dock_space_id = ImGui::GetID("LeftDockSpace");
            ImGui::DockSpace(dock_space_id,
                             ImVec2(0.0f, 0.0f),
                             ImGuiDockNodeFlags_None
                                     | ImGuiDockNodeFlags_PassthruCentralNode
                                     | ImGuiDockNodeFlags_AutoHideTabBar);
        }

        view.pane_width_left = ImGui::GetWindowWidth();
        ImGui::EndChild();
        ImGui::SameLine();

        re_size.inc_x = (!re_size.inc_x) && (old_width != view.pane_width_left);
    }

    void show_right_pane(ex::State &state, ReSize &re_size) {
        if (!view.show_right)
            return;
        const auto old_width = view.pane_width_right;
        ImGui::SameLine();
        ImGui::BeginChild("RightPane",
                          ImVec2((re_size.inc_x || window_resized) ? 0 : view.pane_width_right, 0),
                          0
                          | ImGuiChildFlags_Borders);

        {
            ImGuiID dock_space_id = ImGui::GetID("RightDockSpace");
            ImGui::DockSpace(dock_space_id,
                             ImVec2(0.0f, 0.0f),
                             ImGuiDockNodeFlags_None
                                     | ImGuiDockNodeFlags_PassthruCentralNode
                                     | ImGuiDockNodeFlags_AutoHideTabBar);
        }

        view.pane_width_right = ImGui::GetWindowWidth();
        ImGui::EndChild();

        re_size.inc_x = (!re_size.inc_x) && (old_width != view.pane_width_right);
    }

    void show_content(ex::State &state, ReSize &re_size) {
        const auto old_height = view.pane_height_content;

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
                ImGui::Text(ICON_LC_SETTINGS " icon " ICON_LC_SETTINGS_2);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Tab 2")) {
                ImGui::Text("Content for Tab 2");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        view.pane_height_content = ImGui::GetWindowHeight();
        ImGui::EndChild();

        re_size.inc_y = (!re_size.inc_y) && (old_height != view.pane_height_content);
    }

    void show_logs_pane(ex::State &state, ReSize &re_size) {
        if (!view.show_logs)
            return;
        const auto old_height = view.pane_height_bottom;
        const auto old_expanded = view.expanded_logs;

        ImGui::SetNextItemAllowOverlap();

        view.expanded_logs = ImGui::CollapsingHeader(ICON_LC_LOGS " Log Output ");

        if (ex::gui::plain_close_button()) {
            view.expanded_logs = false;
            view.show_logs = false;
            window_resized = true;
        }

        if (view.expanded_logs) {
            ImGui::BeginChild("LogControlsPane",
                ImVec2(24, 0), 0, ImGuiWindowFlags_NoDecoration);
            {
                view.highlight_logs = !ex::gui::toggle_button(ICON_LC_LIGHTBULB_OFF, !view.highlight_logs);
                ex::gui::plain_tooltip("Disable highlighting for warnings and errors");
                ImGui::Spacing();

                view.follow_logs = ex::gui::toggle_button(ICON_LC_LIST_END, view.follow_logs);
                ex::gui::plain_tooltip("Follow logs");
                ImGui::Spacing();

                if (ex::gui::toggle_button(ICON_LC_CLIPBOARD_COPY)) {
                    ImGui::SetClipboardText(ex::logs::get_all_as_string().c_str());
                }
                ex::gui::plain_tooltip("Copy to clipboard");
                ImGui::Spacing();

                if (ex::gui::toggle_button(ICON_LC_TRASH_2)) {
                    ex::logs::clear_all();
                }
                ex::gui::plain_tooltip("Clear all");
                ImGui::Spacing();

            }
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::BeginChild("BottomPane",
                              ImVec2(0, (re_size.inc_y || window_resized) ? 0 : view.pane_height_bottom),
                              0
                              | ImGuiChildFlags_Borders);

            ImGui::BeginChild("LogPane", ImVec2(0, 0), ImGuiChildFlags_FrameStyle);
            {
                for (const auto &log : ex::logs::get_all_logs()) {

                    if (view.highlight_logs && log.find("[error]") != std::string::npos) {
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
                        ImGui::TextWrapped("%s", log.c_str());
                        ImGui::PopStyleColor();
                        continue;
                    }

                    ImGui::TextWrapped("%s", log.c_str());
                }
                if (view.follow_logs || !old_expanded) {
                    ImGui::SetScrollY(ImGui::GetScrollMaxY());
                }
            }
            ImGui::EndChild();

            view.pane_height_bottom = ImGui::GetWindowHeight();
            ImGui::EndChild();
            ImGui::PopStyleVar();
        }
        re_size.inc_y = (!re_size.inc_y) && (old_height != view.pane_height_bottom);
    }

    void show_main_container(ex::State &state, ReSize &re_size) {
        const auto old_width = view.main_width;
        const auto old_height = view.main_height;

        ImGui::BeginChild("MainContainer",
                          main_size(),
                          0
                                  | (view.show_right ? ImGuiChildFlags_ResizeX : 0)
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

        view.main_width = ImGui::GetWindowWidth();
        view.main_height = ImGui::GetWindowHeight();
        ImGui::EndChild();

        re_size.inc_x = (!re_size.inc_x) && (old_width != view.main_width);
        re_size.inc_y = (!re_size.inc_y) && (old_height != view.main_height);
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

    void check_for_reset() {
        if (!reset_layout)
            return;
        reset_layout = false;
        window_resized = true;
        view = {};
    }

    void should_exit(GLFWwindow *window) {
        if (!view.should_exit)
            return;
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    void GUI::layout(ex::State &state) const {
        ReSize re_size = {};

        ImGui::Begin("main", nullptr,ImGuiWindowFlags_NoDecoration | menu_flag());
        {
            window_resized = is_window_resized();

            check_for_reset();

            view.main_free_x = ImGui::GetContentRegionAvail().x;
            view.main_free_y = ImGui::GetContentRegionAvail().y;

            show_menu_bar(state);
            show_left_pane(state, re_size);
            show_main_container(state, re_size);
            show_right_pane(state, re_size);

            should_exit(window);
        }
        ImGui::End();
    }
} // namespace ex::gui
