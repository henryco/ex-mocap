//
// Created by xd on 02/12/24.
//

#include "../../../headers/imgui/gui_layout.h"

namespace ex::gui {
    constexpr auto flags = ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_BordersInnerV;

    constexpr float tree_spacing = 20.f;
    constexpr float ctrl_width   = 60.f;
    constexpr float min_width    = 100.f;

    void show_tree_node(State &state, GuiState &gui, TreeNode *node, std::size_t *selected, const bool last) {
        if (!node->is_group) {
            // Not a group just regular element

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            ImGui::PushID(node->id_local);
            const bool open = node_mappers_.at(node->id_node).draw_tree_node(
                gui, node->name,
                node->children_n > 0,
                *selected == node->id_local);

            if (open && node->children_n > 0)
                ImGui::Indent();

            if (ImGui::IsItemClicked())
                *selected = node->id_local;

            const auto pos = ImGui::GetCursorScreenPos();

            if (ImGui::IsItemHovered()) {
                ImGui::TableNextColumn();
                ImGui::TextDisabled("");
            } else {
                ImGui::TableNextColumn();
                ImGui::TextDisabled(node->children_n > 0 ? ICON_LC_ARROW_DOWN_RIGHT : (last ? ICON_LC_ARROW_LEFT : ""));
            }

            if (open && node->children_n > 0) {
                // Regular element but it has children
                const auto next = node->children[0];
                for (int i = 0; i < next->children_n; i++)
                    show_tree_node(state, gui, next->children[i], selected, i == next->children_n - 1);
                ImGui::TreePop();

                ImGui::Unindent();
                brackets_end_half(pos);
            }

            ImGui::PopID();
            return;
        }

        // It's a group
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        if (node->children_n <= 0) {
            // Group but it has no children (an empty one)

            ImGui::PushID(node->id_local);
            node_mappers_.at(node->id_node).draw_tree_node(
                gui, node->name,
                false,
                *selected == node->id_local);

            if (ImGui::IsItemClicked())
                *selected = node->id_local;

            if (ImGui::IsItemHovered()) {
                ImGui::TableNextColumn();
                ImGui::TextDisabled("");
            } else {
                ImGui::TableNextColumn();
                ImGui::TextDisabled("--");
            }

            ImGui::PopID();
            return;
        }

        // It's a group, and it has children (not an empty)
        const auto first = node->children[0];

        if (first->children_n <= 0) {
            // Group with children, where the first element _HAS NO_ its own children

            const ImVec2 outer_start = brackets_start();
            {
                ImGui::PushID(first->id_local);
                node_mappers_.at(first->id_node).draw_tree_node(
                    gui, first->name,
                    false,
                    *selected == first->id_local);

                if (ImGui::IsItemClicked())
                    *selected = first->id_local;

                if (ImGui::IsItemHovered()) {
                    ImGui::TableNextColumn();
                    ImGui::TextDisabled("");
                } else {
                    ImGui::TableNextColumn();
                    ImGui::TextDisabled(ICON_LC_ARROW_DOWN);
                }

                if (node->children_n > 1) {
                    for (int i = 1; i < node->children_n; i++) {
                        show_tree_node(state, gui, node->children[i], selected, i == node->children_n - 1);
                    }
                }

                ImGui::PopID();
            }

            brackets_end(outer_start);
            return;
        }

        // Group with children, where the first element _HAS_ its own children
        const ImVec2 outer_start = brackets_start();
        for (int i = 0; i < node->children_n; i++) {
            show_tree_node(state, gui, node->children[i], selected, i == node->children_n - 1);
        }
        brackets_end(outer_start);
    }

    void work_tree_content(State &state, GuiState &gui) {
        if (state.work_tree == nullptr)
            return;

        ImGui::BeginChild(
                "WorkTreeContent",
                ImVec2(0.0f, ImGui::GetContentRegionAvail().y - ImGui::CalcTextSize(ICON_LC_PLUS).y - ImGui::GetStyle().ItemSpacing.x - tree_spacing),
                0);

        if (ImGui::BeginTable("TableWorkTreeTable", 2, flags)) {
            const auto old_indent_spacing = ImGui::GetStyle().IndentSpacing;
            ImGui::GetStyle().IndentSpacing = tree_spacing;

            const float fixed_width = ImGui::GetContentRegionAvail().x - ctrl_width - ImGui::GetStyle().ItemSpacing.x;
            const float items_width = std::max(min_width, fixed_width);

            ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthFixed, items_width);
            ImGui::TableSetupColumn("Controls", ImGuiTableColumnFlags_WidthFixed, ctrl_width);

            static std::size_t selected_node = -1;

            ImGui::Indent(10.f);
            for (int i = 0; i < state.work_tree->children_n; i++) {
                show_tree_node(state, gui, state.work_tree->children[i], &selected_node, i == state.work_tree->children_n - 1);
            }
            ImGui::Unindent(10.f);

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

        ex::gui::next_auto_hide(docked || gui.view.docking);
        ImGui::SetNextWindowDockID(gui.docking.dock_left_top_id, cond_node());
        ImGui::Begin("Work tree", nullptr, reset_flag(gui) | ImGuiWindowFlags_NoCollapse);
        docked = ex::gui::window_is_multi_docked();
        watch = watch_parent_dock(gui);

        ImGui::BeginChild("WorkTreeBox", ImVec2(0, 0), 0, ImGuiWindowFlags_NoDecoration);
        work_tree_content(state, gui);
//        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();
        work_tree_buttons(state, gui);
        ImGui::EndChild();

        ImGui::End();
    }

}