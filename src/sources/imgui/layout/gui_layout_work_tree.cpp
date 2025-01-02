//
// Created by xd on 02/12/24.
//

#include <iostream>


#include "../../../headers/imgui/gui_layout.h"

namespace ex::gui {
    constexpr auto flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_BordersInnerV;
    constexpr auto tree_bull_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    constexpr auto tree_arr_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    constexpr auto tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;

    constexpr float ctrl_width  = 60.f;
    constexpr float min_width  = 100.f;

    void draw_brackets() {
        ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    }

    void show_tree_node(State &state, GuiState &gui, TreeNode *node, std::size_t *selected, const int n = 0) {

        if (!node->is_group) {
            // Not a group just regular element

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushID(node->id_local);

            const auto c_bullet = node->children_n > 0 ? 0 : tree_bull_flags;
            const auto c_flag = tree_node_flags | (*selected == node->id_local ? ImGuiTreeNodeFlags_Selected : 0);
            const bool open = ImGui::TreeNodeEx(node->name, c_flag | c_bullet);

            if (ImGui::IsItemClicked())
                *selected = node->id_local;

            ImGui::TableNextColumn();
            ImGui::TextDisabled("--");

            if (open && node->children_n > 0) {
                // Regular element but it has children
                const auto next = node->children[0];
                for (int i = 0; i < next->children_n; i++)
                    show_tree_node(state, gui, next->children[i], selected, n + 1);
                ImGui::TreePop();
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
            const auto c_flag = tree_node_flags | (*selected == node->id_local ? ImGuiTreeNodeFlags_Selected : 0);
            ImGui::TreeNodeEx(node->name, c_flag);

            if (ImGui::IsItemClicked())
                *selected = node->id_local;

            ImGui::TableNextColumn();
            ImGui::TextDisabled("");

            ImGui::PopID();
            return;
        }

        // It's a group, and it has children (not an empty)
        const auto first = node->children[0];

        if (first->children_n <= 0) {
            // Group with children, where the first element _HAS NO_ its own children

            ImGui::PushID(first->id_local);

            const ImVec2 outer_start = ImGui::GetCursorScreenPos();

            const auto c_flag = tree_node_flags | (*selected == first->id_local ? ImGuiTreeNodeFlags_Selected : 0);
            ImGui::TreeNodeEx(first->name, c_flag | ImGuiTreeNodeFlags_DefaultOpen | tree_bull_flags);

            const ImVec2 inner_start = ImGui::GetCursorScreenPos();

            if (ImGui::IsItemClicked())
                *selected = first->id_local;

            ImGui::TableNextColumn();
            ImGui::TextDisabled("");

            if (node->children_n > 1) {
                for (int i = 1; i < node->children_n; i++) {
                    show_tree_node(state, gui, node->children[i], selected, n + 1);
                }
            }

            ImGui::PopID();

            const ImVec2 inner_end = ImGui::GetCursorScreenPos();

            const auto p1 = ImVec2(inner_start.x, outer_start.y + 2.f);
            const auto p2 = ImVec2(inner_start.x, inner_end.y - 6.f);

            const auto p3 = ImVec2(p1.x + 5.f, p1.y);
            const auto p4 = ImVec2(p2.x + 5.f, p2.y);

            ImDrawList* draw_list = ImGui::GetForegroundDrawList();
            draw_list->AddLine(p1, p2, IM_COL32(0, 0, 0, 255), 1.2f);
            draw_list->AddLine(p1, p3, IM_COL32(0, 0, 0, 255), 1.2f);
            draw_list->AddLine(p2, p4, IM_COL32(0, 0, 0, 255), 1.2f);

            return;
        }

        // Group with children, where the first element _HAS_ its own children
        ImGui::PushID(node->id_local);
        const auto c_flag = tree_node_flags | (*selected == node->id_local ? ImGuiTreeNodeFlags_Selected : 0);
        const bool open = ImGui::TreeNodeEx(node->name, c_flag);

        if (ImGui::IsItemClicked())
            *selected = node->id_local;

        ImGui::TableNextColumn();
        ImGui::TextDisabled("");

        if (open) {
            for (int i = 0; i < node->children_n; i++) {
                ImGui::Unindent();
                show_tree_node(state, gui, node->children[i], selected, n + 1);
                ImGui::Indent();
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    void work_tree_content(State &state, GuiState &gui) {
        if (state.work_tree == nullptr)
            return;

        ImGui::BeginChild(
                "WorkTreeContent",
                ImVec2(0.0f, ImGui::GetContentRegionAvail().y - ImGui::CalcTextSize(ICON_LC_PLUS).y - ImGui::GetStyle().ItemSpacing.x - 20),
                0);

        if (ImGui::BeginTable("TableWorkTreeTable", 2, flags)) {
            const auto old_indent_spacing = ImGui::GetStyle().IndentSpacing;
            ImGui::GetStyle().IndentSpacing = 20.0f;

            const float fixed_width = ImGui::GetContentRegionAvail().x - ctrl_width - ImGui::GetStyle().ItemSpacing.x;
            const float items_width = std::max(min_width, fixed_width);

            ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthFixed, items_width);
            ImGui::TableSetupColumn("Controls", ImGuiTableColumnFlags_WidthFixed, ctrl_width);

            static std::size_t selected_node = -1;

            ImGui::Indent();
            for (int i = 0; i < state.work_tree->children_n; i++) {
                show_tree_node(state, gui, state.work_tree->children[i], &selected_node, 1);
            }
            ImGui::Unindent();

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