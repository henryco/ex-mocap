//
// Created by xd on 04/01/25.
//

#include "../../../headers/imgui/gui_layout.h"

namespace ex::gui {

    void show_work_window(ex::State &state, GuiState &gui) {
        static std::map<std::size_t, NodeWindow<TreeNode> *> pinned_windows;
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

        if (active_window == nullptr)
            return;

        const auto show = node_mappers_.at(n_type).draw_node_window(gui, state, active_window, !prev_show);
        prev_show = show;
        if (!show) {
            delete active_window;
            state.active_node = nullptr;
            active_window     = nullptr;
        } else {
            active_window->pinned |= !active_window->docked;
            // TODO implement different logic (but for now its good enough)
        }

        // TODO
    }

}