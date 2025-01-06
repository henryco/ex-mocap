//
// Created by xd on 04/01/25.
//

#include "../../../headers/imgui/gui_layout.h"

namespace ex::gui {

    void update_active_window(NodeWindow<TreeNode> **active_window, TreeNode *active_node) {
        if (active_node == nullptr || active_node->deleted) {
            if (*active_window != nullptr)
                delete *active_window;
            *active_window = nullptr;
            return;
        }

        if (!node_mappers_.contains(active_node->id_node)) {
            if (*active_window != nullptr)
                delete *active_window;
            *active_window = nullptr;
            return;
        }

        if (*active_window == nullptr) {
            *active_window = new NodeWindow{ .node = active_node };
            gen_uuid(*active_window);
            return;
        }

        if (*active_window != nullptr && (*active_window)->node != active_node) {
            delete *active_window;
            *active_window = new NodeWindow{ .node = active_node };
            gen_uuid(*active_window);
        }
    }

    void show_work_window(ex::State &state, GuiState &gui) {
        static std::map<std::size_t, NodeWindow<TreeNode> *> pinned_windows;
        static NodeWindow<TreeNode> *active_window = nullptr;

        update_active_window(&active_window, state.active_node);

        if (active_window == nullptr)
            return;

        const auto n_type = state.active_node->id_node;
        const auto show = node_mappers_.at(n_type).draw_node_window(gui, state, active_window);
        if (!show) {
            delete active_window;
            state.active_node = nullptr;
            active_window     = nullptr;
        } else {
            active_window->pinned |= !active_window->docked;
            // TODO implement different logic (but for now its good enough)
        }

//        if (active_window->pinned && !pinned_windows.contains(active_window->node->id_local)) {
//            pinned_windows.emplace(active_window->node->id_local, active_window);
//        }
        // TODO
    }

}