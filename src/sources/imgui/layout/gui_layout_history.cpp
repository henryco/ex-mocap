//
// Created by xd on 02/12/24.
//

#include "../../../headers/imgui/gui_layout.h"
#include <IconsLucide.h>

namespace ex::gui {

    bool test_init_history() { // TODO REMOVE
        undo_history_.push("test 1", nullptr);
        undo_history_.push("test 2", nullptr);
        undo_history_.push("test 3", nullptr);
        undo_history_.push("test 4", nullptr);
        undo_history_.push("test 5", nullptr);
        return true;
    }

    void history_tree(ex::State &state, GuiState &gui) {
        ImGui::BeginChild(
                "HistorySpaceTree",
                ImVec2(0.0f, ImGui::GetContentRegionAvail().y - ImGui::CalcTextSize(ICON_LC_PLUS).y - ImGui::GetStyle().ItemSpacing.x - 10),
                0);
        // TODO
        ImGui::Text("test undo");
        ImGui::EndChild();
    }

    void show_history_tree(ex::State &state, GuiState &gui) {
        // TODO REMOVE
        static bool init_test = test_init_history();
        if (init_test)
            init_test = false;
        // TODO REMOVE

        static bool *watch = nullptr;
        if (watch != nullptr && !(*watch))
            return;

        static bool docked = false;
        ex::gui::next_auto_hide(docked || gui.view.docking);
        ImGui::SetNextWindowDockID(gui.docking.dock_left_bot_id, cond_node());
        ImGui::Begin("History", nullptr, reset_flag(gui) | ImGuiWindowFlags_NoCollapse);
        docked = ex::gui::window_is_multi_docked();
        watch = watch_parent_dock(gui);

        ImGui::BeginChild("HistoryBox", ImVec2(0, 0), 0, ImGuiWindowFlags_NoDecoration);
        history_tree(state, gui);
        ImGui::Spacing();
        ImGui::Spacing();
        // TODO ?
        ImGui::EndChild();

        ImGui::End();
    }

}