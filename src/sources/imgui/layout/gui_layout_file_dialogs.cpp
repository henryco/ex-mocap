//
// Created by xd on 2/14/25.
//

#include "../../../headers/imgui/actions/gui_actions.h"
#include "../../../headers/imgui/layout/gui_layout.h"
#include "../../../headers/imgui/gui_strings.h"
#include "../../../headers/imgui/gui_globals.h"
#include "../../../headers/logic/logic_globals.h"

#include <simpfp.h>

namespace ex::gui {

    void show_save_file_picker_(bool *open, ex::State &state) {
        if (open == nullptr || !*open)
            return;

        constexpr auto min_size  = ImVec2(900, 500);
        static constexpr simpfp::Labels labels {
            .main_accept = strings::gui_dialogs_project_save_accept
        };

        if (!simpfp::FileDialogOpen())
            simpfp::OpenFileDialog(strings::gui_dialogs_project_save, state.project.project_path, nullptr, &labels);

        static char file[256];
        if (simpfp::ShowFileDialog(strings::gui_dialogs_project_save, open, min_size, true)) {

            if (simpfp::FileAccepted(file, sizeof(file))) {
                const std::filesystem::path save_path(file);

                if (std::filesystem::exists(save_path) && std::filesystem::is_regular_file(save_path)) {
                    ImGui::OpenPopup(strings::gui_dialogs_project_save_alert);
                } else {
                    state.project.project_path = file;
                    state.project.should_save  = true;
                    *open                      = false;
                    simpfp::CloseFileDialog();
                }
            }

            if (show_yes_no_dialog(
                        strings::gui_dialogs_project_save_alert, strings::gui_dialogs_project_save_alert_msg,
                        strings::gui_dialogs_project_save_alert_yes, strings::gui_dialogs_project_save_alert_no)) {
                state.project.project_path = file;
                state.project.should_save  = true;
                *open                      = false;
                simpfp::CloseFileDialog();
            }

            simpfp::EndFileDialog();
        }
    }

    void show_open_file_picker_(bool *open, ex::State &state) {
        if (open == nullptr || !*open)
            return;

        constexpr auto min_size  = ImVec2(900, 500);
        static constexpr simpfp::Labels labels {
            .main_accept = strings::gui_dialogs_project_open_accept
        };

        if (!simpfp::FileDialogOpen())
            simpfp::OpenFileDialog(strings::gui_dialogs_project_open, state.project.project_path, nullptr, &labels);

        if (simpfp::ShowFileDialog(strings::gui_dialogs_project_open, open, min_size, true)) {
            if (static char file[256]; simpfp::FileAccepted(file, sizeof(file))) {
                const auto path = std::filesystem::path(file);
                if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {

                    state.project.project_path = file;
                    state.project.should_open  = true;
                    *open                      = false;

                    simpfp::CloseFileDialog();
                } else {
                    ImGui::OpenPopup(strings::gui_dialogs_project_open_error);
                }

                show_alert_dialog(strings::gui_dialogs_alert_error, strings::gui_dialogs_project_open_error_msg,
                                  strings::gui_dialogs_project_open_error_ok);
            }
            simpfp::EndFileDialog();
        }
    }

    void show_new_file_picker_(bool *open, ex::State &state) {
        if (open == nullptr || !*open)
            return;

        constexpr auto                  min_size = ImVec2(900, 500);
        static constexpr simpfp::Labels labels{ .main_accept = strings::gui_dialogs_project_new_accept };

        if (!simpfp::FileDialogOpen())
            simpfp::OpenFileDialog(strings::gui_dialogs_project_new, state.project.project_path, nullptr, &labels);

        static char file[256];
        if (simpfp::ShowFileDialog(strings::gui_dialogs_project_new, open, min_size, true)) {
            if (simpfp::FileAccepted(file, sizeof(file))) {
                const std::filesystem::path new_path(file);
                if (std::filesystem::exists(new_path) && std::filesystem::is_regular_file(new_path)) {
                    ImGui::OpenPopup(strings::gui_dialogs_project_save_alert);
                }
                else {
                    state.project.project_path  = file;
                    state.project.should_create = true;
                    *open                       = false;
                    simpfp::CloseFileDialog();
                }
            }

            if (show_yes_no_dialog(strings::gui_dialogs_project_save_alert, strings::gui_dialogs_project_save_alert_msg,
                                   strings::gui_dialogs_project_save_alert_yes,
                                   strings::gui_dialogs_project_save_alert_no)) {
                state.project.project_path  = file;
                state.project.should_create = true;
                *open                       = false;
                simpfp::CloseFileDialog();
            }

            simpfp::EndFileDialog();
        }
    }

    void show_file_dialogs() {
        State    &state = ex::logic::globals::state();
        GuiState &gui = ex::gui::state();

        actions::show_action_dialogs();
        show_open_file_picker_(&gui.file_dialogs.project_open, state);
        show_save_file_picker_(&gui.file_dialogs.project_save, state);
        show_new_file_picker_(&gui.file_dialogs.project_new, state);
    }

}