//
// Created by xd on 02/12/24.
//

#include "../../../headers/imgui/gui_layout.h"
#include <IconsLucide.h>

namespace ex::gui {
    constexpr auto flags = ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_BordersInnerV;

    constexpr float tree_spacing = 20.f;
    constexpr float ctrl_width   = 40.f;
    constexpr float min_width    = 100.f;

    bool show_context_menu_element(State &state, GuiState &gui, TreeNode *node,
        const bool last, const bool nested, const bool selected)
    {
        static auto menu_id = "show_context_menu_element";
        bool clicked = false;

        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup(menu_id);
            clicked = true;
        }
        else if (last) {
            ImGui::OpenPopup(menu_id);
        }

        if (ImGui::BeginPopup(menu_id)) {
            const bool enabled = !node->disabled;
            if (ImGui::MenuItem(enabled ? "Disable" : "Enable")) {

                node->disabled = !node->disabled;
                const bool n_state = node->disabled;

                undo_history().push((enabled ? "Disabled node" : "Enabled node"),
                    new std::function([id_l = node->id_local, n_state] (ex::State *s) {
                        const auto f_node = s->work_tree->find_node(id_l);
                        if (f_node == nullptr)
                            return;
                        f_node->disabled = !n_state;
                        f_node->updated = true;
                    }),
                    new std::function([id_l = node->id_local, n_state] (ex::State *s) {
                        const auto f_node = s->work_tree->find_node(id_l);
                        if (f_node == nullptr)
                            return;
                        f_node->disabled = n_state;
                        f_node->updated = true;
                    }));
            }

            node_mappers_.at(node->id_node).draw_tree_menu(gui, state, node, nested, selected);

            if (ImGui::MenuItem("Duplicate")) {
                // TODO DELETE
            }

            if (ImGui::MenuItem("Cut")) {
                // TODO DELETE
            }

            if (ImGui::MenuItem("Copy")) {
                // TODO DELETE
            }

            if (ImGui::MenuItem("Paste Object")) {
                // TODO DELETE
            }

            if (ImGui::MenuItem("Paste Reference")) {
                // TODO DELETE
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Delete")) {
                // TODO DELETE
            }

            ImGui::EndPopup();
        }
        return clicked;
    }

    void show_tree_node(State &state, GuiState &gui, TreeNode *node, std::size_t *selected,
                        TreeNode **active, std::size_t *menu_node, const bool disabled_,
                        const int total, const int n)
    {
        if (!node->is_group) {
            // Not a group just regular element

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            const bool connected = node->connected || n == 0;
            const bool disabled  = node->disabled || !connected;

            if (disabled || disabled_)
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);

            ImGui::PushID(node->id_local);
            const bool open = node_mappers_.at(node->id_node).draw_tree_node(
                gui, state, node, node->children_n > 0, *selected == node->id_local);

            if (disabled || disabled_)
                ImGui::PopStyleVar();

            const bool menu = show_context_menu_element(state, gui, node, *menu_node == node->id_local,
                node->children_n > 0, *selected == node->id_local);

            if (menu) {
                *menu_node = node->id_local;
            }

            if (open && node->children_n > 0)
                ImGui::Indent();

            if (ImGui::IsItemClicked() || menu) {
                *selected = node->id_local;
                *active   = node;
            }

            const auto pos = ImGui::GetCursorScreenPos();

            if (disabled) {
                ImGui::TableNextColumn();
                ImGui::TextDisabled(ICON_LC_LINK_2_OFF);
            }
            else if (ImGui::IsItemHovered()) {
                ImGui::TableNextColumn();
                ImGui::TextDisabled(connected ? "" : ICON_LC_UNPLUG);
            }
            else {
                ImGui::TableNextColumn();
                ImGui::TextDisabled(connected ? (node->children_n > 0 ? ICON_LC_ARROW_DOWN_RIGHT : "") : ICON_LC_UNPLUG);
            }

            if (open && node->children_n > 0) {
                if (disabled || disabled_)
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);

                // Regular element but it has children
                const auto next = node->children[0];
                for (int i = 0; i < next->children_n; i++)
                    show_tree_node(state, gui, next->children[i], selected, active, menu_node,
                        disabled || disabled_, next->children_n, i);
                ImGui::TreePop();

                if (disabled || disabled_)
                    ImGui::PopStyleVar();

                ImGui::Unindent();
                brackets_end_half(pos, 0, disabled || disabled_ ? 0.5f : 1.f);
            }

            ImGui::PopID();
            return;
        }

        // It's a group
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        if (node->children_n <= 0) {
            // Group but it has no children (an empty one)
//
//            const bool connected = node->connected || n == 0;
//            const bool disabled  = node->disabled || !connected;
//
//            if (disabled || disabled_)
//                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);
//
//            ImGui::PushID(node->id_local);
//            node_mappers_.at(node->id_node).draw_tree_node(
//                gui, state, node, false, *selected == node->id_local);
//
//            if (disabled || disabled_)
//                ImGui::PopStyleVar();
//
//            const bool menu = show_context_menu_group(state, gui, node, *menu_node == node->id_local,
//                false, *selected == node->id_local);
//
//            if (menu) {
//                *menu_node = node->id_local;
//            }
//
//            if (ImGui::IsItemClicked() || menu) {
//                *selected = node->id_local;
//                *active   = node;
//            }
//
//            if (disabled) {
//                ImGui::TableNextColumn();
//                ImGui::TextDisabled(ICON_LC_LINK_2_OFF);
//            }
//            else if (ImGui::IsItemHovered()) {
//                ImGui::TableNextColumn();
//                ImGui::TextDisabled(connected ? "" : ICON_LC_UNPLUG);
//            }
//            else {
//                ImGui::TableNextColumn();
//                ImGui::TextDisabled(connected ? ICON_LC_ARROW_DOWN : ICON_LC_UNPLUG);
//            }
//
//            ImGui::PopID();
            return;
        }

        // It's a group, and it has children (not an empty)
        const auto first = node->children[0];

        const bool connected = first->connected || n == 0;
        const bool disabled  = first->disabled || !connected;

        if (first->children_n <= 0) {
            // Group with children, where the first element _HAS NO_ its own children
            const ImVec2 outer_start = brackets_start();
            {
                if (disabled || disabled_)
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);

                ImGui::PushID(first->id_local);
                node_mappers_.at(first->id_node).draw_tree_node(
                    gui, state, first, false, *selected == first->id_local);

                if (disabled || disabled_)
                    ImGui::PopStyleVar();

                const bool menu = show_context_menu_element(state, gui, first, *menu_node == first->id_local,
                    false, *selected == first->id_local);

                if (menu) {
                    *menu_node = node->id_local;
                }

                if (ImGui::IsItemClicked() || menu) {
                    *selected = first->id_local;
                    *active   = first;
                }

                if (disabled) {
                    ImGui::TableNextColumn();
                    ImGui::TextDisabled(ICON_LC_LINK_2_OFF);
                }
                else if (ImGui::IsItemHovered()) {
                    ImGui::TableNextColumn();
                    ImGui::TextDisabled(connected ? "" : ICON_LC_UNPLUG);
                }
                else {
                    ImGui::TableNextColumn();
                    ImGui::TextDisabled(connected ? ICON_LC_ARROW_DOWN : ICON_LC_UNPLUG);
                }

                if (node->children_n > 1) {
                    for (int i = 1; i < node->children_n; i++) {
                        show_tree_node(state, gui, node->children[i], selected, active, menu_node,
                            disabled_, node->children_n, i);
                    }
                }

                ImGui::PopID();
            }

            brackets_end(outer_start, 0, disabled_ ? 0.5f : 1.f);
            return;
        }

        // Group with children, where the first element _HAS_ its own children
        const ImVec2 outer_start = brackets_start();
        for (int i = 0; i < node->children_n; i++) {
            show_tree_node(state, gui, node->children[i], selected, active, menu_node,
                 disabled_, node->children_n, i);
        }
        brackets_end(outer_start, 0,  disabled_ ? 0.5f : 1.f);
    }

    void work_tree_content(State &state, GuiState &gui) {
        if (state.work_tree == nullptr)
            return;

        ImGui::BeginChild(
                "WorkTreeContent",
                ImVec2(0.0f, ImGui::GetContentRegionAvail().y - ImGui::CalcTextSize(ICON_LC_PLUS).y - ImGui::GetStyle().ItemSpacing.x - 10),
                0);

        if (ImGui::BeginTable("TableWorkTreeTable", 2, flags)) {
            const auto old_indent_spacing = ImGui::GetStyle().IndentSpacing;
            ImGui::GetStyle().IndentSpacing = tree_spacing;

            const float fixed_width = ImGui::GetContentRegionAvail().x - ctrl_width - ImGui::GetStyle().ItemSpacing.x;
            const float items_width = std::max(min_width, fixed_width);

            ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthFixed, items_width);
            ImGui::TableSetupColumn("Controls", ImGuiTableColumnFlags_WidthFixed, ctrl_width);

            static std::size_t selected_node = -1;
            static std::size_t last_menu     = -1;

            const std::size_t prev_menu = last_menu;

            TreeNode *active_node = nullptr;

            ImGui::Indent(10.f);
            for (int i = 0; i < state.work_tree->children_n; i++) {
                show_tree_node(state, gui, state.work_tree->children[i], &selected_node, &active_node, &last_menu,
                               false, state.work_tree->children_n, i);
            }
            ImGui::Unindent(10.f);

            if (active_node != nullptr) {
                if (state.active_node != nullptr && state.active_node != active_node) {
                    state.active_node = active_node;
                }

                if (state.active_node == nullptr) {
                    state.active_node = active_node;
                }
            }

            if (prev_menu == last_menu) {
                last_menu = -1;
            }

            ImGui::GetStyle().IndentSpacing = old_indent_spacing;
            ImGui::EndTable();
        }

        ImGui::EndChild();
    }

    void work_tree_buttons(ex::State &state, GuiState &gui) {
        if (ex::gui::toggle_button(ICON_LC_PLUS)) {
            logger()->debug("Add work element");
            // TODO
        }
        ex::gui::plain_tooltip("Add work element");
        ImGui::SameLine(0, 10);

        if (ex::gui::toggle_button(ICON_LC_MINUS)) {
            logger()->debug("Remove work element");
            // TODO
        }
        ex::gui::plain_tooltip("Remove work element");
        ImGui::SameLine(0, 10);

        if (ex::gui::toggle_button(ICON_LC_CHEVRON_UP)) {
            logger()->debug("Move up element");
            // TODO
        }
        ex::gui::plain_tooltip("Move up element");
        ImGui::SameLine(0, 10);

        if (ex::gui::toggle_button(ICON_LC_CHEVRON_DOWN)) {
            logger()->debug("Move down element");
            // TODO
        }
        ex::gui::plain_tooltip("Move down element");
    }

    void show_work_tree(ex::State &state, GuiState &gui) {
        static bool *watch = nullptr;
        if (watch != nullptr && !(*watch))
            return;

        static bool docked = false;
        static bool show   = true;

        show |= gui.view.reset;
        bool *ptr = (!docked || gui.view.docking) ? &show : nullptr;

        if (!show)
            return;

        ex::gui::next_auto_hide(docked || gui.view.docking);
        ImGui::SetNextWindowDockID(gui.docking.dock_left_top_id, cond_node());
        ImGui::Begin("Work tree", ptr, reset_flag(gui) | ImGuiWindowFlags_NoCollapse);
        docked = ex::gui::window_is_multi_docked();
        watch = watch_parent_dock(gui);

        ImGui::BeginChild("WorkTreeBox", ImVec2(0, 0), 0, ImGuiWindowFlags_NoDecoration);
        work_tree_content(state, gui);
        ImGui::Spacing();
        ImGui::Spacing();
        work_tree_buttons(state, gui);
        ImGui::EndChild();

        ImGui::End();
    }

}