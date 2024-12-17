//
// Created by xd on 02/12/24.
//

#include "../../../headers/imgui/gui_layout.h"

namespace ex::gui {

    void work_tree_content(ex::State &state, GuiState &gui) {
        ImGui::BeginChild(
                "WorkTreeContent",
                ImVec2(0.0f, ImGui::GetContentRegionAvail().y - ImGui::CalcTextSize(ICON_LC_PLUS).y - ImGui::GetStyle().ItemSpacing.x - 10),
                0);
        // TODO

        ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_BordersInnerV;
        ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;

        if (ImGui::BeginTable("TableWithTreeNodes", 2,flags)) {
            // Disable table headers
            ImGui::TableSetupScrollFreeze(0, 0); // Disable freezing rows/columns

            // Row 1
            ImGui::TableNextRow();

            // Column 1: Tree Node 1
            ImGui::TableSetColumnIndex(0);
            if (ImGui::TreeNode("Node 1"))
            {
                ImGui::Text("Child Item 1");
                ImGui::TreePop();
            }

            // Column 2: Some Content
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Content for Node 1");

            // Row 2
            ImGui::TableNextRow();

            // Column 1: Tree Node 2
            ImGui::TableSetColumnIndex(0);
            if (ImGui::TreeNode("Node 2"))
            {
                ImGui::Text("Child Item 2");
                ImGui::TreePop();
            }

            // Column 2: Some Content
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Content for Node 2");

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