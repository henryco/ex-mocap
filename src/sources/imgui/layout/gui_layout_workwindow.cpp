//
// Created by xd on 04/01/25.
//

#include "../../../headers/imgui/gui_layout.h"

namespace ex::gui {

    static std::map<std::size_t, NodeWindow<TreeNode> *> pinned_windows__;
    static NodeWindow<TreeNode>                         *active_window__ = nullptr;

    void update_active_window(TreeNode *active_node) {
        if (active_node == nullptr || active_node->deleted) {
            if (active_window__ != nullptr)
                delete active_window__;
            active_window__ = nullptr;
            return;
        }

        if (!node_mappers_.contains(active_node->id_node)) {
            if (active_window__ != nullptr)
                delete active_window__;
            active_window__ = nullptr;
            return;
        }

        if (active_window__ == nullptr) {
            active_window__ = new NodeWindow{ .node = active_node };
            gen_uuid(active_window__);
            return;
        }

        if (active_window__->node != active_node) {
            delete active_window__;
            active_window__ = new NodeWindow{ .node = active_node };
            gen_uuid(active_window__);
        }
    }

    bool render_window(ex::State &state, GuiState &gui, NodeWindow<TreeNode> *window) {
        if (window == nullptr || window->node == nullptr || window->node->deleted)
            return false;
        const auto n_type = window->node->id_node;
        if (!node_mappers_.contains(n_type))
            return false;
        return node_mappers_.at(n_type).draw_node_window(gui, state, window);
    }

    void show_work_window(ex::State &state, GuiState &gui) {
        update_active_window(state.active_node);

        if (!render_window(state, gui, active_window__)) {
            if (active_window__ != nullptr) {
                if (active_window__->node != nullptr)
                    pinned_windows__.erase(active_window__->node->id_local);
                delete active_window__;
            }
            active_window__   = nullptr;
            state.active_node = nullptr;
        } else {
            active_window__->pinned |= !active_window__->docked;
            if (active_window__->pinned && !pinned_windows__.contains(active_window__->node->id_local)) {
                pinned_windows__.emplace(active_window__->node->id_local, active_window__);
            }
        }

        for (auto it = pinned_windows__.begin(); it != pinned_windows__.end();) {
            const auto window = it->second;
            if (window == active_window__) {
                ++it;
                continue;
            }
            if (!render_window(state, gui, window)) {
                if (window != nullptr)
                    delete window;
                it = pinned_windows__.erase(it);
            } else {
                ++it;
            }
        }
    }

}