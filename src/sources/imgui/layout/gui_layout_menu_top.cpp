//
// Created by xd on 02/12/24.
//

#include "../../../headers/utils/gui_utils.h"
#include "../../../headers/imgui/gui_strings.h"
#include "../../../headers/imgui/gui_actions.h"
#include "../../../headers/imgui/gui_globals.h"
#include "../../../headers/imgui/layout/gui_layout.h"
#include "../../../headers/logic/logic_globals.h"

#include <IconsLucide.h>
#include <simpfp.h>

namespace ex::gui {

    namespace dialogs_ {
        static bool show_open_dialog = false;
    }

    static void new_action() {
        // TODO
    }

    static void open_action() {
        const State &state = ex::logic::globals::state();
        GuiState &gui = ex::gui::state();

        if (state.project.has_changes) {
            dialogs_::show_open_dialog = true;
            return;
        }

        gui.file_dialogs.project_open = true;
    }

    static void save_action() {
        const State &state = ex::logic::globals::state();
        GuiState &gui = ex::gui::state();
        // TODO
        if (state.project.has_changes)
            gui.file_dialogs.project_save = true;
    }

    static void save_as_action() {
        const State &state = ex::logic::globals::state();
        GuiState &gui = ex::gui::state();
        if (state.project.has_changes)
            gui.file_dialogs.project_save = true;
    }

    static void undo_action_() {
        if (!undo_history().can_undo())
            return;
        undo_history().undo();
        undo_history().apply(&ex::logic::globals::state());
    }

    static void redo_action_() {
        if (!undo_history().can_redo())
            return;
        undo_history().redo();
        undo_history().apply(&ex::logic::globals::state());
    }

    static void about_action() {
        // TODO
    }

    static void exit_action_() {
        const State &state = ex::logic::globals::state();
        if (state.project.has_changes) {
            // TODO DIALOG?
        }
        ex::gui::state().window.exit = true;
    }

    inline void register_actions_() {
        static const auto &[file, history, work_tree] = key_mappings();
        ex::gui::actions::set_action(file.open.key, &open_action);
        ex::gui::actions::set_action(file.save.key, &save_action);
        ex::gui::actions::set_action(file.save_as.key, &save_as_action);
        ex::gui::actions::set_action(history.undo.key, &undo_action_);
        ex::gui::actions::set_action(history.redo.key, &redo_action_);
        ex::gui::actions::set_action(file.exit.key, &exit_action_);
    }

    void end_pin_button_(GuiState &gui) {
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

    void process_dialogs_() {
        GuiState &gui = ex::gui::state();

        if (dialogs_::show_open_dialog) {
            ImGui::OpenPopup(strings::gui_dialogs_project_open_alert);
            dialogs_::show_open_dialog = false;
        }

        if (show_yes_no_dialog_(strings::gui_dialogs_project_open_alert,
            strings::gui_dialogs_project_open_alert_msg,
            strings::gui_dialogs_project_open_alert_yes,
            strings::gui_dialogs_project_open_alert_no)) {
            gui.file_dialogs.project_open = true;
            return;
        }

    }

    void show_menu_top_bar() {
        const State &state = ex::logic::globals::state();
        GuiState &gui = ex::gui::state();

        process_dialogs_();
        register_actions_();

        if (!gui.view.show_menu)
            return;

        if (ImGui::BeginMenuBar()) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));

            const auto &[file, history, work_tree] = key_mappings();

            if (ImGui::BeginMenu(strings::menu_top_file)) {
                if (ImGui::MenuItem(strings::menu_top_file_new)) {
                    new_action();
                }

                ImGui::Separator();

                if (ImGui::MenuItem(strings::menu_top_file_open, file.open.label)) {
                    open_action();
                }

                if (ImGui::MenuItem(strings::menu_top_file_save, file.save.label, false, state.project.has_changes)) {
                    save_action();
                }

                if (ImGui::MenuItem(strings::menu_top_file_save_as, file.save_as.label, false, state.project.has_changes)) {
                    save_as_action();
                }

                ImGui::Separator();

                if (ImGui::MenuItem(strings::menu_top_file_exit, file.exit.label)) {
                    exit_action_();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu(strings::menu_top_edit)) {

                if (ImGui::MenuItem(strings::menu_top_edit_undo, history.undo.label, false, undo_history().can_undo())) {
                    undo_action_();
                }
                if (ImGui::MenuItem(strings::menu_top_edit_redo, history.redo.label, false, undo_history().can_redo())) {
                    redo_action_();
                }

                ImGui::Separator();

                const auto &cut_action = actions::get_action(work_tree.cut.key);
                if (ImGui::MenuItem(strings::menu_top_edit_cut, work_tree.cut.label, false, cut_action != nullptr)) {
                    cut_action();
                }

                const auto &copy_action = actions::get_action(work_tree.copy.key);
                if (ImGui::MenuItem(strings::menu_top_edit_copy, work_tree.copy.label, false, copy_action != nullptr)) {
                    copy_action();
                }

                const auto &paste_action = actions::get_action(work_tree.paste.key);
                if (ImGui::MenuItem(strings::menu_top_edit_paste, work_tree.paste.label, false, paste_action != nullptr)) {
                    paste_action();
                }

                const auto &insert_action = actions::get_action(work_tree.insert.key);
                if (ImGui::MenuItem(strings::menu_top_edit_insert, work_tree.insert.label, false, insert_action != nullptr)) {
                    insert_action();
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
                    about_action();
                }
                ImGui::EndMenu();
            }

            end_pin_button_(gui);
            ImGui::PopStyleVar();
            ImGui::EndMenuBar();
        }
    }

}