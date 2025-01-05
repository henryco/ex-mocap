//
// Created by xd on 04/01/25.
//

#include "../../../headers/imgui/gui_layout.h"

namespace ex::gui {

    void show_work_window(ex::State &state, GuiState &gui) {
        static bool prev_show = false;
        if (gui.active_node == nullptr || gui.active_node->node == nullptr || gui.active_node->node->deleted)
            return;

        const auto n_type = gui.active_node->node->id_node;
        if (!node_mappers_.contains(n_type)) {
            return;
        }

        if (gui.active_node->uuid == nullptr)
            gen_uuid(gui.active_node);

        const auto show = node_mappers_.at(n_type).draw_node_window(gui, state, gui.active_node, !prev_show);
//        if (!show) {
//            delete gui.active_node;
//            gui.active_node = nullptr;
//        }
        prev_show = show;
    }

}