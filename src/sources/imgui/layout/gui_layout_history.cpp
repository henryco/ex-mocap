//
// Created by xd on 02/12/24.
//

#include "../../../headers/imgui/gui_layout.h"
#include <IconsLucide.h>

namespace ex::gui {

    namespace history {
        constexpr auto flags = ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg;
        constexpr float tree_spacing = 20.f;
        constexpr float ctrl_width   = 40.f;
        constexpr float min_width    = 100.f;

        constexpr auto tree_flags = ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_Leaf |
            ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    bool test_init_history() { // TODO REMOVE
        undo_history_.push("test 1", nullptr, nullptr);
        undo_history_.push("test 2", nullptr, nullptr);
        undo_history_.push("test 3", nullptr, nullptr);
        undo_history_.push("test 4", nullptr, nullptr);
        undo_history_.push("test 5", nullptr, nullptr);
        return true;
    }

    void history_tree(ex::State &state, GuiState &gui) {
        // TODO REMOVE
        static bool init_test = test_init_history();
        if (init_test)
            init_test = false;
        // TODO REMOVE

        ImGui::BeginChild(
                "HistorySpaceTree",
                ImVec2(0.0f, ImGui::GetContentRegionAvail().y - ImGui::CalcTextSize(ICON_LC_PLUS).y - ImGui::GetStyle().ItemSpacing.x - 10),
                0);

        if (undo_history_.current != nullptr) {
            ImGui::TextDisabled("%s", undo_history_.current->name);
        }

        if (ImGui::BeginTable("TableHistory", 1, history::flags)) {

            const auto old_indent_spacing = ImGui::GetStyle().IndentSpacing;
            ImGui::GetStyle().IndentSpacing = history::tree_spacing;

            const float fixed_width = ImGui::GetContentRegionAvail().x - history::ctrl_width - ImGui::GetStyle().ItemSpacing.x;
            const float items_width = std::max(history::min_width, fixed_width);

            ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthFixed, items_width);

            static int selected = -1;
            static int previous = -1;

            if (undo_history_.current != nullptr) {
                selected = static_cast<int>(undo_history_.current->id);
            }

            ImGui::Indent(10.f);

            auto head = undo_history_.records;
            while (head != nullptr) {

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                const auto c_flag = history::tree_flags | (selected == head->id ? ImGuiTreeNodeFlags_Selected : 0);
                ImGui::TreeNodeEx(head->name, c_flag);

                if (ImGui::IsItemClicked()) {
                    selected = head->id;
                }

                head = head->prev;
            }

            ImGui::Unindent(10.f);

            ImGui::GetStyle().IndentSpacing = old_indent_spacing;
            ImGui::EndTable();

            if (selected >= 0 && selected != previous) {
                undo_history_.move(static_cast<std::size_t>(selected));
                undo_history_.apply(&state);
            }
            previous = selected;
        }

        ImGui::EndChild();
    }

    void show_history_tree(ex::State &state, GuiState &gui) {
        static bool *watch = nullptr;
        if (watch != nullptr && !(*watch))
            return;

        static bool docked = false;
        static bool show   = true;

        show |= gui.view.reset;
        bool *ptr = (!docked || gui.view.docking) ? &show : nullptr;

        if (!show)
            return;

        ex::gui::next_auto_hide(docked || gui.view.docking);
        ImGui::SetNextWindowDockID(gui.docking.dock_left_bot_id, cond_node());
        ImGui::Begin("History", ptr, reset_flag(gui) | ImGuiWindowFlags_NoCollapse);
        docked = ex::gui::window_is_multi_docked();
        watch = watch_parent_dock(gui);

        ImGui::BeginChild("HistoryBox", ImVec2(0, 0), 0, ImGuiWindowFlags_NoDecoration);
        history_tree(state, gui);

        ImGui::EndChild();

        ImGui::End();
    }

}