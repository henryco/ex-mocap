//
// Created by xd on 04/01/25.
//

#include "../../../headers/imgui/gui_layout.h"

namespace ex::gui {

    void show_work_window(ex::State &state, GuiState &gui) {
        static bool prev_show = false;
        if (state.active_node == nullptr || state.active_node->deleted)
            return;

        const auto n_type = state.active_node->id_node;
        if (!node_mappers_.contains(n_type)) {
            return;
        }

        ex::gui::next_no_close();
        ImGui::SetNextWindowDockID(gui.docking.dock_right_id, (gui.view.reset || !prev_show) ? ImGuiCond_Always : ImGuiCond_Once);

        const auto show = node_mappers_.at(n_type).draw_node_window(gui, state, state.active_node);
        if (!show)
            state.active_node = nullptr;
        prev_show = show;
    }

}