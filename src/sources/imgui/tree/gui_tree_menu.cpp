//
// Created by xd on 17/01/25.
//

#include "../../../headers/imgui/tree/gui_tree_menu.h"
#include "../../../headers/imgui/actions/gui_actions_registry.h"
#include "../../../headers/imgui/gui_globals.h"
#include "../../../headers/imgui/gui_strings.h"
#include "../../../headers/imgui/tree/gui_tree_ops.h"
#include "../../../headers/logic/logic_globals.h"

namespace ex::gui::tree {

    namespace tm {
        const auto &node_mappers_ = node_components();
    }


    void remove_actions_() {
        const auto &mappings = key_mappings();
        actions::registry::remove_action(mappings.work_tree.copy.key);
        actions::registry::remove_action(mappings.work_tree.cut.key);
        actions::registry::remove_action(mappings.work_tree.deletion.key);
        actions::registry::remove_action(mappings.work_tree.duplicate.key);
        actions::registry::remove_action(mappings.work_tree.insert.key);
        actions::registry::remove_action(mappings.work_tree.paste.key);
        actions::registry::remove_action(mappings.work_tree.reference.key);
    }

    static void action_delete_node_() {
        const auto &selection = ex::gui::tree::state().selection;
        if (selection == nullptr)
            return;
        const auto &node = selection->selected;
        tree::delete_node(node);
    }

    static void action_duplicate_node_() {
        const auto &selection = ex::gui::tree::state().selection;
        if (selection == nullptr)
            return;
        const auto &node = selection->selected;
        tree::duplicate_node(node);
    }

    static void action_cut_node_() {
        const auto &selection = ex::gui::tree::state().selection;
        if (selection == nullptr)
            return;
        const auto &node = selection->selected;
        tree::cut_node(node);
    }

    static void action_copy_node_() {
        const auto &selection = ex::gui::tree::state().selection;
        if (selection == nullptr)
            return;
        const auto &node = selection->selected;
        tree::copy_node(node);
    }

    static void action_ref_node_() {
        const auto &selection = ex::gui::tree::state().selection;
        if (selection == nullptr)
            return;
        const auto &node = selection->selected;
        tree::ref_node(node);
    }

    static void action_paste_after_node_() {
        const auto &selection = ex::gui::tree::state().selection;
        if (selection == nullptr)
            return;
        const auto &node = selection->selected;
        tree::paste_after_node(node);
    }

    static void action_paste_into_node_() {
        const auto &selection = ex::gui::tree::state().selection;
        if (selection == nullptr)
            return;
        const auto &node = selection->selected;
        tree::paste_into_node(node);
    }

    static void action_cut_selected_() {
        const auto &selection = ex::gui::tree::state().selection;
        if (selection == nullptr)
            return;
        tree::cut_selected(selection);
    }

    static void action_copy_selected_() {
        const auto &selection = ex::gui::tree::state().selection;
        if (selection == nullptr)
            return;
        tree::copy_selected(selection);
    }

    static void action_delete_selected_() {
        const auto &selection = ex::gui::tree::state().selection;
        if (selection == nullptr)
            return;
        tree::delete_selected(selection);
    }

    static void action_paste_only_() {
        tree::paste_node();
    }

    void process_paste_action_only() {
        const auto &mappings = key_mappings();
        remove_actions_();

        if (ex::gui::tree::state().buffer == nullptr)
            return;

        actions::registry::set_action(mappings.work_tree.paste.key, action_paste_only_);
    }

    void process_single_action_(TreeNode *node) {
        const auto &mappings = key_mappings();
        remove_actions_();

        if (node->state >>= node_state::is_link) {
            actions::registry::set_action(mappings.work_tree.deletion.key, action_delete_node_);
            return;
        }

        actions::registry::set_action(mappings.work_tree.duplicate.key, action_duplicate_node_);
        actions::registry::set_action(mappings.work_tree.cut.key, action_cut_node_);
        actions::registry::set_action(mappings.work_tree.copy.key, action_copy_node_);
        actions::registry::set_action(mappings.work_tree.reference.key, action_ref_node_);
        actions::registry::set_action(mappings.work_tree.deletion.key, action_delete_node_);

        if (ex::gui::tree::state().buffer == nullptr)
            return;

        actions::registry::set_action(mappings.work_tree.paste.key, action_paste_after_node_);
        actions::registry::set_action(mappings.work_tree.insert.key, action_paste_into_node_);
    }

    void process_multi_action_() {
        const auto &mappings = key_mappings();
        remove_actions_();

        const auto selection = ex::gui::tree::state().selection;
        auto head = selection->root;
        bool same_root = true;
        while (head != nullptr) {
            if (head->selected->parent != selection->root->selected->parent)
                same_root = false;
            head = head->next;
        }

        actions::registry::set_action(mappings.work_tree.deletion.key, action_delete_selected_);
        if (same_root) {
            actions::registry::set_action(mappings.work_tree.cut.key, action_cut_selected_);
            actions::registry::set_action(mappings.work_tree.copy.key, action_copy_selected_);
        }
    }

    TreeNode *create_node_from_menu() {
        for (int i = 0; i < tm::node_mappers_.key_entries.size; ++i) {
            const auto &type_id = tm::node_mappers_.key_entries[i];
            const auto &mapper  = tm::node_mappers_.val_entries[i];
            ImGui::PushID(type_id);
            if (ImGui::MenuItem(mapper.get_node_name() )) {
                ImGui::PopID();
                TreeNode *new_node   = TreeNode::create();
                new_node->state      = node_state::created;
                new_node->children_n = 0;
                new_node->id_node    = type_id;
                new_node->name       = strings::gui_tree_menu_general_new;
                return new_node;
            }
            ImGui::PopID();
        }
        return nullptr;
    }

    bool show_context_menu_group() {
        const auto &tree_state = ex::gui::tree::state();

        const auto selection = tree_state.selection;
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
            enabled |= !(head->selected->state >>= node_state::disabled);
            disabled |= (head->selected->state >>= node_state::disabled);
            if (head->selected->parent != selection->root->selected->parent)
                same_root = false;
            head = head->next;
        }

        const bool is_grouped = selection->selected->parent != nullptr &&
                                selection->selected->parent->parent != nullptr &&
                                (selection->selected->parent->parent->state >>= node_state::is_group);

        if (ImGui::BeginPopup(menu_id)) {

            if (enabled && ImGui::MenuItem(strings::gui_tree_menu_group_disable)) {
                tree::disable_selected(selection);
            }

            if (disabled && ImGui::MenuItem(strings::gui_tree_menu_group_enable)) {
                tree::enable_selected(selection);
            }

            if (same_root) {
                if (ImGui::MenuItem(strings::gui_tree_menu_group)) {
                    tree::group_selected(selection);
                }

                if (is_grouped) {
                    if (ImGui::MenuItem(strings::gui_tree_menu_group_ungroup)) {
                        tree::ungroup_selected(selection);
                    }
                }

                ImGui::Separator();

                if (ImGui::MenuItem(strings::gui_tree_menu_group_cut, key_mappings().work_tree.cut.label)) {
                    tree::cut_selected(selection);
                }

                if (ImGui::MenuItem(strings::gui_tree_menu_group_copy, key_mappings().work_tree.copy.label)) {
                    tree::copy_selected(selection);
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem(strings::gui_tree_menu_group_delete)) {
                tree::delete_selected(selection);
            }

            ImGui::EndPopup();
        }
        ImGui::PopStyleVar(2);

        return clicked;
    }

    bool show_context_menu_element(TreeNode *node, const bool last, const bool nested, const bool selected) {
        if (node == nullptr)
            return false;

        const auto &tree_state = ex::gui::tree::state();

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
        const bool is_grouped =
                (parent != nullptr) && (parent->parent != nullptr) && (parent->parent->state >>= node_state::is_group);


        if (ImGui::BeginPopup(menu_id)) {

            if (ImGui::BeginMenu(strings::gui_tree_menu_node_new)) {
                if (ImGui::BeginMenu(strings::gui_tree_menu_node_add_into)) {
                    if (const auto new_node = create_node_from_menu())
                        tree::add_into_node(node, new_node);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu(strings::gui_tree_menu_node_add_after)) {
                    if (const auto new_node = create_node_from_menu())
                        tree::add_after_node(node, new_node);
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::MenuItem(strings::gui_tree_menu_node_rename)) {
                std::memset(name_buffer, 0, sizeof(name_buffer));
                std::memcpy(name_buffer, node->name, std::strlen(node->name));
                rename_popup = true;
            }

            const bool enabled = !(node->state >>= node_state::disabled);
            if (ImGui::MenuItem(enabled ? strings::gui_tree_menu_node_disable : strings::gui_tree_menu_node_enable)) {
                if (enabled) tree::disable_node(node);
                else tree::enable_node(node);
            }

            if (ImGui::MenuItem(strings::gui_tree_menu_node_group)) {
                tree::selection_node selection;
                selection.selected = node;
                selection.root     = &selection;
                selection.next     = nullptr;
                selection.prev     = nullptr;
                tree::group_selected(&selection);
            }

            if (is_grouped) {
                if (ImGui::MenuItem(strings::gui_tree_menu_node_ungroup)) {
                    tree::selection_node selection;
                    selection.selected = node;
                    selection.root     = &selection;
                    selection.next     = nullptr;
                    selection.prev     = nullptr;
                    tree::ungroup_selected(&selection);
                }
            }

            if (!(node->state >>= node_state::is_link)) {
                if (ImGui::MenuItem(strings::gui_tree_menu_node_inspect)) {
                    tree::pick_inspected_node(node);
                }

                tm::node_mappers_[node->id_node].draw_tree_menu(node, nested, selected);

                if (ImGui::MenuItem(strings::gui_tree_menu_node_duplicate, key_mappings().work_tree.duplicate.label)) {
                    tree::duplicate_node(node);
                }

                if (ImGui::MenuItem(strings::gui_tree_menu_node_cut, key_mappings().work_tree.cut.label)) {
                    tree::cut_node(node);
                }

                if (ImGui::BeginMenu(strings::gui_tree_menu_node_copy)) {

                    if (ImGui::MenuItem(strings::gui_tree_menu_node_copy, key_mappings().work_tree.copy.label)) {
                        tree::copy_node(node);
                    }

                    if (ImGui::MenuItem(strings::gui_tree_menu_node_copy_ref, key_mappings().work_tree.reference.label)) {
                        tree::ref_node(node);
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu(strings::gui_tree_menu_node_paste, tree_state.buffer != nullptr)) {

                    if (ImGui::MenuItem(strings::gui_tree_menu_node_paste_after, key_mappings().work_tree.paste.label)) {
                        tree::paste_after_node(node);
                    }

                    if (ImGui::MenuItem(strings::gui_tree_menu_node_paste_into, key_mappings().work_tree.insert.label)) {
                        tree::paste_into_node(node);
                    }

                    ImGui::EndMenu();
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem(strings::gui_tree_menu_node_delete)) {
                tree::delete_node(node);
            }

            ImGui::EndPopup();
        }

        if (rename_popup) {
            ImGui::OpenPopup("rename-popup");
        }

        if (ImGui::BeginPopup("rename-popup")) {
            ImGui::Text(strings::gui_tree_menu_node_edit_name);
            ImGui::InputText("##edit", name_buffer, IM_ARRAYSIZE(name_buffer));
            if (ImGui::Button(strings::gui_tree_menu_node_edit_apply)) {
                tree::rename_node(node, name_buffer);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        } else if (rename_popup) {
            ImGui::OpenPopup("rename-popup");
        }

        ImGui::PopStyleVar(2);
        return clicked;
    }

    bool show_context_menu_general() {
        const auto &tree_state = ex::gui::tree::state();

        static auto menu_id = "show_context_menu_general";

        bool clicked = false;

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && !ImGui::IsItemHovered() &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup(menu_id);
            clicked = true;
        }

        if (ImGui::BeginPopup(menu_id)) {

            if (ImGui::BeginMenu(strings::gui_tree_menu_general_add)) {
                if (const auto new_node = create_node_from_menu())
                    tree::add_node(new_node);
                ImGui::EndMenu();
            }

            if (tree_state.buffer != nullptr) {
                if (ImGui::MenuItem(strings::gui_tree_menu_general_paste, key_mappings().work_tree.paste.label)) {
                    tree::paste_node();
                }
            }

            ImGui::EndPopup();
        }

        ImGui::PopStyleVar(2);
        return clicked;
    }

    void register_context_menu_actions() {
        const auto &tree_state = ex::gui::tree::state();

        static char last_active = 0;

        if (tree_state.selection == nullptr && tree_state.buffer == nullptr) {
            if (last_active != 0)
                remove_actions_();
            last_active = 0;
            return;
        }

        if (tree_state.selection == nullptr && tree_state.buffer != nullptr) {
            if (last_active != 1)
                process_paste_action_only();
            last_active = 1;
            return;
        }

        if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
            return;

        if (tree::selection_node::single(tree_state.selection)) {
            static TreeNode *last_node    = nullptr;
            TreeNode        *current_node = tree_state.selection->selected;
            if (last_active != 2 || current_node != last_node) {
                process_single_action_(current_node);
                last_node = current_node;
            }
            last_active = 2;
            return;
        }

        if (last_active != 3)
            process_multi_action_();
        last_active = 3;
    }

}
