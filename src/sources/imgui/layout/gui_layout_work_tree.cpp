//
// Created by xd on 02/12/24.
//

#include <IconsLucide.h>

#include "../../../headers/imgui/gui_globals.h"
#include "../../../headers/imgui/layout/gui_layout.h"
#include "../../../headers/imgui/tree/gui_tree_menu.h"
#include "../../../headers/imgui/tree/gui_tree_ops.h"
#include "../../../headers/logic/logic_globals.h"
#include "../../../headers/utils/data_utils.h"

namespace work_tree::internals_ {
    static const auto &mappers_ = ex::gui::node_components();

    inline const ex::gui::GuiNodeComponent *fetch_(const std::uint64_t &key) {
        return &mappers_[key];
    }

    static ex::data::cache_line cache (fetch_, 10);

}

namespace ex::gui {

    constexpr auto flags = ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_BordersInnerV;

    constexpr float tree_spacing = 20.f;
    constexpr float ctrl_width   = 40.f;
    constexpr float min_width    = 100.f;

    constexpr auto col_orange = IM_COL32(255, 140, 40, 255);
    constexpr auto col_purple = IM_COL32(255, 0, 255, 255);
    constexpr auto vec_purple = ImVec4(255, 0, 255, 255);

    static constexpr auto dnd_type = "node";

    void highlight_border() {
        constexpr auto off       = 3.f;
        const auto     p_min     = ImGui::GetItemRectMin();
        const auto     p_max     = ImGui::GetItemRectMax();
        const auto     draw_list = ImGui::GetForegroundDrawList();
        draw_list->AddRect(ImVec2(p_min.x, p_min.y - off), ImVec2(p_max.x, p_max.y + off), col_purple, 3.f, 0, 1.f);
    }

    void highlight_line() {
        const auto p_min     = ImGui::GetItemRectMin();
        const auto p_max     = ImGui::GetItemRectMax();
        const auto draw_list = ImGui::GetForegroundDrawList();
        draw_list->AddLine(ImVec2(p_min.x, p_max.y), ImVec2(p_max.x, p_max.y), col_purple, 1.f);
    }

    void drag_space(State &state, GuiState &gui, TreeNode *node, const GuiNodeComponent &node_mapper) {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_PayloadAutoExpire | ImGuiDragDropFlags_SourceNoHoldToOpenOthers)) {
            if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                ImGui::ClearActiveID();
                ImGui::EndDragDropSource();
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
            } else {
                ImGui::SetDragDropPayload(dnd_type, &node, sizeof(node));
                node_mapper.draw_tree_node(gui, state, node, false, false);
                ImGui::EndDragDropSource();
            }
        }
    }

    void insertion_space(TreeNode *node, const int n) {
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0.f));

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::PushID(n);
        ImGui::InvisibleButton("", ImVec2(ImGui::GetContentRegionAvail().x, 0.01f), ImGuiButtonFlags_AllowOverlap);

        if (ImGui::BeginDragDropTarget()) {
            if (const auto payload = ImGui::AcceptDragDropPayload(dnd_type, ImGuiDragDropFlags_AcceptPeekOnly)) {
                const auto data_node = static_cast<TreeNode **>(payload->Data);
                if (data_node == nullptr || *data_node == node || TreeNode::is_parent(node, *data_node)) {
                    ImGui::EndDragDropTarget();
                    ImGui::PopID();
                    ImGui::PopStyleVar();
                    ImGui::TableNextColumn();
                    return;
                }
                highlight_line();
            }

            if (const auto payload = ImGui::AcceptDragDropPayload(dnd_type, ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
                const auto data_node = static_cast<TreeNode **>(payload->Data);
                tree::move_after_node(node, *data_node);
            }

            ImGui::EndDragDropTarget();
        }

        ImGui::PopID();

        ImGui::PopStyleVar();
        ImGui::TableNextColumn();
    }

    void insertion_node(TreeNode *node) {
        if (ImGui::BeginDragDropTarget()) {
            if (const auto payload = ImGui::AcceptDragDropPayload(dnd_type, ImGuiDragDropFlags_AcceptPeekOnly)) {
                const auto data_node = static_cast<TreeNode **>(payload->Data);
                if (data_node == nullptr || *data_node == node || TreeNode::is_parent(node, *data_node)) {
                    ImGui::EndDragDropTarget();
                    return;
                }
                highlight_border();
            }

            if (const auto payload = ImGui::AcceptDragDropPayload(dnd_type, ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
                const auto data_node = static_cast<TreeNode **>(payload->Data);
                tree::move_into_node(node, *data_node);
            }

            ImGui::EndDragDropTarget();
        }
    }

    bool select_many(tree::selection_node **selection, TreeNode *node, TreeNode *start, TreeNode *stop) {
        if (start == nullptr || stop == nullptr || node == nullptr || selection == nullptr)
            return false;

        if (node == start || node->id_local == start->id_local) {
            if (*selection != nullptr)
                tree::selection_node::free(*selection);

            *selection             = new tree::selection_node;
            (*selection)->root     = *selection;
            (*selection)->prev     = nullptr;
            (*selection)->next     = nullptr;
            (*selection)->selected = node;

            goto find_more;
        }

        if (*selection != nullptr) {
            const auto next = new tree::selection_node;
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

    bool select_node(tree::selection_node **selection, TreeNode *node, const int counter) {
        // SELECTION ADD SINGLE (Ctrl)
        if (ImGui::GetIO().KeyCtrl && *selection != nullptr) {

            if ((*selection)->root->next == nullptr && (*selection)->selected == node) {

                if (node->state >>= node_state::selected) {
                    // OPTIONAL
                    tree::selection_node::free(*selection, true);
                    node->state -= node_state::selected;
                    *selection = nullptr;
                    return false;
                    // OPTIONAL
                }

                return true;
            }

            node->state ^= node_state::selected;

            if (node->state >>= node_state::selected) {
                auto head = (*selection)->root;
                while (head->next != nullptr) {
                    head = head->next;
                }

                auto *new_node     = new tree::selection_node;
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
            const auto *max_node = (*selection)->root;
            const auto *cur_node = (*selection)->root;

            while (cur_node != nullptr) {
                if (cur_node->index <= min_node->index && cur_node->index >= 0)
                    min_node = cur_node;
                if (min_node->index < 0 && cur_node->index >= 0)
                    min_node = cur_node;
                if (cur_node->index > max_node->index)
                    max_node = cur_node;
                cur_node = cur_node->next;
            }

            const int index_min = std::min(counter, min_node->index);
            const int index_max = std::max(counter, min_node->index);

            TreeNode *start = nullptr;
            TreeNode *stop  = nullptr;

            start = (min_node->index <= counter) ? min_node->selected : node;

            if (counter < index_max) {
                stop = max_node->selected;
            }
            else {
                stop = (min_node->index >= counter) ? min_node->selected : node;
            }

            if (start == stop) {
                tree::selection_node::free(*selection, true);
                *selection = nullptr;
                return false;
            }

            auto *root = node->parent;
            while (root->parent != nullptr) {
                root = root->parent;
            }

            tree::selection_node *new_selected = nullptr;
            tree::selection_node::free(*selection, true);
            select_many(&new_selected, root, start, stop);

            if (new_selected == nullptr) {
                *selection = nullptr;
                return false;
            }

            auto base  = new_selected->root;
            *selection = base;

            while (base != nullptr) {
                base->selected->state |= node_state::selected;
                if (base->next == nullptr)
                    base->index = index_max;
                base = base->next;
            }

            (*selection)->root->index = index_min;
            return true;
        }

        // REGULAR SELECTION (SINGLE)
        if (*selection != nullptr) {
            tree::selection_node::free(*selection, true);
            *selection = new tree::selection_node;
        }

        if (*selection == nullptr) {
            *selection = new tree::selection_node;
        }

        (*selection)->selected = node;
        (*selection)->index    = counter;
        (*selection)->root     = *selection;
        (*selection)->next     = nullptr;
        (*selection)->prev     = nullptr;
        node->state |= node_state::selected;

        return true;
    }

    int count_children(TreeNode *node, const int counter) {
        if (node == nullptr)
            return counter;

        int counter_i = counter;

        if (!(node->state >>= node_state::is_group)) {
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

    int show_tree_node(State &state, GuiState &gui, TreeNode *node, tree::selection_node **selected, TreeNode **active,
                       std::size_t *menu_node, std::size_t *ref_node, const bool disabled_, const int counter,
                       const int n) {

        if (node == nullptr)
            return counter;

        int counter_i = counter;

        if (!(node->state >>= node_state::is_group)) {
            // Not a group just regular element

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            const bool reference = node->state >>= node_state::is_link;
            const bool connected = (node->state >>= node_state::connected) || reference || n <= 0;
            const bool symlinked = *ref_node > 0 && *ref_node == node->id_local;
            const bool disabled  = node->state >>= node_state::disabled;
            const bool dimmed    = node->state >>= node_state::dimmed;

            if (disabled || disabled_ || dimmed)
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);

            ImGui::PushID(node->id_local);

//            const auto &mapper       = work_tree::internals_::mappers_[node->id_node];
            const auto &mapper       = *work_tree::internals_::cache(node->id_node);

            const bool is_nested_   = node->children_n > 0;
            const bool is_selected_ = node->state >>= node_state::selected;
            const bool open = mapper.draw_tree_node(gui, state, node, is_nested_, is_selected_);

            if (ImGui::IsItemHovered())
                *ref_node = (node->state >>= node_state::is_link) ? node->id_link : (-1);
            else if (node->id_link != -1 && node->id_link == *ref_node)
                *ref_node = (-1);

            if (disabled || disabled_ || dimmed)
                ImGui::PopStyleVar();

            if (symlinked)
                highlight_border();

            insertion_node(node);

            bool menu_element = false;
            if (tree::selection_node::single(*selected)) {
                menu_element = tree::show_context_menu_element(node, *menu_node == node->id_local,
                                                         node->children_n > 0, (node->state >>= node_state::selected));
                drag_space(state, gui, node, mapper);
            } else {
                tree::show_context_menu_group();
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

            if (symlinked) {
                ImGui::TableNextColumn();
                ImGui::TextColored(vec_purple, ICON_LC_LINK_2);
            }
            else if (disabled) {
                ImGui::TableNextColumn();
                ImGui::TextDisabled(ICON_LC_LINK_2_OFF);
            }
            else if (reference) {
                ImGui::TableNextColumn();
                ImGui::TextDisabled(ICON_LC_LINK);
            }
            else if (ImGui::IsItemHovered()) {
                ImGui::TableNextColumn();
                if (!connected)
                    ImGui::PushStyleColor(ImGuiCol_TextDisabled, col_orange);
                ImGui::TextDisabled(connected ? "" : ICON_LC_UNPLUG);
                if (!connected)
                    ImGui::PopStyleColor();
            }
            else {
                ImGui::TableNextColumn();
                if (!connected)
                    ImGui::PushStyleColor(ImGuiCol_TextDisabled, col_orange);
                ImGui::TextDisabled(connected ? (node->children_n > 0 ? ICON_LC_ARROW_DOWN_RIGHT : "")
                                              : ICON_LC_UNPLUG);
                if (!connected)
                    ImGui::PopStyleColor();
            }

            if (open && node->children_n > 0 && !(node->children[0]->state >>= node_state::created)) {
                if (disabled || disabled_ || dimmed)
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);

                // Regular element but it has children
                const auto next = node->children[0];
                for (int i = 0, u = 0; i < next->children_n; i++) {
                    counter_i = show_tree_node(state, gui, next->children[i],
                        selected, active, menu_node, ref_node,
                                               disabled || disabled_ || dimmed, counter_i + 1, u);
                    u += !(next->children[i]->state >>= node_state::disabled);
                }
                ImGui::TreePop();

                if (disabled || disabled_ || dimmed)
                    ImGui::PopStyleVar();

                ImGui::Unindent();
                brackets_end_half(pos, 0, (disabled || disabled_ || dimmed) ? 0.5f : 1.f);
            }

            else if (node->children_n > 0)
                counter_i = count_children(node, counter_i);

            ImGui::PopID();

            insertion_space(node, counter_i);

            return counter_i;
        }

        // It's a group
        if (node->children_n <= 0 || node->children == nullptr) {
            // ignore such case
            return counter_i;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        // It's a group, and it has children (not an empty)
        const auto first = node->children[0];

        const bool symlinked = *ref_node > 0 && *ref_node == first->id_local;
        const bool reference = first->state >>= node_state::is_link;
        const bool disabled  = first->state >>= node_state::disabled;
        const bool dimmed    = first->state >>= node_state::dimmed;

        if (first->children_n <= 0) {
            // Group with children, where the first element _HAS NO_ its own children
            const ImVec2 outer_start = brackets_start();
            {
                if (disabled || disabled_ || dimmed)
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);

                ImGui::PushID(first->id_local);

//                const auto &mapper       = work_tree::internals_::mappers_[first->id_node];
                const auto &mapper = *work_tree::internals_::cache(first->id_node);

                const bool is_selected_ = first->state >>= node_state::selected;
                mapper.draw_tree_node(gui, state, first, false, is_selected_);

                if (ImGui::IsItemHovered())
                    *ref_node = (first->state >>= node_state::is_link) ? first->id_link : (-1);
                else if (first->id_link != -1 && first->id_link == *ref_node)
                    *ref_node = (-1);

                if (disabled || disabled_ || dimmed)
                    ImGui::PopStyleVar();

                if (symlinked)
                    highlight_border();

                insertion_node(first);

                bool menu_element = false;
                if (tree::selection_node::single(*selected)) {
                    menu_element = tree::show_context_menu_element(first, *menu_node == first->id_local, false,
                                                             first->state >>= node_state::selected);
                    drag_space(state, gui, first, mapper);
                } else {
                    tree::show_context_menu_group();
                }

                if (menu_element) {
                    *menu_node = first->id_local;
                }

                if ((ImGui::IsItemClicked() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) || menu_element) {
                    if (select_node(selected, first, counter_i)) {
                        *active = first;
                    }
                }

                if (symlinked) {
                    ImGui::TableNextColumn();
                    ImGui::TextColored(vec_purple, ICON_LC_LINK_2);
                }
                else if (disabled) {
                    ImGui::TableNextColumn();
                    ImGui::TextDisabled(ICON_LC_LINK_2_OFF);
                }
                else if (reference) {
                    ImGui::TableNextColumn();
                    ImGui::TextDisabled(ICON_LC_LINK);
                }
                else if (ImGui::IsItemHovered()) {
                    ImGui::TableNextColumn();
                    ImGui::TextDisabled("");
                }
                else {
                    ImGui::TableNextColumn();
                    ImGui::TextDisabled(ICON_LC_ARROW_DOWN);
                }

                insertion_space(first, counter_i);
                ImGui::PopID();

                int u = disabled ? 0 : 1;
                if (node->children_n > 1) {
                    for (int i = 1; i < node->children_n; i++) {
                        counter_i = show_tree_node(state, gui, node->children[i],
                            selected, active, menu_node, ref_node,
                                                   disabled_, counter_i + 1, u);
                        u += !(node->children[i]->state >>= node_state::disabled);
                    }
                }
            }

            brackets_end(outer_start, 0, disabled_ ? 0.5f : 1.f);

            insertion_space(node->parent, counter);
            return counter_i;
        }

        // Group with children, where the first element _HAS_ its own children
        const ImVec2 outer_start = brackets_start();
        for (int i = 0, u = 0; i < node->children_n; i++) {
            counter_i = show_tree_node(state, gui, node->children[i],
                selected, active, menu_node, ref_node, disabled_,
                                       counter_i + 1, u);
            u += !(node->children[i]->state >>= node_state::disabled);
        }

        brackets_end(outer_start, 0, disabled_ ? 0.5f : 1.f);

        insertion_space(node->parent, counter);
        return counter_i;
    }

    void work_tree_content() {
        tree::TreeState &tree_state = ex::gui::tree::state();
        State &state = ex::logic::globals::state();
        GuiState &gui = ex::gui::state();

        if (state.logic.work_tree == nullptr)
            return;

        ImGui::BeginChild(
                "WorkTreeContent",
                ImVec2(0.0f, -10.f),
                0);

        if (ImGui::BeginTable("TableWorkTreeTable", 2, flags)) {
            const auto old_indent_spacing = ImGui::GetStyle().IndentSpacing;
            ImGui::GetStyle().IndentSpacing = tree_spacing;

            const float fixed_width = ImGui::GetContentRegionAvail().x - ctrl_width - ImGui::GetStyle().ItemSpacing.x;
            const float items_width = std::max(min_width, fixed_width);

            ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthFixed, items_width);
            ImGui::TableSetupColumn("Controls", ImGuiTableColumnFlags_WidthFixed, ctrl_width);

            static std::size_t last_ref    = -1;
            static std::size_t last_menu   = -1;
            const std::size_t  prev_menu   = last_menu;
            TreeNode          *active_node = nullptr;

            ImGui::Indent(10.f);
            work_tree::internals_::cache.begin();
            for (int i = 0, p = 0; i < state.logic.work_tree->children_n; i++) {
                p = show_tree_node(state, gui, state.logic.work_tree->children[i], &tree_state.selection,
                    &active_node, &last_menu, &last_ref, false, p + 1, i);
            }
            work_tree::internals_::cache.end();
            ImGui::Unindent(10.f);

            if (active_node != nullptr) {
                if (state.logic.active_node != nullptr && state.logic.active_node != active_node) {
                    state.logic.active_node = active_node;
                }

                if (state.logic.active_node == nullptr) {
                    state.logic.active_node = active_node;
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

    void show_work_tree() {
        GuiState &gui = ex::gui::state();

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
        tree::register_context_menu_actions();

        docked = ex::gui::window_is_multi_docked();
        watch = watch_parent_dock(gui);

        ImGui::BeginChild("WorkTreeBox", ImVec2(0, 0), 0, ImGuiWindowFlags_NoDecoration);
        tree::show_context_menu_general();
        work_tree_content();
        ImGui::EndChild();

        ImGui::End();
        tree::show_inspected_node();
    }

}