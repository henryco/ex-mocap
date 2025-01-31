//
// Created by xd on 17/01/25.
//

#include "../../../headers/imgui/gui_layout.h"
#include "../../../headers/imgui/gui_globals.h"
#include "../../../headers/imgui/gui_tree_ops.h"

namespace ex::gui {

    namespace tm {
        const auto &node_mappers_ = node_components();
    }

    TreeNode *create_node_from_menu(State &state, GuiState &gui) {
        for (const auto &[type_id, mapper]: tm::node_mappers_) {
            ImGui::PushID(type_id);
            if (ImGui::MenuItem(mapper.get_node_name(gui, state) )) {
                ImGui::PopID();
                TreeNode *new_node = TreeNode::create();
                new_node->created    = true;
                new_node->children_n = 0;
                new_node->id_node    = type_id;
                new_node->name       = "New node";
                return new_node;
            }
            ImGui::PopID();
        }
        return nullptr;
    }

    bool show_context_menu_group(State &state, GuiState &gui, selection_node *selection) {
        if (selection == nullptr)
            return false;

        static auto menu_id = "show_context_menu_group";
        bool clicked = false;

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));
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

        const bool is_grouped = selection->selected->parent != nullptr &&
                                selection->selected->parent->parent != nullptr &&
                                selection->selected->parent->parent->is_group;

        if (ImGui::BeginPopup(menu_id)) {

            if (enabled && ImGui::MenuItem("Disable")) {
                tree::disable_selected(state, gui, selection);
            }

            if (disabled && ImGui::MenuItem("Enable")) {
                tree::enable_selected(state, gui, selection);
            }

            if (same_root) {
                if (ImGui::MenuItem("Group")) {
                    tree::group_selected(state, gui, selection);
                }

                if (is_grouped) {
                    if (ImGui::MenuItem("Ungroup")) {
                        tree::ungroup_selected(state, gui, selection);
                    }
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Cut", key_mappings().work_tree.cut.label)) {
                    tree::cut_selected(state, gui, selection);
                }

                if (ImGui::MenuItem("Copy", key_mappings().work_tree.copy.label)) {
                    tree::copy_selected(state, gui, selection);
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Delete")) {
                tree::delete_selected(state, gui, selection);
            }

            ImGui::EndPopup();
        }
        ImGui::PopStyleVar(2);

        return clicked;
    }

    bool show_context_menu_element(State &state, GuiState &gui, TreeNode *node,
        const bool last, const bool nested, const bool selected)
    {
        if (node == nullptr)
            return false;

        static auto menu_id = "show_context_menu_element";
        static char name_buffer[64] = "";

        bool rename_popup = false;
        bool clicked      = false;

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup(menu_id);
            clicked = true;
        }
        else if (last) {
            ImGui::OpenPopup(menu_id);
        }

        const auto parent = node->parent;
        // ReSharper disable once CppDFANullDereference
        const bool is_grouped = (parent != nullptr) && (parent->parent != nullptr) && (parent->parent->is_group);

        if (ImGui::BeginPopup(menu_id)) {

            if (ImGui::BeginMenu("New")) {
                if (ImGui::BeginMenu("Add into")) {
                    if (const auto new_node = create_node_from_menu(state, gui))
                        tree::add_into_node(state, gui, node, new_node);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Add After")) {
                    if (const auto new_node = create_node_from_menu(state, gui))
                        tree::add_after_node(state, gui, node, new_node);
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

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

            if (ImGui::MenuItem("Group")) {
                selection_node selection;
                selection.selected = node;
                selection.root     = &selection;
                selection.next     = nullptr;
                selection.prev     = nullptr;
                tree::group_selected(state, gui, &selection);
            }

            if (is_grouped) {
                if (ImGui::MenuItem("Ungroup")) {
                    selection_node selection;
                    selection.selected = node;
                    selection.root     = &selection;
                    selection.next     = nullptr;
                    selection.prev     = nullptr;
                    tree::ungroup_selected(state, gui, &selection);
                }
            }

            if (!node->is_link) {
                if (ImGui::MenuItem("Inspect")) {
                    tree::pick_inspected_node(state, gui, node);
                }

                tm::node_mappers_.at(node->id_node).draw_tree_menu(gui, state, node, nested, selected);

                if (ImGui::MenuItem("Duplicate", key_mappings().work_tree.duplicate.label)) {
                    tree::duplicate_node(state, gui, node);
                }

                if (ImGui::MenuItem("Cut", key_mappings().work_tree.cut.label)) {
                    tree::cut_node(state, gui, node);
                }

                if (ImGui::BeginMenu("Copy")) {

                    if (ImGui::MenuItem("Copy", key_mappings().work_tree.copy.label)) {
                        tree::copy_node(state, gui, node);
                    }

                    if (ImGui::MenuItem("Copy Ref", key_mappings().work_tree.reference.label)) {
                        tree::ref_node(state, gui, node);
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Paste", state.clipboard_node != nullptr)) {

                    if (ImGui::MenuItem("Paste after", key_mappings().work_tree.paste.label)) {
                        tree::paste_after_node(state, gui, node);
                    }

                    if (ImGui::MenuItem("Paste into", key_mappings().work_tree.insert.label)) {
                        tree::paste_into_node(state, gui, node);
                    }

                    ImGui::EndMenu();
                }
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

        ImGui::PopStyleVar(2);
        return clicked;
    }

    bool show_context_menu_general(State &state, GuiState &gui) {
        static auto menu_id = "show_context_menu_general";

        bool clicked = false;

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));
        if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            selection_node::free(state.selected_nodes, true);
            state.selected_nodes = nullptr;
            ImGui::OpenPopup(menu_id);
            clicked = true;
        }

        if (ImGui::BeginPopup(menu_id)) {
            if (ImGui::BeginMenu("Add new node")) {
                if (const auto new_node = create_node_from_menu(state, gui))
                    tree::add_node(state, gui, new_node);
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

        ImGui::PopStyleVar(2);
        return clicked;
    }

}