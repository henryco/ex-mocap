//
// Created by xd on 02/01/25.
//

#ifndef GUI_MAPPERS_H
#define GUI_MAPPERS_H

#include "gui_state.h"

namespace ex::gui {
    struct GuiMapper {
        bool (*draw_tree_node)(GuiState &gui, const char *name, bool nested, bool selected) = nullptr;
        bool (*draw_node_window)(GuiState &gui, ex::State &state, TreeNode *node)           = nullptr;
    };
}

#endif //GUI_MAPPERS_H
