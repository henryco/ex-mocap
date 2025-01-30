//
// Created by xd on 04/01/25.
//

#include "../../../headers/imgui/gui_layout.h"

namespace ex::gui {

    static std::map<std::size_t, NodeWindow<TreeNode> *> pinned_windows__;
    static NodeWindow<TreeNode>                         *active_window__ = nullptr;

    NodeWindow<TreeNode> *create_new_window(const State &state, TreeNode *node) {
        if (node == nullptr)
            return nullptr;

        if (node->is_link) {
            TreeNode             *source  = TreeNode::find_node(state.work_tree, node->id_link);
            NodeWindow<TreeNode> *new_win = new NodeWindow{ .node = (source == nullptr) ? node : source };
            new_win->name                 = new_win->node->name;
            return new_win;
        }

        return new NodeWindow{ .name = node->name, .node = node };
    }

    void update_active_window(const State &state, TreeNode *active_node) {
        if (active_node == nullptr && active_window__ == nullptr) {
            return;
        }

        if (active_node == nullptr || active_node->deleted || active_node->moved) {
            std::cout << "reset window" << '\n';
            if (active_window__ != nullptr && !active_window__->pinned)
                delete active_window__;
            active_window__ = nullptr;
            return;
        }

        if (!node_mappers_.contains(active_node->id_node)) {
            if (active_window__ != nullptr && !active_window__->pinned)
                delete active_window__;
            active_window__ = nullptr;
            return;
        }

        if (active_window__ == nullptr) {
            active_window__ = create_new_window(state, active_node);
            gen_uuid(active_window__);
            return;
        }

        if (active_window__->node != active_node) {
            if (!active_window__->pinned)
                delete active_window__;
            active_window__ = create_new_window(state, active_node);
            gen_uuid(active_window__);
        }
    }

    bool render_window(ex::State &state, GuiState &gui, NodeWindow<TreeNode> *window) {
        if (window == nullptr || window->node == nullptr || window->node->deleted || window->node->moved)
            return false;
        const auto n_type = window->node->id_node;
        if (!node_mappers_.contains(n_type))
            return false;

        if (window->node->name != window->name) {
            window->name = window->node->name;
            gen_uuid(window);
        }

        return node_mappers_.at(n_type).draw_node_window(gui, state, window);
    }

    void show_work_window(ex::State &state, GuiState &gui) {
        update_active_window(state, state.active_node);

        for (auto it = pinned_windows__.begin(); it != pinned_windows__.end();) {
            const auto window = it->second;
            if (window != nullptr && window->node == active_window__->node) {
                active_window__->visible = window->visible;
                active_window__->pinned  = window->pinned;
                active_window__->docked  = window->docked;
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
    }

}