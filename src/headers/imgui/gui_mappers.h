//
// Created by xd on 02/01/25.
//

#ifndef GUI_MAPPERS_H
#define GUI_MAPPERS_H


#include <cstring>
#include <string>

#include "../state.h"
#include "gui_state.h"

namespace ex::gui {

    template <typename NODE_>
    struct NodeWindow {
        const char *uuid    = nullptr;
        const char *name    = nullptr;
        NODE_      *node    = nullptr;
        bool       *visible = nullptr;

        bool docked = false;
        bool pinned = false;
    };

    inline void gen_uuid(NodeWindow<TreeNode> *window) {
        const auto s_uuid =
                std::string(std::string(window->node->name) + "##" + std::to_string(window->node->id_local) + "/" +
                            std::to_string(window->node->id_node));

        const auto buffer = new char[s_uuid.size() + 1];
        strcpy(buffer, s_uuid.c_str());
        window->uuid = buffer;
    }

    struct GuiMappers {
        bool (*draw_tree_node)(GuiState &gui, ex::State &state, TreeNode *node, bool nested, bool selected) = nullptr;
        void (*draw_tree_menu)(GuiState &gui, ex::State &state, TreeNode *node, bool nested, bool selected) = nullptr;
        bool (*draw_node_window)(GuiState &gui, ex::State &state, NodeWindow<TreeNode> *win)                = nullptr;
    };
}

#endif //GUI_MAPPERS_H
