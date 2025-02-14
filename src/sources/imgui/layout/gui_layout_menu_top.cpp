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
        State &state = ex::logic::globals::state();
        undo_history().undo();
        undo_history().apply(&state);
    }

    static void redo_action_() {
        State &state = ex::logic::globals::state();
        undo_history().redo();
        undo_history().apply(&state);
    }

    inline void register_actions_(const int &undo_key, const int &redo_key) {
        static bool init_undo = true;
        if (init_undo) {
            ex::gui::actions::set_action(undo_key, &undo_action_);
            ex::gui::actions::set_action(redo_key, &redo_action_);
            init_undo = false;
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
        GuiState &gui = ex::gui::state();

        const auto &[file, history, work_tree] = key_mappings();

        register_actions_(history.undo.key, history.redo.key);

        if (!gui.view.show_menu)
            return;

        if (ImGui::BeginMenuBar()) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));

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
                    gui.window.exit = true;
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

            end_pin_button_(gui);
            ImGui::PopStyleVar();
            ImGui::EndMenuBar();
        }
    }

}