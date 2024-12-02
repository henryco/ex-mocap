//
// Created by xd on 02/12/24.
//

#include "../../../headers/imgui/gui_layout.h"
#include "../../../headers/utils/log_utils.h"

namespace ex::gui {

    void show_logs_pane(ex::State &state, ReSize &re_size, GuiState &gui) {
        if (!gui.view.show_logs)
            return;
        const auto old_height = gui.view.pane_height_bottom;
        const auto old_expanded = gui.view.expanded_logs;

        gui.view.expanded_logs = ImGui::CollapsingHeader(ICON_LC_LOGS " Log Output ", &gui.view.show_logs);
        if (!gui.view.show_logs) {
            gui.view.expanded_logs = false;
            gui.window.resized     = true;
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
                              ImVec2(0, (re_size.inc_y || gui.window.resized) ? 0 : gui.view.pane_height_bottom),
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


}