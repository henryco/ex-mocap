//
// Created by xd on 2/14/25.
//

#include "../../../headers/imgui/layout/gui_layout.h"
#include "../../../headers/imgui/gui_strings.h"
#include "../../../headers/imgui/gui_globals.h"
#include "../../../headers/logic/logic_globals.h"
#include "../../../headers/utils/gl_utils.h"

#include <simpfp.h>

namespace ex::gui {

    bool show_yes_no_dialog_(const char *title, const char *msg, const char *yes, const char *no) {
        bool result = false;

        const auto offset     = ImGui::GetStyle().FramePadding.x * 2 + ImGui::GetStyle().ItemSpacing.x * 2;
        const auto title_size = ImGui::CalcTextSize(msg).x + offset * 4;

        const auto center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(title_size, 0));
        if (ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("%s", msg);
            ImGui::Spacing();
            ImGui::Spacing();

            const float ok_size = ImGui::CalcTextSize(yes).x + offset;
            const float no_size = ImGui::CalcTextSize(no).x + offset;
            const float free_w  = ImGui::GetContentRegionMax().x;
            const float spacing = ImGui::GetStyle().ItemSpacing.x;

            ImGui::SetCursorPosX(free_w - (ok_size + no_size + spacing));

            if (ImGui::Button(yes, ImVec2(ok_size, 0))) {
                ImGui::CloseCurrentPopup();
                result = true;
            }
            ImGui::SameLine();
            if (ImGui::Button(no, ImVec2(no_size, 0))) {
                ImGui::CloseCurrentPopup();
                result = false;
            }

            ImGui::EndPopup();
        }

        return result;
    }

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
                    ImGui::OpenPopup("Are you sure?");
                } else {
                    state.project.project_path = file;
                    state.project.should_save  = true;
                    *open                      = false;

                    simpfp::CloseFileDialog();
                }
            }

            if (show_yes_no_dialog_("Are you sure?", "This action will overwrite selected files", "Accept", "Cancel")) {
                // TODO
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
                    // TODO: ASK DIALOG

                    undo_history().reset();
                    state.project.title        = (new std::string("ExMoCap - " + path.filename().string()))->c_str();
                    state.project.project_path = file;
                    state.project.should_open  = true;
                    *open                      = false;

                    ex::gl::set_window_title(state.gl.window, state.project.title);

                    simpfp::CloseFileDialog();
                } else {
                    // TODO ERROR DIALOG
                }
            }
            simpfp::EndFileDialog();
        }
    }

    void show_file_dialogs() {
        State    &state = ex::logic::globals::state();
        GuiState &gui = ex::gui::state();

        show_open_file_picker_(&gui.file_dialogs.project_open, state);
        show_save_file_picker_(&gui.file_dialogs.project_save, state);
    }

}