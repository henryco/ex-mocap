//
// Created by xd on 02/12/24.
//

#include "../../../headers/imgui/gui_strings.h"
#include "../../../headers/imgui/gui_actions.h"
#include "../../../headers/imgui/gui_globals.h"
#include "../../../headers/imgui/layout/gui_layout.h"
#include "../../../headers/logic/logic_globals.h"

#include <IconsLucide.h>
#include <simpfp.h>

namespace ex::gui {

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

    static void exit_action_() {
        ex::gui::state().window.exit = true;
    }

    inline void register_actions_() {
        static bool init_actions = true;
        if (init_actions) {
            const auto &[file, history, work_tree] = key_mappings();
            ex::gui::actions::set_action(history.undo.key, &undo_action_);
            ex::gui::actions::set_action(history.redo.key, &redo_action_);
            ex::gui::actions::set_action(file.exit.key, &exit_action_);
            init_actions = false;
        }
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

    void show_menu_top_bar() {
        register_actions_();

        GuiState &gui = ex::gui::state();
        if (!gui.view.show_menu)
            return;

        if (ImGui::BeginMenuBar()) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));

            const auto &[file, history, work_tree] = key_mappings();

            if (ImGui::BeginMenu(strings::menu_top_file)) {
                if (ImGui::MenuItem(strings::menu_top_file_new)) {
                    // TODO
                }

                ImGui::Separator();

                if (ImGui::MenuItem(strings::menu_top_file_open, file.open.label)) {
                    gui.file_dialogs.project_open = true;
                }

                if (ImGui::MenuItem(strings::menu_top_file_save, file.save.label)) {
                    gui.file_dialogs.project_save = true;
                    // TODO
                }

                if (ImGui::MenuItem(strings::menu_top_file_save_as, file.save_as.label)) {
                    gui.file_dialogs.project_save = true;
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
                    // TODO
                }
                ImGui::EndMenu();
            }

            end_pin_button_(gui);
            ImGui::PopStyleVar();
            ImGui::EndMenuBar();
        }
    }

}