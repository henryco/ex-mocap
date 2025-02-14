//
// Created by xd on 02/12/24.
//

#include "../../../headers/imgui/gui_strings.h"
#include "../../../headers/imgui/gui_globals.h"
#include "../../../headers/imgui/layout/gui_layout.h"
#include "../../../headers/logic/logic_globals.h"
#include "../../../headers/utils/gl_utils.h"

#include <IconsLucide.h>
#include <simpfp.h>

namespace ex::gui {

    void end_pin_button(GuiState &gui) {
        const auto icon = gui.view.docking ? ICON_LC_PIN_OFF : ICON_LC_PIN;
        ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize(icon).x
                - ImGui::GetStyle().FramePadding.x * 3 - ImGui::GetStyle().ItemSpacing.x * 1);
        if (ex::gui::plain_button(icon)) {
            gui.view.docking = !gui.view.docking;
            gui.window.resized = true;
            logger()->debug("unpin windows: {}", gui.view.docking);
        }
        ex::gui::plain_tooltip(strings::menu_top_view_unpin);
    }

    void show_save_file_picker(bool *open, ex::State &state) {
        if (!open) return;

        constexpr auto min_size  = ImVec2(900, 500);
        static constexpr simpfp::Labels labels {
            .main_accept = "Save"
        };

        if (!simpfp::FileDialogOpen())
            simpfp::OpenFileDialog("Save a file", state.project_path, nullptr, &labels);

        if (simpfp::ShowFileDialog("Save a file", open, min_size, true)) {
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

    void show_open_file_picker(bool *open, ex::State &state) {
        if (!open) return;

        constexpr auto min_size  = ImVec2(900, 500);
        static constexpr simpfp::Labels labels {
            .main_accept = "Open"
        };

        if (!simpfp::FileDialogOpen())
            simpfp::OpenFileDialog("Open a file", state.project_path, nullptr, &labels);

        if (simpfp::ShowFileDialog("Open a file", open, min_size, true)) {
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

    void show_menu_top_bar() {
        State    &state = ex::logic::globals::state();
        GuiState &gui = ex::gui::state();

        static bool file_picker_save = false;
        static bool file_picker_open = false;

        show_open_file_picker(&file_picker_open, state);
        show_save_file_picker(&file_picker_save, state);

        if (!gui.view.show_menu)
            return;

        if (ImGui::BeginMenuBar()) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));

            if (ImGui::BeginMenu(strings::menu_top_file)) {
                if (ImGui::MenuItem(strings::menu_top_file_new)) {
                    // TODO
                }

                if (ImGui::MenuItem(strings::menu_top_file_open)) {
                    file_picker_open = true;
                }

                if (ImGui::MenuItem(strings::menu_top_file_save)) {
                    file_picker_save = true;
                    // TODO
                }

                if (ImGui::MenuItem(strings::menu_top_file_save_as)) {
                    file_picker_save = true;
                }

                ImGui::Separator();

                if (ImGui::MenuItem(strings::menu_top_file_exit)) {
                    gui.window.exit = true;
                }

                ImGui::EndMenu();
            }

            const auto &[history, work_tree] = key_mappings();
            if (ImGui::BeginMenu(strings::menu_top_edit)) {

                if (ImGui::MenuItem(strings::menu_top_edit_undo, history.undo.label, false, undo_history().can_undo())) {
                    undo_history().undo();
                    undo_history().apply(&state);
                }
                if (ImGui::MenuItem(strings::menu_top_edit_redo, history.redo.label, false, undo_history().can_redo())) {
                    undo_history().redo();
                    undo_history().apply(&state);
                }

                ImGui::Separator();

                if (ImGui::MenuItem(strings::menu_top_edit_cut, work_tree.cut.label, false, false)) {
                    // TODO
                }
                if (ImGui::MenuItem(strings::menu_top_edit_copy, work_tree.copy.label, false, false)) {
                    // TODO
                }
                if (ImGui::MenuItem(strings::menu_top_edit_paste, work_tree.paste.label, false, false)) {
                    // TODO
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu(strings::menu_top_view)) {
                if (ImGui::MenuItem(strings::menu_top_view_unpin, nullptr, &gui.view.docking)) {
                    gui.window.resized = true;
                    logger()->debug("unpin windows: {}", gui.view.docking);
                }
                ImGui::Separator();

                if (ImGui::MenuItem(strings::menu_top_view_sidebar_left, nullptr, &gui.view.show_left)) {
                    gui.window.resized = true;
                }

                if (ImGui::MenuItem(strings::menu_top_view_sidebar_right, nullptr, &gui.view.show_right)) {
                    gui.window.resized = true;
                }

                if (ImGui::MenuItem(strings::menu_top_view_logs, nullptr, &gui.view.show_logs)) {
                    gui.view.expanded_logs = gui.view.show_logs;
                    gui.window.resized     = true;
                }

                if (ImGui::MenuItem(strings::menu_top_view_metrics, nullptr, &gui.view.show_debug)) {
                    gui.window.resized = true;
                }

                ImGui::Separator();

                if (ImGui::MenuItem(strings::menu_top_view_reset_layout)) {
                    gui.window.reset        = true;
                    gui.window.resized      = true;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu(strings::menu_top_help)) {
                if (ImGui::MenuItem(strings::menu_top_help_about)) {
                    // TODO
                }
                ImGui::EndMenu();
            }

            end_pin_button(gui);
            ImGui::PopStyleVar();
            ImGui::EndMenuBar();
        }
    }

}