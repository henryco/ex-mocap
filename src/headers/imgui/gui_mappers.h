//
// Created by xd on 02/01/25.
//

#ifndef GUI_MAPPERS_H
#define GUI_MAPPERS_H

#include "gui_state.h"

namespace ex::gui {
    struct GuiMapper {
        bool (*draw_tree_node)(GuiState &gui, bool selected) = nullptr;
    };
}

#endif //GUI_MAPPERS_H
