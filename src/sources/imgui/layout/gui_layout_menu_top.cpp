//
// Created by xd on 02/12/24.
//

#include "../../../headers/imgui/actions/gui_actions_registry.h"
#include "../../../headers/imgui/actions/gui_actions.h"
#include "../../../headers/imgui/gui_globals.h"
#include "../../../headers/imgui/gui_strings.h"
#include "../../../headers/imgui/layout/gui_layout.h"
#include "../../../headers/logic/logic_globals.h"
#include "../../../headers/utils/gui_utils.h"

#include <IconsLucide.h>
#include <simpfp.h>

namespace ex::gui {

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
        const State &state = ex::logic::globals::state();
        GuiState &gui = ex::gui::state();

        if (!gui.view.show_menu)
            return;

        if (ImGui::BeginMenuBar()) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));

            const auto &[file, history, work_tree, layout] = key_mappings();

            if (ImGui::BeginMenu(strings::menu_top_file)) {

                if (ImGui::MenuItem(strings::menu_top_file_new, file.create.label)) {
                    actions::new_action();
                }

                ImGui::Separator();

                if (ImGui::MenuItem(strings::menu_top_file_open, file.open.label)) {
                    actions::open_action();
                }

                if (ImGui::MenuItem(strings::menu_top_file_save, file.save.label, false, state.project.has_changes)) {
                    actions::save_action();
                }

                if (ImGui::MenuItem(strings::menu_top_file_save_as, file.save_as.label, false, state.project.has_changes)) {
                    actions::save_as_action();
                }

                ImGui::Separator();

                if (ImGui::MenuItem(strings::menu_top_file_exit, file.exit.label)) {
                    actions::exit_action();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu(strings::menu_top_edit)) {

                if (ImGui::MenuItem(strings::menu_top_edit_undo, history.undo.label, false, undo_history().can_undo())) {
                    actions::undo_action();
                }

                if (ImGui::MenuItem(strings::menu_top_edit_redo, history.redo.label, false, undo_history().can_redo())) {
                    actions::redo_action();
                }

                ImGui::Separator();

                const auto &cut_action = actions::registry::get_action(work_tree.cut.key);
                if (ImGui::MenuItem(strings::menu_top_edit_cut, work_tree.cut.label, false, cut_action != nullptr)) {
                    cut_action();
                }

                const auto &copy_action = actions::registry::get_action(work_tree.copy.key);
                if (ImGui::MenuItem(strings::menu_top_edit_copy, work_tree.copy.label, false, copy_action != nullptr)) {
                    copy_action();
                }

                const auto &paste_action = actions::registry::get_action(work_tree.paste.key);
                if (ImGui::MenuItem(strings::menu_top_edit_paste, work_tree.paste.label, false, paste_action != nullptr)) {
                    paste_action();
                }

                const auto &insert_action = actions::registry::get_action(work_tree.insert.key);
                if (ImGui::MenuItem(strings::menu_top_edit_insert, work_tree.insert.label, false, insert_action != nullptr)) {
                    insert_action();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu(strings::menu_top_view)) {

                bool docked = gui.view.docking;
                if (ImGui::MenuItem(strings::menu_top_view_unpin, layout.pin.label, &docked)) {
                    actions::pin_unpin_layout_action();
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
                    actions::about_action();
                }

                ImGui::EndMenu();
            }

            end_pin_button_(gui);
            ImGui::PopStyleVar();
            ImGui::EndMenuBar();
        }
    }

}