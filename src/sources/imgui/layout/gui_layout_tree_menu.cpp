//
// Created by xd on 17/01/25.
//

#include "../../../headers/imgui/gui_layout.h"
#include <IconsLucide.h>

namespace ex::gui {

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
                // TODO
            }

            if (disabled && ImGui::MenuItem("Enable")) {
                // TODO
            }

            if (ImGui::MenuItem("Group")) {
                // TODO
            }

            if (ImGui::MenuItem("Ungroup")) {
                // TODO
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Cut", "Ctrl+Z")) {
                node_buffer::create(&state.clipboard_node, selection, true, false);
                // TODO change opacity maybe?
            }

            if (ImGui::MenuItem("Copy", "Ctrl+C")) {
                node_buffer::create(&state.clipboard_node, selection, false, false);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Delete")) {
                // TODO
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

            if (ImGui::MenuItem("Duplicate", "Ctrl+D")) {
                // TODO
            }

            if (ImGui::MenuItem("Cut", "Ctrl+Z")) {
                node_buffer::create(&state.clipboard_node, node, true, false);
                // TODO change opacity maybe?
            }

            if (ImGui::BeginMenu("Copy")) {

                if (ImGui::MenuItem("Copy", "Ctrl+C")) {
                    node_buffer::create(&state.clipboard_node, node, false, false);
                }

                if (ImGui::MenuItem("Copy Ref", "Ctrl+R")) {
                    node_buffer::create(&state.clipboard_node, node, false, true);
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Paste", state.clipboard_node != nullptr)) {

                if (ImGui::MenuItem("Paste", "Ctrl+V")) {
                    // TODO
                }

                if (ImGui::MenuItem("Paste into")) {
                    // TODO
                }

                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Delete")) {
                node->deleted = true;
                // TODO
            }

            ImGui::EndPopup();
        }
        return clicked;
    }

}