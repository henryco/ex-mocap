//
// Created by xd on 04/01/25.
//

#include "../../../headers/imgui/gui_layout.h"

namespace ex::gui {

    void show_work_window(ex::State &state, GuiState &gui) {
        static NodeWindow<TreeNode> *active_window = nullptr;
        static bool prev_show = false;

        if (state.active_node == nullptr || state.active_node->deleted)
            return;

        const auto n_type = state.active_node->id_node;
        if (!node_mappers_.contains(n_type)) {
            return;
        }

        if (state.active_node == nullptr) {
            if (active_window != nullptr)
                delete active_window;
            active_window = nullptr;
        }
        else if (active_window == nullptr) {
            active_window = new NodeWindow{ .node = state.active_node };
            gen_uuid(active_window);
        }
        else if (active_window != nullptr && active_window->node != state.active_node) {
            delete active_window;
            active_window = new NodeWindow{ .node = state.active_node };
            gen_uuid(active_window);
        }

        const auto show = node_mappers_.at(n_type).draw_node_window(gui, state, active_window, !prev_show);
//        if (!show) {
//            delete gui.active_node;
//            gui.active_node = nullptr;
//        }
        prev_show = show;
    }

}