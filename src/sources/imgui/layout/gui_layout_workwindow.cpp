//
// Created by xd on 04/01/25.
//

#include "../../../headers/imgui/gui_layout.h"

namespace ex::gui {

    void show_work_window(ex::State &state, GuiState &gui) {
        static bool prev_show = false;
        // TODO
//        if (state.active_node == nullptr || state.active_node->deleted)
//            return;
//
//        const auto n_type = state.active_node->id_node;
//        if (!node_mappers_.contains(n_type)) {
//            return;
//        }
//
//        ImGui::PushID(state.active_node->id_local);
//        {
//            const auto show = node_mappers_.at(n_type).draw_node_window(gui, state, state.active_node, !prev_show);
//            if (!show)
//                state.active_node = nullptr;
//            prev_show = show;
//        }
//        ImGui::PopID();
    }

}