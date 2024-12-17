//
// Created by xd on 02/12/24.
//

#include "../../../headers/imgui/gui_layout.h"

namespace ex::gui {
    constexpr auto flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_BordersInnerV;
    constexpr auto tree_bull_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    constexpr auto tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;

    constexpr float ctrl_width  = 60.f;
    constexpr float min_width  = 100.f;

    void work_tree_content(ex::State &state, GuiState &gui) {
        ImGui::BeginChild(
                "WorkTreeContent",
                ImVec2(0.0f, ImGui::GetContentRegionAvail().y - ImGui::CalcTextSize(ICON_LC_PLUS).y - ImGui::GetStyle().ItemSpacing.x - 10),
                0);
        // TODO

        if (ImGui::BeginTable("TableWorkTreeTable", 2,flags)) {
            const auto old_indent_spacing = ImGui::GetStyle().IndentSpacing;
            ImGui::GetStyle().IndentSpacing = 10.0f;

            const float fixed_width = ImGui::GetContentRegionAvail().x - ctrl_width - ImGui::GetStyle().ItemSpacing.x;
            const float items_width = std::max(min_width, fixed_width);

            ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthFixed, items_width);
            ImGui::TableSetupColumn("Controls", ImGuiTableColumnFlags_WidthFixed, ctrl_width);

            for (int i = 0; i < 5; i++) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                const bool open = ImGui::TreeNodeEx(("Node: " + std::to_string(i)).c_str(), tree_node_flags);

                ImGui::TableNextColumn();
                ImGui::TextDisabled("--");

                if (open) {
                    for (int n = 0; n < 3; n++) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::TreeNodeEx(("Inner" + std::to_string(n)).c_str(), tree_node_flags | tree_bull_flags);

                        ImGui::TableNextColumn();
                        ImGui::TextDisabled("++");
                    }
                    ImGui::TreePop();
                }
            }

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