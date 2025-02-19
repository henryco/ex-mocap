//
// Created by henryco on 17/02/25.
//

#include "../../../headers/imgui/actions/gui_actions.h"
#include "../../../headers/imgui/gui_globals.h"
#include "../../../headers/imgui/gui_strings.h"
#include "../../../headers/logic/logic_globals.h"
#include "../../../headers/utils/gui_utils.h"
#include <filesystem>

namespace ex::gui::actions {

    namespace dialogs_ {
        static bool show_new_dialog  = false;
        static bool show_open_dialog = false;
        static bool show_exit_dialog = false;
    }

    void new_anyway_action() {
        ex::gui::state().file_dialogs.project_new = true;
    }

    void open_anyway_action() {
        ex::gui::state().file_dialogs.project_open = true;
    }

    void exit_anyway_action() {
        ex::gui::state().window.exit = true;
    }

    void new_action() {
        dialogs_::show_new_dialog = true;
    }

    void open_action() {
        if (ex::logic::globals::state().project.has_changes)
            dialogs_::show_open_dialog = true;
        else
            open_anyway_action();
    }

    void save_action() {
        State &state = ex::logic::globals::state();
        GuiState &gui = ex::gui::state();

        if (!state.project.has_changes)
            return;

        if (state.project.project_path == nullptr) {
            gui.file_dialogs.project_save = true;
            return;
        }

        const std::filesystem::path path = state.project.project_path;
        if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
            state.project.should_save = true;
            return;
        }

        gui.file_dialogs.project_save = true;
    }

    void save_as_action() {
        const State &state = ex::logic::globals::state();
        GuiState &gui = ex::gui::state();
        if (state.project.has_changes)
            gui.file_dialogs.project_save = true;
    }

    void undo_action() {
        if (!undo_history().can_undo())
            return;
        undo_history().undo();
        undo_history().apply(&ex::logic::globals::state());
    }

    void redo_action() {
        if (!undo_history().can_redo())
            return;
        undo_history().redo();
        undo_history().apply(&ex::logic::globals::state());
    }

    void pin_unpin_layout_action() {
        GuiState &gui      = ex::gui::state();
        gui.view.docking   = !gui.view.docking;
        gui.window.resized = true;
    }

    void about_action() {
        // TODO
    }

    void exit_action() {
        dialogs_::show_exit_dialog = true;
    }

    void show_action_dialogs() {
        if (dialogs_::show_new_dialog) {
            ImGui::OpenPopup(strings::gui_dialogs_project_new_alert);
            dialogs_::show_new_dialog = false;
        }

        if (dialogs_::show_open_dialog) {
            ImGui::OpenPopup(strings::gui_dialogs_project_open_alert);
            dialogs_::show_open_dialog = false;
        }

        if (dialogs_::show_exit_dialog) {
            ImGui::OpenPopup(strings::gui_dialogs_project_exit_alert);
            dialogs_::show_exit_dialog = false;
        }

        if (show_yes_no_dialog(strings::gui_dialogs_project_open_alert, strings::gui_dialogs_project_open_alert_msg,
                                strings::gui_dialogs_project_open_alert_yes,
                                strings::gui_dialogs_project_open_alert_no)) {
            open_anyway_action();
            return;
        }

        if (show_yes_no_dialog(strings::gui_dialogs_project_new_alert, strings::gui_dialogs_project_new_alert_msg,
                                strings::gui_dialogs_project_new_alert_yes,
                                strings::gui_dialogs_project_new_alert_no)) {
            new_anyway_action();
            return;
        }

        if (show_yes_no_dialog(strings::gui_dialogs_project_exit_alert, strings::gui_dialogs_project_exit_alert_msg,
                                strings::gui_dialogs_project_exit_alert_yes,
                                strings::gui_dialogs_project_exit_alert_no)) {
            exit_anyway_action();
            return;
        }
    }

}