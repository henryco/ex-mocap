//
// Created by xd on 04/01/25.
//

#include "../../../headers/imgui/gui_globals.h"
#include "../../../headers/imgui/layout/gui_layout.h"
#include "../../../headers/logic/logic_globals.h"

#include <cstring>
#include <string>

namespace ex::gui {

    namespace ww {
        const auto &node_mappers_ = node_components();
    }

    static std::map<std::size_t, GuiNodeWindow<TreeNode> *> pinned_windows__;
    static GuiNodeWindow<TreeNode>                         *active_window__ = nullptr;

    void gen_uuid(GuiNodeWindow<TreeNode> *window) {
        const auto s_uuid =
                std::string(std::string(window->node->name) + "##" + std::to_string(window->node->id_local) + "/" +
                            std::to_string(window->node->id_node));
        const auto buffer = new char[s_uuid.size() + 1];
        strcpy(buffer, s_uuid.c_str());
        window->uuid = buffer;
    }

    GuiNodeWindow<TreeNode> *create_new_window(const State &state, TreeNode *node) {
        if (node == nullptr)
            return nullptr;

        if (node->state >>= node_state::is_link) {
            TreeNode                *source  = TreeNode::find_node(state.logic.work_tree, node->id_link);
            GuiNodeWindow<TreeNode> *new_win = new GuiNodeWindow{ .node = (source == nullptr) ? node : source };
            new_win->id                      = new_win->node->id_local;
            new_win->name                    = new_win->node->name;
            return new_win;
        }

        return new GuiNodeWindow{ .id = node->id_local, .name = node->name, .node = node };
    }

    void update_active_window(State &state, TreeNode *active_node) {
        if (active_node == nullptr && active_window__ == nullptr) {
            return;
        }

        if (active_window__ != nullptr && state.logic.dirty && active_window__->id >= 0) {

            active_window__->node = TreeNode::find_node(state.logic.work_tree, active_window__->id);
            if (active_window__->node == nullptr ||
                (active_window__->node->state >>= node_state::created) ||
                (active_window__->node->state >>= node_state::deleted))
            {
                if (!active_window__->pinned)
                    delete active_window__;
                active_window__         = nullptr;
                state.logic.active_node = nullptr;
                return;
            }

            state.logic.active_node = active_window__->node;
            return;
        }

        if (active_node == nullptr ||
            (active_node->state >>= node_state::created) ||
            (active_node->state >>= node_state::deleted))
        {
            if (active_window__ != nullptr && !active_window__->pinned)
                delete active_window__;
            active_window__ = nullptr;
            return;
        }

        if (!ww::node_mappers_.contains(active_node->id_node)) {
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

    bool render_window(ex::State &state, GuiState &gui, GuiNodeWindow<TreeNode> *window) {
        if (window == nullptr || window->node == nullptr ||
            (window->node->state >>= node_state::deleted) ||
            (window->node->state >>= node_state::moved))
        {
            return false;
        }

        if (state.logic.dirty && window->id >= 0) {
            window->node = TreeNode::find_node(state.logic.work_tree, window->id);
            if (window->node == nullptr)
                return false;
        }

        const auto n_type = window->node->id_node;
        if (!ww::node_mappers_.contains(n_type))
            return false;

        if (window->node->name != window->name) {
            window->name = window->node->name;
            gen_uuid(window);
        }

        return ww::node_mappers_.at(n_type).draw_node_window(gui, state, window);
    }

    void show_work_window() {
        State &state = ex::logic::globals::state();
        GuiState &gui = ex::gui::state();

        update_active_window(state, state.logic.active_node);

        for (auto it = pinned_windows__.begin(); it != pinned_windows__.end();) {
            const auto window = it->second;

            if (window != nullptr && state.logic.dirty && window->id >= 0) {
                window->node = TreeNode::find_node(state.logic.work_tree, window->id);
                if (window->node == nullptr) {
                    delete window;
                    it = pinned_windows__.erase(it);
                    continue;
                }
            }

            if (window != nullptr && active_window__ != nullptr && window->node == active_window__->node) {
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
                    pinned_windows__.erase(active_window__->id);
                delete active_window__;
            }
            active_window__         = nullptr;
            state.logic.active_node = nullptr;
        } else {
            active_window__->pinned |= !active_window__->docked;
            if (active_window__->pinned && !pinned_windows__.contains(active_window__->id)) {
                pinned_windows__.emplace(active_window__->id, active_window__);
            }
        }
    }

}