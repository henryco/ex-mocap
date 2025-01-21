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

    bool select_many(selection_node **selection, TreeNode *node, TreeNode *start, TreeNode *stop) {
        if (start == nullptr || stop == nullptr || node == nullptr || selection == nullptr)
            return false;

        if (node == start || node->id_local == start->id_local) {
            if (*selection != nullptr)
                selection_node::free(*selection);

            *selection             = new selection_node;
            (*selection)->root     = *selection;
            (*selection)->prev     = nullptr;
            (*selection)->next     = nullptr;
            (*selection)->selected = node;

            goto find_more;
        }

        // TODO FIX SELECTION INDEX

        if (*selection != nullptr) {
            const auto next = new selection_node;
            next->root      = (*selection)->root;
            next->prev      = *selection;
            next->next      = nullptr;
            next->selected  = node;

            (*selection)->next = next;
            *selection         = next;

            if (node == stop || node->id_local == stop->id_local)
                return true;
        }

    find_more:

        if (node->children_n <= 0 || node->children == nullptr) {
            return false;
        }

        for (int i = 0; i < node->children_n; i++) {
            if (select_many(selection, node->children[i], start, stop))
                return true;
        }

        return false;
    }

    bool select_node(selection_node **selection, TreeNode *node, const int counter) {
        // SELECTION ADD SINGLE (Ctrl)
        if (ImGui::GetIO().KeyCtrl && *selection != nullptr) {

            if ((*selection)->root->next == nullptr && (*selection)->selected == node) {

                if (node->selected) {
                    // OPTIONAL
                    selection_node::free(*selection, true);
                    node->selected = false;
                    *selection = nullptr;
                    return false;
                    // OPTIONAL
                }

                return true;
            }

            node->selected = !node->selected;

            if (node->selected) {
                auto head = (*selection)->root;
                while (head->next != nullptr) {
                    head = head->next;
                }

                auto *new_node     = new selection_node;
                new_node->root     = (*selection)->root;
                new_node->prev     = head;
                new_node->next     = nullptr;
                new_node->index    = counter;
                new_node->selected = node;
                head->next         = new_node;

                *selection = new_node;
                return true;
            }

            auto head = (*selection)->root;
            while (head != nullptr) {
                if (head->selected == node) {

                    if (head->prev == nullptr && head->next == nullptr) {
                        *selection = nullptr;
                        delete head;
                        return false;
                    }

                    if (head->prev == nullptr) {
                        const auto root = head->next;
                        root->prev = nullptr;

                        *selection = root;

                        auto next = root;
                        while (next != nullptr) {
                            next->root = root;
                            next = next->next;
                        }

                        delete head;
                        return false;
                    }

                    if (head->next == nullptr) {
                        head->prev->next = nullptr;
                        *selection       = head->prev;

                        delete head;
                        return false;
                    }

                    head->prev->next = head->next;
                    head->next->prev = head->prev;
                    *selection       = head->next;

                    delete head;
                    return false;
                }
                head = head->next;
            }

            return false;
        }

        // SELECTION ADD MANY (RANGE)
        if (ImGui::GetIO().KeyShift && *selection != nullptr) {

            const auto *min_node = (*selection)->root;
            const auto *cur_node = (*selection)->root;
            while (cur_node != nullptr) {
                if (cur_node->index <= min_node->index)
                    min_node = cur_node;
                cur_node = cur_node->next;
            }

            auto *start = (min_node->index <= counter) ? min_node->selected : node;
            auto *stop  = (min_node->index >= counter) ? min_node->selected : node;

            if (start == stop) {
                selection_node::free(*selection, true);
                *selection = nullptr;
                return false;
            }

            auto *root = node->parent;
            while (root->parent != nullptr) {
                root = root->parent;
            }

            selection_node *new_selected = nullptr;
            selection_node::free(*selection, true);
            select_many(&new_selected, root, start, stop);

            if (new_selected == nullptr) {
                *selection = nullptr;
                return false;
            }

            auto base  = new_selected->root;
            *selection = base;

            while (base != nullptr) {
                base->selected->selected = true;
                base = base->next;
            }

            return true;
        }

        // REGULAR SELECTION (SINGLE)
        if (*selection != nullptr) {
            selection_node::free(*selection, true);
            *selection = new selection_node;
        }

        if (*selection == nullptr) {
            *selection = new selection_node;
        }

        (*selection)->selected = node;
        (*selection)->index    = counter;
        (*selection)->root     = *selection;
        (*selection)->next     = nullptr;
        (*selection)->prev     = nullptr;
        node->selected         = true;

        return true;
    }

    int count_children(TreeNode *node, const int counter) {
        if (node == nullptr)
            return counter;

        int counter_i = counter;

        if (!node->is_group) {
            if (node->children_n > 0 && node->children != nullptr) {
                const auto next = node->children[0];
                for (int i = 0; i < next->children_n; i++)
                    counter_i = count_children(next->children[i], counter_i + 1);
            }
            return counter_i;
        }

        if (node->children_n <= 0 || node->children == nullptr)
            return counter_i;

        const auto first = node->children[0];
        if (first->children_n <= 0) {
            if (node->children_n > 1) {
                for (int i = 1; i < node->children_n; i++)
                    counter_i = count_children(node->children[i], counter_i + 1);
            }
            return counter_i;
        }

        for (int i = 0; i < node->children_n; i++)
            counter_i = count_children(node->children[i], counter_i + 1);

        return counter_i;
    }

    int show_tree_node(State &state, GuiState &gui, TreeNode *node, selection_node **selected,
                        TreeNode **active, std::size_t *menu_node, const bool disabled_,
                        const int counter, const int n)
    {
        if (node == nullptr)
            return counter;

        int counter_i = counter;

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
                gui, state, node, node->children_n > 0, node->selected);

            if (disabled || disabled_)
                ImGui::PopStyleVar();


            bool menu_element = false;
            if (selection_node::single(*selected)) {
                menu_element = show_context_menu_element(state, gui, node,
                    *menu_node == node->id_local, node->children_n > 0, node->selected);
            } else {
                show_context_menu_group(state, gui, *selected);
            }

            if (menu_element) {
                *menu_node = node->id_local;
            }

            if (open && node->children_n > 0)
                ImGui::Indent();

            if ((ImGui::IsItemClicked() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) || menu_element) {
                if (select_node(selected, node, counter_i)) {
                    *active = node;
                }
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
                for (int i = 0; i < next->children_n; i++) {
                    counter_i = show_tree_node(state, gui, next->children[i], selected, active, menu_node,
                        disabled || disabled_, counter_i + 1, i);
                }
                ImGui::TreePop();

                if (disabled || disabled_)
                    ImGui::PopStyleVar();

                ImGui::Unindent();
                brackets_end_half(pos, 0, disabled || disabled_ ? 0.5f : 1.f);
            }

            else if (node->children_n > 0)
                counter_i = count_children(node, counter_i);

            ImGui::PopID();
            return counter_i;
        }

        // It's a group
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        if (node->children_n <= 0 || node->children == nullptr) {
            // ignore such case
            return counter_i;
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
                    gui, state, first, false, first->selected);

                if (disabled || disabled_)
                    ImGui::PopStyleVar();

                bool menu_element = false;
                if (selection_node::single(*selected)) {
                    menu_element = show_context_menu_element(state, gui, first,
                        *menu_node == first->id_local, false, first->selected);
                } else {
                    show_context_menu_group(state, gui, *selected);
                }

                if (menu_element) {
                    *menu_node = first->id_local;
                }

                if ((ImGui::IsItemClicked() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) || menu_element) {
                    if (select_node(selected, first, counter_i)) {
                        *active = first;
                    }
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
                        counter_i = show_tree_node(state, gui, node->children[i], selected, active, menu_node,
                            disabled_, counter_i + 1, i);
                    }
                }

                ImGui::PopID();
            }

            brackets_end(outer_start, 0, disabled_ ? 0.5f : 1.f);
            return counter_i;
        }

        // Group with children, where the first element _HAS_ its own children
        const ImVec2 outer_start = brackets_start();
        for (int i = 0; i < node->children_n; i++) {
            counter_i = show_tree_node(state, gui, node->children[i], selected, active, menu_node,
                 disabled_, counter_i + 1, i);
        }
        brackets_end(outer_start, 0,  disabled_ ? 0.5f : 1.f);

        return counter_i;
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

            static std::size_t last_menu   = -1;
            const std::size_t  prev_menu   = last_menu;
            TreeNode          *active_node = nullptr;

            ImGui::Indent(10.f);
            for (int i = 0, p = 0; i < state.work_tree->children_n; i++) {
                p = show_tree_node(state, gui, state.work_tree->children[i], &state.selected_nodes,
                    &active_node, &last_menu, false, p + 1, i);
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