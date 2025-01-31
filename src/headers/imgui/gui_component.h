//
// Created by xd on 02/01/25.
//

#ifndef GUI_MAPPERS_H
#define GUI_MAPPERS_H

#include "../state.h"
#include "gui_state.h"

namespace ex::gui {

    template <typename NODE_>
    struct GuiNodeWindow {
        const char *uuid    = nullptr;
        const char *name    = nullptr;
        NODE_      *node    = nullptr;
        bool       *visible = nullptr;

        bool docked = false;
        bool pinned = false;
    };

    struct GuiNodeComponent {
        bool (*draw_tree_node)(GuiState &gui, ex::State &state, TreeNode *node, bool nested, bool selected) = nullptr;
        void (*draw_tree_menu)(GuiState &gui, ex::State &state, TreeNode *node, bool nested, bool selected) = nullptr;
        bool (*draw_node_window)(GuiState &gui, ex::State &state, GuiNodeWindow<TreeNode> *win)             = nullptr;
        const char *(*get_node_name)(GuiState &gui, ex::State &state)                                       = nullptr;
    };
}

#endif //GUI_MAPPERS_H
