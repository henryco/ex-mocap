//
// Created by xd on 17/01/25.
//

#include "../../../headers/imgui/gui_layout.h"
#include "../../../headers/imgui/gui_tree_ops.h"

namespace ex::gui {

    namespace tree {

        TreeNode *deep_copy_node(State &state, GuiState &gui, TreeNode *node, const bool generate_id) {
            if (node == nullptr)
                return nullptr;

            void *new_data = nullptr;
            if (node_mappers_.contains(node->id_node)) {
                new_data = node_mappers_.at(node->id_node).copy_node_data(gui, state, node, node->data);
            }

            const auto new_node  = new TreeNode;
            new_node->connected  = false;
            new_node->selected   = false;
            new_node->deleted    = false;
            new_node->created    = true;
            new_node->updated    = false;
            new_node->parent     = nullptr;
            new_node->data       = new_data;
            new_node->name       = node->name;
            new_node->is_group   = node->is_group;
            new_node->shared     = node->shared;
            new_node->disabled   = node->disabled;
            new_node->id_local   = generate_id ? 0 : node->id_local;
            new_node->id_node    = node->id_node;
            new_node->children_n = node->children_n;

            if (node->children_n > 0) {
                new_node->children = new TreeNode *[node->children_n];
                for (auto i = 0; i < node->children_n; i++) {
                    const auto copy_child = deep_copy_node(state, gui, node->children[i], generate_id);
                    if (copy_child == nullptr) {
                        new_node->children[i] = nullptr;
                        continue;
                    }
                    copy_child->parent    = new_node;
                    new_node->children[i] = copy_child;
                }
            }

            return new_node;
        }

        void copy_selected(State &state, GuiState &_, selection_node *selection) {
            node_buffer::create(&state.clipboard_node, selection, false, false);
        }

        void copy_node(State &state, GuiState &_, TreeNode *node) {
            node_buffer::create(&state.clipboard_node, node, false, false);
        }

        void ref_node(State &state, GuiState &_, TreeNode *node) {
            node_buffer::create(&state.clipboard_node, node, false, true);
        }

        void cut_selected(State &state, GuiState &_, selection_node *selection) {
            node_buffer::create(&state.clipboard_node, selection, true, false);
            // TODO change opacity maybe?
        }

        void cut_node(State &state, GuiState &_, TreeNode *node) {
            node_buffer::create(&state.clipboard_node, node, true, false);
            // TODO change opacity maybe?
        }

        void paste_after_node(State &state, GuiState &gui, TreeNode *node) {
            // TODO
        }

        void paste_into_node(State &state, GuiState &gui, TreeNode *node) {
            // TODO
        }

        void duplicate_node(State &state, GuiState &gui, TreeNode *node) {
            // TODO
        }

        void enable_node(State &_, GuiState &__, TreeNode *node) {
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

        void disable_node(State &_, GuiState &__, TreeNode *node) {
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

        void enable_selected(State &_, GuiState &__, selection_node *selection) {
            if (selection == nullptr)
                return;

            std::map<std::size_t, bool> og_map;
            auto head = selection;
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
                for (const auto &[k, ___] : og_state) {
                    const auto f_node = s->work_tree->find_node(k);
                    if (f_node == nullptr)
                        return;
                    f_node->disabled = false;
                    f_node->updated = true;
                }
            }));
        }

        void disable_selected(State &_, GuiState &__, selection_node *selection) {
            if (selection == nullptr)
                return;
            std::map<std::size_t, bool> og_map;
            auto head = selection;
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
                for (const auto &[k, ___] : og_state) {
                    const auto f_node = s->work_tree->find_node(k);
                    if (f_node == nullptr)
                        return;
                    f_node->disabled = true;
                    f_node->updated = true;
                }
            }));
        }

        void delete_selected(State &state, GuiState &gui, selection_node *selection) {
            if (selection == nullptr)
                return;

            node_buffer::remove(&state.clipboard_node);

            auto head = selection;
            while (head != nullptr) {
                const auto selected = head->selected;
                if (!selected->is_group)
                    selected->deleted = true;
                head = head->next;
            }

            undo_history().push("Selection deleted",
            new std::function([ &gui ] (ex::State *s) {
                // TODO
            }),
            new std::function([ ] (ex::State *s) {
                // TODO
            }),
            new std::function([ ] {
                // TODO
            }));
        }

        void delete_node(State &state, GuiState &gui, TreeNode *node) {
            if (node == nullptr)
                return;

            node_buffer::remove(&state.clipboard_node);

            const auto parent     = node->parent;
            const auto node_id    = node->id_local;
            const auto deep_copy  = deep_copy_node(state, gui, node, false);
            const auto parent_id  = parent->id_local;
            const int  node_index = parent->get_node_index(node);

            node->deleted = true;

            undo_history().push("Node deleted",
            new std::function([deep_copy, parent_id, node_index, &gui] (ex::State *s) {
                const auto p_node = s->work_tree->find_node(parent_id);
                if (p_node == nullptr)
                    return;
                p_node->set_node(node_index, deep_copy_node(*s, gui, deep_copy, false));
            }),
            new std::function([node_id] (ex::State *s) {
                const auto f_node = s->work_tree->find_node(node_id);
                if (f_node == nullptr)
                    return;
                f_node->deleted = true;
            }),
            new std::function([deep_copy] {
                if (deep_copy == nullptr)
                    return;
                delete deep_copy;
            }));
        }

        void group_selected(State &state, GuiState &gui, selection_node *selection) {
            // TODO
        }

        void ungroup_selected(State &state, GuiState &gui, selection_node *selection) {
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