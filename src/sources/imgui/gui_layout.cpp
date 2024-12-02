//
// Created by xd on 24/11/24.
//

#include "../../headers/imgui/gui_layout.h"
#include "../../headers/utils/gui_utils.h"
#include "../../headers/utils/log_utils.h"

#include <imgui_internal.h>

#define cond_node() (gui.view.reset ? ImGuiCond_Always : ImGuiCond_Once)

namespace ex::gui {

    inline float left_offset(GuiState &gui) { return gui.view.show_left ? (gui.view.pane_width_left + offset_main_h) : 0; }

    inline float right_offset(GuiState &gui) { return gui.view.show_right ? (gui.view.pane_width_right + offset_main_h) : 0; }

    inline float bottom_offset(GuiState &gui) { return gui.view.show_logs ? (offset_main_v) : 0; }

    inline float logs_expanded_offset(GuiState &gui) { return gui.view.expanded_logs ? (gui.view.pane_height_bottom + offset_logs_v) : 0; }

    inline ImVec2 main_size(GuiState &gui) { return { gui.view.main_free_x - left_offset(gui) - right_offset(gui), 0 }; }

    inline ImVec2 content_size(GuiState &gui) { return { 0, gui.view.main_free_y - bottom_offset(gui) - logs_expanded_offset(gui)}; }

    inline int menu_flag(GuiState &gui) { return gui.view.show_menu ? ImGuiWindowFlags_MenuBar : 0; }

    inline int reset_flag(GuiState &gui, const bool standalone = false) {
        return gui.view.reset ? (ImGuiWindowFlags_NoSavedSettings
                             | (standalone ? ImGuiWindowFlags_AlwaysAutoResize : 0))
                          : 0;
    }

    int dock_host_flag(GuiState &gui) {
        return 0
               | ((gui.view.docking || gui.view.first_run) ? 0 : ImGuiDockNodeFlags_NoUndocking)
#ifndef EXM_DISABLE_EXPERIMENTAL
               | ((!gui.view.docking && !gui.view.first_run) ? ImGuiDockNodeFlags_NoWindowMenuButton : 0)
               | ((gui.view.docking || gui.view.first_run) ? 0 : ImGuiDockNodeFlags_NoDocking)
               | ImGuiDockNodeFlags_DockedWindowsInFocusRoute
#endif
        ;
    }

    int content_resize_flag(GuiState &gui) {
        static bool content_resize_flag_first = true;
        if (!gui.view.expanded_logs) {
            content_resize_flag_first = true;
            return 1;
        }
        if (content_resize_flag_first) {
            content_resize_flag_first = false;
            return 1;
        }
        return ImGuiChildFlags_ResizeY;
    }

    bool *watch_parent_dock(GuiState &gui) {
        if (ex::gui::window_withing_dock(gui.docking.dock_left_id))
            return &gui.view.show_left;
        if (ex::gui::window_withing_dock(gui.docking.dock_right_id))
            return &gui.view.show_right;
        return nullptr;
    }

    void show_work_tree(ex::State &state, GuiState &gui) {
        static bool *watch = nullptr;
        if (watch != nullptr && !(*watch))
            return;

        static bool docked = false;

        ex::gui::next_auto_hide(docked || gui.view.docking);
        ImGui::SetNextWindowDockID(gui.docking.dock_left_top_id, cond_node());
        ImGui::Begin("Work tree", nullptr, reset_flag(gui) | ImGuiWindowFlags_NoCollapse);
        docked = ex::gui::window_is_multi_docked();
        watch = watch_parent_dock(gui);

        ImGui::Text("This window 1: %d | %d", docked, gui.view.docking);
        ImGui::End();
    }

    void show_workspace_switcher(ex::State &state, GuiState &gui) {
        static bool *watch = nullptr;
        if (watch != nullptr && !(*watch))
            return;

        static bool docked = false;
        ex::gui::next_auto_hide(docked || gui.view.docking);
        ImGui::SetNextWindowDockID(gui.docking.dock_left_bot_id, cond_node());
        ImGui::Begin("Workspace", nullptr, reset_flag(gui) | ImGuiWindowFlags_NoCollapse);
        docked = ex::gui::window_is_multi_docked();
        watch = watch_parent_dock(gui);

        ImGui::Text("This window 2: %d | %d", docked, gui.view.docking);
        ImGui::End();
    }

    void show_floating_window(ex::State &state, GuiState &gui) {

        static bool docked = false;
        static bool show = true;

        if (!show)
            return;

        static bool *watch = nullptr;
        if (watch != nullptr && !(*watch))
            return;

        bool *ptr = (!docked || gui.view.docking) ? &show : nullptr;

        ex::gui::next_no_close();
        ImGui::Begin("My Window3", ptr, reset_flag(gui, true) | ImGuiWindowFlags_NoCollapse);
        docked = ex::gui::window_is_docked();
        watch = watch_parent_dock(gui);
        ImGui::Text("This window 3: %d | %d", docked, gui.view.docking);
        ImGui::End();
    }

    void show_left_dock_space(ex::State &state, GuiState &gui) {
        static bool first_time = true;

        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.f, 0.f, 0.f, .0f));
        ImGui::PushStyleColor(ImGuiCol_TabSelected, ImVec4(0.f, 0.f, 0.f, .0));
        ImGui::PushStyleColor(ImGuiCol_TabDimmedSelected, ImVec4(0.f, 0.f, 0.f, .0f));

        const int flags = (ex::gui::is_docking_empty(gui.docking.dock_left_id))
                        ? ImGuiDockNodeFlags_NoDockingSplit
                        : 0;

        const int aux_flags = ImGuiDockNodeFlags_PassthruCentralNode
                              | dock_host_flag(gui);

        gui.docking.dock_left_id = ImGui::GetID("DockMainLeft");
        ImGui::DockSpace(gui.docking.dock_left_id,
                         ImVec2(0.0f, 0.0f),
                         ImGuiDockNodeFlags_None
                                 | aux_flags
                                 | flags
        );

        if (first_time || gui.view.reset) {
            first_time = false;
            ImGui::DockBuilderRemoveNode(gui.docking.dock_left_id);
            ImGui::DockBuilderAddNode(gui.docking.dock_left_id,
                                      ImGuiDockNodeFlags_None
                                      | aux_flags
                                      );
            ImGui::DockBuilderSplitNode(gui.docking.dock_left_id,
                                        ImGuiDir_Up,
                                        0.7f,
                                        &gui.docking.dock_left_top_id,
                                        &gui.docking.dock_left_bot_id);

            if (const auto node_1 = ImGui::DockBuilderGetNode(gui.docking.dock_left_top_id))
                node_1->SetLocalFlags(aux_flags);

            if (const auto node_2 = ImGui::DockBuilderGetNode(gui.docking.dock_left_bot_id))
                node_2->SetLocalFlags(aux_flags);

            ImGui::DockBuilderFinish(gui.docking.dock_left_id);
        }

        ImGui::PopStyleColor(3);
    }

    void show_left_pane(ex::State &state, ReSize &re_size, GuiState &gui) {
        if (!gui.view.show_left)
            return;
        const auto old_width = gui.view.pane_width_left;
        ImGui::BeginChild("LeftPane",
                          ImVec2(gui.view.pane_width_left, 0),
                          0
                          | ImGuiChildFlags_Borders
                          | ImGuiChildFlags_ResizeX,
                          reset_flag(gui));

        {
            ImGui::Text("%d", ex::gui::is_docking_empty(gui.docking.dock_left_id));
            // TODO
            ImGui::Separator();
            show_left_dock_space(state, gui);
        }

        gui.view.pane_width_left = ImGui::GetWindowWidth();
        ImGui::EndChild();
        ImGui::SameLine();

        re_size.inc_x = (!re_size.inc_x) && (old_width != gui.view.pane_width_left);
    }

    void show_right_dock_space(ex::State &state, GuiState &gui) {
        static bool first_time = true;

        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.f, 0.f, 0.f, .0f));
        ImGui::PushStyleColor(ImGuiCol_TabSelected, ImVec4(0.f, 0.f, 0.f, .0));
        ImGui::PushStyleColor(ImGuiCol_TabDimmedSelected, ImVec4(0.f, 0.f, 0.f, .0f));

        const int flags = (first_time || ex::gui::is_docking_empty(gui.docking.dock_right_id))
                ? ImGuiDockNodeFlags_NoDockingSplit
                : 0;

        const int aux_flags = ImGuiDockNodeFlags_PassthruCentralNode
                | dock_host_flag(gui);

        gui.docking.dock_right_id = ImGui::GetID("DockMainRight");
        ImGui::DockSpace(gui.docking.dock_right_id,
                         ImVec2(0.0f, 0.0f),
                         ImGuiDockNodeFlags_None
                                 | aux_flags
                                 | flags
        );

        if (first_time || gui.view.reset) {
            first_time = false;
            // TODO
        }

        ImGui::PopStyleColor(3);
    }

    void show_right_pane(ex::State &state, ReSize &re_size, GuiState &gui) {
        if (!gui.view.show_right)
            return;
        const auto old_width = gui.view.pane_width_right;
        ImGui::SameLine();
        ImGui::BeginChild("RightPane",
                          ImVec2((re_size.inc_x || gui.window.window_resized) ? 0 : gui.view.pane_width_right, 0),
                          0
                          | ImGuiChildFlags_Borders,
                          reset_flag(gui));

        {
            ImGui::Text("%d", ex::gui::is_docking_empty(gui.docking.dock_right_id));
            // TODO
            ImGui::Separator();
            show_right_dock_space(state, gui);
        }

        gui.view.pane_width_right = ImGui::GetWindowWidth();
        ImGui::EndChild();

        re_size.inc_x = (!re_size.inc_x) && (old_width != gui.view.pane_width_right);
    }

    void show_content(ex::State &state, ReSize &re_size, GuiState &gui) {
        const auto old_height = gui.view.pane_height_content;

        ImGui::BeginChild("ContentBox",
                          content_size(gui),
                          0
                                  | ImGuiChildFlags_Borders
                                  | content_resize_flag(gui)
                                  | ((re_size.inc_y || gui.window.window_resized) ? ImGuiChildFlags_AutoResizeY : 0),
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

    void show_logs_pane(ex::State &state, ReSize &re_size, GuiState &gui) {
        if (!gui.view.show_logs)
            return;
        const auto old_height = gui.view.pane_height_bottom;
        const auto old_expanded = gui.view.expanded_logs;

        gui.view.expanded_logs = ImGui::CollapsingHeader(ICON_LC_LOGS " Log Output ", &gui.view.show_logs);
        if (!gui.view.show_logs) {
            gui.view.expanded_logs = false;
            gui.window.window_resized = true;
        }

        if (gui.view.expanded_logs) {
            ImGui::BeginChild("LogControlsPane",
                ImVec2(24, 0), 0, ImGuiWindowFlags_NoDecoration | reset_flag(gui));
            {
                gui.view.highlight_logs = !ex::gui::toggle_button(ICON_LC_LIGHTBULB_OFF, !gui.view.highlight_logs);
                ex::gui::plain_tooltip("Disable highlighting for warnings and errors");
                ImGui::Spacing();

                gui.view.follow_logs = ex::gui::toggle_button(ICON_LC_LIST_END, gui.view.follow_logs);
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
                              ImVec2(0, (re_size.inc_y || gui.window.window_resized) ? 0 : gui.view.pane_height_bottom),
                              0
                              | ImGuiChildFlags_Borders, reset_flag(gui));

            ImGui::BeginChild("LogPane", ImVec2(0, 0), ImGuiChildFlags_FrameStyle, reset_flag(gui));
            {
                for (const auto &log : ex::logs::get_all_logs_ext()) {
                    const auto &message = std::get<0>(log);
                    const auto &level = std::get<1>(log);

                    if (gui.view.highlight_logs) {
                        if (level == spdlog::level::warn) {
                            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 140, 40, 255));
                            ImGui::TextWrapped("%s", message.c_str());
                            ImGui::PopStyleColor();
                            continue;
                        }
                        if (level == spdlog::level::err || level == spdlog::level::critical) {
                            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(220, 20, 60, 255));
                            ImGui::TextWrapped("%s", message.c_str());
                            ImGui::PopStyleColor();
                            continue;
                        }
                    }

                    ImGui::TextWrapped("%s", message.c_str());
                }
                if (gui.view.follow_logs || !old_expanded) {
                    ImGui::SetScrollY(ImGui::GetScrollMaxY());
                }
            }
            ImGui::EndChild();

            gui.view.pane_height_bottom = ImGui::GetWindowHeight();
            ImGui::EndChild();
            ImGui::PopStyleVar();
        }
        re_size.inc_y = (!re_size.inc_y) && (old_height != gui.view.pane_height_bottom);
    }

    void show_main_container(ex::State &state, ReSize &re_size, GuiState &gui) {
        const auto old_width = gui.view.main_width;
        const auto old_height = gui.view.main_height;

        ImGui::BeginChild("MainContainer",
                          main_size(gui),
                          0
                                  | (gui.view.show_right ? ImGuiChildFlags_ResizeX : 0)
                                  | ((re_size.inc_x || gui.window.window_resized) ? ImGuiChildFlags_AutoResizeX : 0)
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

    bool is_window_resized() {
        static ImVec2 iwr_last_size = {};
        const auto current_size = ImGui::GetContentRegionAvail();
        if (current_size.x != iwr_last_size.x || current_size.y != iwr_last_size.y) {
            iwr_last_size = current_size;
            return true;
        }
        return false;
    }

    void check_for_reset(GuiState &gui) {
        if (!gui.window.reset_layout)
            return;
        gui.window.reset_layout = false;
        gui.window.window_resized = true;
        gui.view = {};
        gui.view.reset = true;
    }

    bool layout::build(ex::State &state) {
        static GuiState gui;

        ReSize re_size = {};

        ImGui::GetStyle().TabRounding = 0.0f;

        ImGui::Begin("main", nullptr,
                     ImGuiWindowFlags_NoDecoration
                             | ImGuiWindowFlags_NoDocking
                             | ImGuiWindowFlags_NoBringToFrontOnFocus
                             | menu_flag(gui)
                             | reset_flag(gui));
        {
            gui.window.window_resized = is_window_resized();

            check_for_reset(gui);

            gui.view.main_free_x = ImGui::GetContentRegionAvail().x;
            gui.view.main_free_y = ImGui::GetContentRegionAvail().y;

            show_menu_top_bar(state, gui);
            show_left_pane(state, re_size, gui);
            show_main_container(state, re_size, gui);
            show_right_pane(state, re_size, gui);

            show_work_tree(state, gui);
            show_workspace_switcher(state, gui);
            show_floating_window(state, gui);
        }
        ImGui::End();

        gui.view.first_run = false;
        if (gui.view.reset) {
            gui.view.reset = false;
        }

        return gui.view.should_exit;
    }
} // namespace ex::gui
