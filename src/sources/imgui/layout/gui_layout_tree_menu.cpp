//
// Created by xd on 17/01/25.
//

#include "../../../headers/imgui/gui_layout.h"
#include "../../../headers/imgui/gui_tree_ops.h"

namespace ex::gui {

    namespace tree {

        void copy_selected(State &state, GuiState &, selection_node *selection) {
            node_buffer::create(&state.clipboard_node, selection, false, false);
        }

        void copy_node(State &state, GuiState &, TreeNode *node) {
            node_buffer::create(&state.clipboard_node, node, false, false);
        }

        void ref_node(State &state, GuiState &, TreeNode *node) {
            node_buffer::create(&state.clipboard_node, node, false, true);
        }

        void cut_selected(State &state, GuiState &, selection_node *selection) {
            node_buffer::create(&state.clipboard_node, selection, true, false);
            // TODO change opacity maybe?
        }

        void cut_node(State &state, GuiState &, TreeNode *node) {
            node_buffer::create(&state.clipboard_node, node, true, false);
            // TODO change opacity maybe?
        }

        void paste_after_node(State &, GuiState &, TreeNode *) {
            // TODO
        }

        void paste_into_node(State &, GuiState &, TreeNode *) {
            // TODO
        }

        void duplicate_node(State &, GuiState &, TreeNode *) {
            // TODO
        }

        void enable_node(State &, GuiState &, TreeNode *node) {
            const bool og_state = node->disabled;
            node->disabled = false;
            undo_history().push("Node enabled",
            new std::function([id_l = node->id_local, og_state] (ex::State *s) {
                const auto f_node = s->work_tree->find_node(id_l);
                if (f_node == nullptr)
                    return;
                f_node->disabled = og_state;
                f_node->updated = true;
            }),
            new std::function([id_l = node->id_local] (ex::State *s) {
                const auto f_node = s->work_tree->find_node(id_l);
                if (f_node == nullptr)
                    return;
                f_node->disabled = false;
                f_node->updated = true;
            }));
        }

        void disable_node(State &, GuiState &, TreeNode *node) {
            const bool og_state = node->disabled;
            node->disabled = true;
            undo_history().push("Node disabled",
            new std::function([id_l = node->id_local, og_state] (ex::State *s) {
                const auto f_node = s->work_tree->find_node(id_l);
                if (f_node == nullptr)
                    return;
                f_node->disabled = og_state;
                f_node->updated = true;
            }),
            new std::function([id_l = node->id_local] (ex::State *s) {
                const auto f_node = s->work_tree->find_node(id_l);
                if (f_node == nullptr)
                    return;
                f_node->disabled = true;
                f_node->updated = true;
            }));
        }

        void enable_selected(State &, GuiState &, selection_node *selection) {
            if (selection == nullptr)
                return;

            std::map<std::size_t, bool> og_map;
            auto head = selection->root;
            while (head != nullptr) {
                og_map.emplace(head->selected->id_local, head->selected->disabled);
                head->selected->disabled = false;
                head = head->next;
            }

            undo_history().push("Selection enabled",
            new std::function([og_state = og_map](ex::State *s) {
                for (const auto &[k, v] : og_state) {
                    const auto f_node = s->work_tree->find_node(k);
                    if (f_node == nullptr)
                        return;
                    f_node->disabled = v;
                    f_node->updated = true;
                }
            }),
            new std::function([og_state = og_map](ex::State *s) {
                for (const auto &[k, _] : og_state) {
                    const auto f_node = s->work_tree->find_node(k);
                    if (f_node == nullptr)
                        return;
                    f_node->disabled = false;
                    f_node->updated = true;
                }
            }));
        }

        void disable_selected(State &, GuiState &, selection_node *selection) {
            if (selection == nullptr)
                return;
            std::map<std::size_t, bool> og_map;
            auto head = selection->root;
            while (head != nullptr) {
                og_map.emplace(head->selected->id_local, head->selected->disabled);
                head->selected->disabled = true;
                head = head->next;
            }

            undo_history().push("Selection disabled",
            new std::function([og_state = og_map](ex::State *s) {
                for (const auto &[k, v] : og_state) {
                    const auto f_node = s->work_tree->find_node(k);
                    if (f_node == nullptr)
                        return;
                    f_node->disabled = v;
                    f_node->updated = true;
                }
            }),
            new std::function([og_state = og_map](ex::State *s) {
                for (const auto &[k, _] : og_state) {
                    const auto f_node = s->work_tree->find_node(k);
                    if (f_node == nullptr)
                        return;
                    f_node->disabled = true;
                    f_node->updated = true;
                }
            }));
        }

        void delete_selected(State &state, GuiState &, selection_node *selection) {
            if (selection == nullptr)
                return;

            const auto copy_selection = selection_node::copy(selection);

            node_buffer::remove(&state.clipboard_node);
            selection_node::free(state.selected_nodes);
            state.selected_nodes = nullptr;

            auto head = copy_selection->root;
            while (head != nullptr) {
                head->selected->selected = false;
                head = head->next;
            }

            const auto snapshot = state.work_tree->deep_copy();

            head = copy_selection->root;
            while (head != nullptr) {
                head->selected->deleted = true;
                head = head->next;
            }

            undo_history().push("Selection deleted",
            new std::function([snapshot] (ex::State *s) {
                if (s->work_tree != nullptr && s->work_tree != snapshot)
                    delete s->work_tree;
                s->work_tree            = snapshot->deep_copy();
                s->work_tree->connected = false;
                s->work_tree->created   = true;
            }),
            new std::function([copy_selection] (ex::State *s) {
                node_buffer::remove(&(s->clipboard_node));
                selection_node::free(s->selected_nodes);
                s->selected_nodes = nullptr;

                auto s_head = copy_selection->root;
                while (s_head != nullptr) { // TODO FIXME
                    const auto f_node = s->work_tree->find_node(s_head->selected->id_local);
                    if (f_node != nullptr)
                        f_node->deleted = true;
                    s_head = s_head->next;
                }
            }),
            new std::function([snapshot, copy_selection] {
                if (snapshot != nullptr)
                    delete snapshot;
                selection_node::free(copy_selection);
            }));
        }

        void delete_node(State &state, GuiState &, TreeNode *node) {
            if (node == nullptr)
                return;

            node_buffer::remove(&state.clipboard_node);
            selection_node::free(state.selected_nodes);
            state.selected_nodes = nullptr;

            node->selected = false;
            const auto node_id  = node->id_local;
            const auto snapshot = state.work_tree->deep_copy();
            node->deleted = true;

            undo_history().push("Node deleted",
            new std::function([snapshot] (ex::State *s) {
                if (s->work_tree != nullptr && s->work_tree != snapshot)
                    delete s->work_tree;
                s->work_tree            = snapshot->deep_copy();
                s->work_tree->connected = false;
                s->work_tree->created   = true;
            }),
            new std::function([node_id] (ex::State *s) {
                node_buffer::remove(&(s->clipboard_node));
                selection_node::free(s->selected_nodes);
                const auto f_node = s->work_tree->find_node(node_id);
                if (f_node == nullptr)
                   return;
                s->selected_nodes = nullptr;
                f_node->deleted   = true;
            }),
            new std::function([snapshot] {
                if (snapshot != nullptr)
                    delete snapshot;
            }));
        }

        void group_selected(State &, GuiState &, selection_node *) {
            // TODO
        }

        void ungroup_selected(State &, GuiState &, selection_node *) {
            // TODO
        }
    }

    bool show_context_menu_group(State &state, GuiState &gui, selection_node *selection) {
        if (selection == nullptr)
            return false;

        static auto menu_id = "show_context_menu_group";
        bool clicked = false;

        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup(menu_id);
            clicked = true;
        }

        auto head = selection->root;
        bool enabled = false, disabled = false;
        while (head != nullptr) {
            enabled |= !head->selected->disabled;
            disabled |= head->selected->disabled;
            head = head->next;
        }

        if (ImGui::BeginPopup(menu_id)) {

            if (enabled && ImGui::MenuItem("Disable")) {
                tree::disable_selected(state, gui, selection);
            }

            if (disabled && ImGui::MenuItem("Enable")) {
                tree::enable_selected(state, gui, selection);
            }

            if (ImGui::MenuItem("Group")) {
                tree::group_selected(state, gui, selection);
            }

            if (ImGui::MenuItem("Ungroup")) {
                tree::ungroup_selected(state, gui, selection);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Cut", "Ctrl+Z")) {
                tree::cut_selected(state, gui, selection);
            }

            if (ImGui::MenuItem("Copy", "Ctrl+C")) {
                tree::copy_selected(state, gui, selection);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Delete")) {
                tree::delete_selected(state, gui, selection);
            }

            ImGui::EndPopup();
        }

        return clicked;
    }

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
                if (enabled) tree::disable_node(state, gui, node);
                else tree::enable_node(state, gui, node);
            }

            node_mappers_.at(node->id_node).draw_tree_menu(gui, state, node, nested, selected);

            if (ImGui::MenuItem("Duplicate", "Ctrl+D")) {
                tree::duplicate_node(state, gui, node);
            }

            if (ImGui::MenuItem("Cut", "Ctrl+Z")) {
                tree::cut_node(state, gui, node);
            }

            if (ImGui::BeginMenu("Copy")) {

                if (ImGui::MenuItem("Copy", "Ctrl+C")) {
                    tree::copy_node(state, gui, node);
                }

                if (ImGui::MenuItem("Copy Ref", "Ctrl+R")) {
                    tree::ref_node(state, gui, node);
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Paste", state.clipboard_node != nullptr)) {

                if (ImGui::MenuItem("Paste", "Ctrl+V")) {
                    tree::paste_after_node(state, gui, node);
                }

                if (ImGui::MenuItem("Paste into")) {
                    tree::paste_into_node(state, gui, node);
                }

                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Delete")) {
                tree::delete_node(state, gui, node);
            }

            ImGui::EndPopup();
        }
        return clicked;
    }

}