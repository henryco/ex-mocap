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

    void show_save_file_picker_(bool *open, ex::State &state) {
        if (!open) return;

        constexpr auto min_size  = ImVec2(900, 500);
        static constexpr simpfp::Labels labels {
            .main_accept = strings::gui_dialogs_project_save_accept
        };

        if (!simpfp::FileDialogOpen())
            simpfp::OpenFileDialog(strings::gui_dialogs_project_save, state.project_path, nullptr, &labels);

        if (simpfp::ShowFileDialog(strings::gui_dialogs_project_save, open, min_size, true)) {
            if (static char file[256]; simpfp::FileAccepted(file, sizeof(file))) {
                state.project_path = file;
                state.should_save  = true;
                *open              = false;

                // TODO: ASK DIALOG
                simpfp::CloseFileDialog();
            }
            simpfp::EndFileDialog();
        }
    }

    void show_open_file_picker_(bool *open, ex::State &state) {
        if (!open) return;

        constexpr auto min_size  = ImVec2(900, 500);
        static constexpr simpfp::Labels labels {
            .main_accept = strings::gui_dialogs_project_open_accept
        };

        if (!simpfp::FileDialogOpen())
            simpfp::OpenFileDialog(strings::gui_dialogs_project_open, state.project_path, nullptr, &labels);

        if (simpfp::ShowFileDialog(strings::gui_dialogs_project_open, open, min_size, true)) {
            if (static char file[256]; simpfp::FileAccepted(file, sizeof(file))) {
                const auto path = std::filesystem::path(file);
                if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
                    // TODO: ASK DIALOG

                    undo_history().reset();
                    state.title        = (new std::string("ExMoCap - " + path.filename().string()))->c_str();
                    state.project_path = file;
                    state.should_open  = true;
                    *open              = false;

                    ex::gl::set_window_title(state.window, state.title);

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