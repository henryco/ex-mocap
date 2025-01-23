//
// Created by xd on 17/01/25.
//

#include "../../../headers/imgui/gui_layout.h"
#include "../../../headers/imgui/gui_tree_ops.h"
#include <ranges>

namespace ex::gui {

    namespace tree {

        void reset_state_(State &state) {
            selection_node::free(state.selected_nodes, true);
            node_buffer::remove(&(state.clipboard_node));
            state.selected_nodes = nullptr;
        }

        void copy_selected(State &state, GuiState &, selection_node *selection) {
            reset_state_(state);
            node_buffer::create(&state.clipboard_node, selection, false, false);
        }

        void copy_node(State &state, GuiState &, TreeNode *node) {
            reset_state_(state);
            node_buffer::create(&state.clipboard_node, node, false, false);
        }

        void ref_node(State &state, GuiState &, TreeNode *node) {
            reset_state_(state);
            node_buffer::create(&state.clipboard_node, node, false, true);
        }

        void cut_selected(State &state, GuiState &, selection_node *selection) {
            reset_state_(state);
            node_buffer::create(&state.clipboard_node, selection, true, false);
            auto head = selection->root;
            while (head != nullptr) {
                head->selected->dimmed = true;
                head = head->next;
            }
        }

        void cut_node(State &state, GuiState &, TreeNode *node) {
            reset_state_(state);
            node_buffer::create(&state.clipboard_node, node, true, false);
            node->dimmed = true;
        }

        void rename_node(State &state, GuiState &, TreeNode *node, const char *name) {
            const auto id_local = node->id_local;
            const auto og_name = node->name;
            const auto new_name = new char[64]{ "" };

            std::memcpy(new_name, name, std::strlen(name));
            node->name = new_name;

            reset_state_(state);
            state.active_node = nullptr;

            undo_history().push("Node renamed",
            new std::function([id_local, og_name] (State *s) {
                reset_state_(*s);
                const auto f_node = s->work_tree->find_node(id_local);
                if (f_node == nullptr)
                    throw std::runtime_error("Cannot find original node");
                f_node->name = og_name;
                s->active_node = nullptr;
            }),
            new std::function([id_local, new_name] (State *s) {
                reset_state_(*s);
                const auto f_node = s->work_tree->find_node(id_local);
                if (f_node == nullptr)
                    throw std::runtime_error("Cannot find original node");
                f_node->name = new_name;
                s->active_node = nullptr;
            }));
        }

        void duplicate_node(State &state, GuiState &, TreeNode *node) {
            reset_state_(state);
            const auto snapshot = state.work_tree->deep_copy();

            TreeNode *copy = TreeNode::deep_copy(node, true);
            copy->name     = (new std::string(node->name))->append(" (copy)").c_str();
            copy->parent   = nullptr;
            copy->index    = 0;

            const auto parent_id   = node->parent->id_local;
            const auto child_index = node->index + 1;

            auto source_cpy = new TreeNode *;
            *source_cpy     = nullptr;

            node->parent->insert_node(child_index, copy);

            undo_history().push("Node duplicated",
            new std::function([snapshot, copy, source_cpy] (ex::State *s) -> void {
                reset_state_(*s);
                // TODO FIXME
                if (*source_cpy == nullptr) {
                    TreeNode *og_node = s->work_tree->find_node(copy);
                    if (og_node == nullptr)
                        throw std::runtime_error("Could not find original node");
                    *source_cpy = og_node->deep_copy();
                    og_node->deleted = true;
                }

                if (s->work_tree != nullptr && s->work_tree != snapshot)
                    delete s->work_tree;
                s->work_tree            = snapshot->deep_copy();
                s->work_tree->created   = true;
                s->work_tree->connected = false;
            }),

            new std::function([parent_id, child_index, source_cpy] (ex::State *s) -> void {
                reset_state_(*s);
                if (const auto p_node = s->work_tree->find_node(parent_id); p_node != nullptr) {
                    if (*source_cpy == nullptr)
                        throw std::runtime_error("Could not find original node");
                    const auto new_copy = (*source_cpy)->deep_copy();
                    new_copy->created   = true;
                    p_node->insert_node(child_index, new_copy);
                } else
                    throw std::runtime_error("Cannot find original node");
            }),

            new std::function([snapshot, source_cpy] {
                if (snapshot != nullptr)
                    delete snapshot;
                if (source_cpy != nullptr) {
                    if (*source_cpy != nullptr)
                        delete *source_cpy;
                    delete source_cpy;
                }
            }));
        }

        void paste_after_node(State &state, GuiState &, TreeNode *) {
            // TODO
            reset_state_(state);
        }

        void paste_into_node(State &state, GuiState &, TreeNode *) {
            // TODO
            reset_state_(state);
        }

        void group_selected(State &state, GuiState &, selection_node *) {
            // TODO
            reset_state_(state);
        }

        void ungroup_selected(State &state, GuiState &, selection_node *) {
            // TODO
            reset_state_(state);
        }

        void enable_node(State &state, GuiState &, TreeNode *node) {
            const bool og_state = node->disabled;
            node->disabled = false;

            reset_state_(state);

            undo_history().push("Node enabled",
            new std::function([id_l = node->id_local, og_state] (ex::State *s) {
                reset_state_(*s);
                const auto f_node = s->work_tree->find_node(id_l);
                if (f_node == nullptr)
                    throw std::runtime_error("Cannot find original node");
                f_node->disabled = og_state;
                f_node->updated = true;
            }),
            new std::function([id_l = node->id_local] (ex::State *s) {
                reset_state_(*s);
                const auto f_node = s->work_tree->find_node(id_l);
                if (f_node == nullptr)
                    throw std::runtime_error("Cannot find original node");
                f_node->disabled = false;
                f_node->updated = true;
            }));
        }

        void disable_node(State &state, GuiState &, TreeNode *node) {
            const bool og_state = node->disabled;
            node->disabled = true;

            reset_state_(state);

            undo_history().push("Node disabled",
            new std::function([id_l = node->id_local, og_state] (ex::State *s) {
                reset_state_(*s);
                const auto f_node = s->work_tree->find_node(id_l);
                if (f_node == nullptr)
                    throw std::runtime_error("Cannot find original node");
                f_node->disabled = og_state;
                f_node->updated = true;
            }),
            new std::function([id_l = node->id_local] (ex::State *s) {
                reset_state_(*s);
                const auto f_node = s->work_tree->find_node(id_l);
                if (f_node == nullptr)
                    throw std::runtime_error("Cannot find original node");
                f_node->disabled = true;
                f_node->updated = true;
            }));
        }

        void enable_selected(State &state, GuiState &, selection_node *selection) {
            if (selection == nullptr)
                return;

            std::map<std::size_t, bool> og_map;
            auto head = selection->root;
            while (head != nullptr) {
                og_map.emplace(head->selected->id_local, head->selected->disabled);
                head->selected->disabled = false;
                head = head->next;
            }

            reset_state_(state);

            undo_history().push("Selection enabled",
            new std::function([og_state = og_map](ex::State *s) {
                reset_state_(*s);
                for (const auto &[k, v] : og_state) {
                    const auto f_node = s->work_tree->find_node(k);
                    if (f_node == nullptr)
                        throw std::runtime_error("Cannot find original node");
                    f_node->disabled = v;
                    f_node->updated = true;
                }
            }),
            new std::function([og_state = og_map](ex::State *s) {
                reset_state_(*s);
                for (const auto &k : std::views::keys(og_state)) {
                    const auto f_node = s->work_tree->find_node(k);
                    if (f_node == nullptr)
                        throw std::runtime_error("Cannot find original node");
                    f_node->disabled = false;
                    f_node->updated = true;
                }
            }));
        }

        void disable_selected(State &state, GuiState &, selection_node *selection) {
            if (selection == nullptr)
                return;

            std::map<std::size_t, bool> og_map;
            auto head = selection->root;
            while (head != nullptr) {
                og_map.emplace(head->selected->id_local, head->selected->disabled);
                head->selected->disabled = true;
                head = head->next;
            }

            reset_state_(state);

            undo_history().push("Selection disabled",
            new std::function([og_state = og_map](ex::State *s) {
                for (const auto &[k, v] : og_state) {
                    const auto f_node = s->work_tree->find_node(k);
                    if (f_node == nullptr)
                        throw std::runtime_error("Cannot find original node");
                    f_node->disabled = v;
                    f_node->updated = true;
                }
                reset_state_(*s);
            }),
            new std::function([og_state = og_map](ex::State *s) {
                for (const auto &k : std::views::keys(og_state)) {
                    const auto f_node = s->work_tree->find_node(k);
                    if (f_node == nullptr)
                        throw std::runtime_error("Cannot find original node");
                    f_node->disabled = true;
                    f_node->updated = true;
                }
                reset_state_(*s);
            }));
        }

        void delete_selected(State &state, GuiState &, selection_node *selection) {
            if (selection == nullptr)
                return;

            const auto copy_selection = selection_node::copy(selection);
            reset_state_(state);

            auto head = copy_selection->root;
            while (head != nullptr) {
                head->selected->selected = false;
                head = head->next;
            }

            const auto snapshot = state.work_tree->deep_copy();

            head = copy_selection->root;
            while (head != nullptr) {
                if (!head->selected->is_group)
                    head->selected->deleted = true;
                head->selected = TreeNode::shallow_copy(head->selected);
                head = head->next;
            }

            undo_history().push("Selection deleted",
            new std::function([snapshot] (ex::State *s) {
                reset_state_(*s);
                if (s->work_tree != nullptr && s->work_tree != snapshot)
                    delete s->work_tree;
                s->work_tree            = snapshot->deep_copy();
                s->work_tree->connected = false;
                s->work_tree->created   = true;
            }),
            new std::function([copy_selection] (ex::State *s) {
                reset_state_(*s);
                auto s_head = copy_selection->root;
                while (s_head != nullptr) {
                    const auto f_node = s->work_tree->find_node(s_head->selected->id_local);
                    if (f_node != nullptr && !f_node->is_group)
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

            reset_state_(state);

            node->selected = false;
            const auto node_id  = node->id_local;
            const auto snapshot = state.work_tree->deep_copy();
            node->deleted = true;

            undo_history().push("Node deleted",
            new std::function([snapshot] (ex::State *s) {
                reset_state_(*s);

                if (s->work_tree != nullptr && s->work_tree != snapshot)
                    delete s->work_tree;
                s->work_tree            = snapshot->deep_copy();
                s->work_tree->connected = false;
                s->work_tree->created   = true;
            }),
            new std::function([node_id] (ex::State *s) {
                reset_state_(*s);

                const auto f_node = s->work_tree->find_node(node_id);
                if (f_node == nullptr)
                   throw std::runtime_error("Cannot find original node");
                s->selected_nodes = nullptr;
                f_node->deleted   = true;
            }),
            new std::function([snapshot] {
                if (snapshot != nullptr)
                    delete snapshot;
            }));
        }
    }

    bool show_context_menu_group(State &state, GuiState &gui, selection_node *selection) {
        if (selection == nullptr)
            return false;

        static auto menu_id = "show_context_menu_group";
        bool clicked = false;

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.f);
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup(menu_id);
            clicked = true;
        }

        auto head = selection->root;
        bool enabled = false, disabled = false, same_root = true;
        while (head != nullptr) {
            enabled |= !head->selected->disabled;
            disabled |= head->selected->disabled;
            if (head->selected->parent != selection->root->selected->parent)
                same_root = false;
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

            if (same_root) {
                ImGui::Separator();

                if (ImGui::MenuItem("Cut", "Ctrl+Z")) {
                    tree::cut_selected(state, gui, selection);
                }

                if (ImGui::MenuItem("Copy", "Ctrl+C")) {
                    tree::copy_selected(state, gui, selection);
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Delete")) {
                tree::delete_selected(state, gui, selection);
            }

            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();

        return clicked;
    }

    bool show_context_menu_element(State &state, GuiState &gui, TreeNode *node,
        const bool last, const bool nested, const bool selected)
    {
        static auto menu_id = "show_context_menu_element";
        static char name_buffer[64] = "";

        bool rename_popup = false;
        bool clicked      = false;

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.f);
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup(menu_id);
            clicked = true;
        }
        else if (last) {
            ImGui::OpenPopup(menu_id);
        }

        if (ImGui::BeginPopup(menu_id)) {

            if (ImGui::MenuItem("Rename")) {
                std::memset(name_buffer, 0, sizeof(name_buffer));
                std::memcpy(name_buffer, node->name, std::strlen(node->name));
                rename_popup = true;
            }

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

        if (rename_popup) {
            ImGui::OpenPopup("rename-popup");
        }

        if (ImGui::BeginPopup("rename-popup")) {
            ImGui::Text("Edit name: ");
            ImGui::InputText("##edit", name_buffer, IM_ARRAYSIZE(name_buffer));
            if (ImGui::Button("Apply")) {
                tree::rename_node(state, gui, node, name_buffer);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        } else if (rename_popup) {
            ImGui::OpenPopup("rename-popup");
        }

        ImGui::PopStyleVar();
        return clicked;
    }

}