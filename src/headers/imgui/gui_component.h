//
// Created by xd on 02/01/25.
//

#ifndef GUI_MAPPERS_H
#define GUI_MAPPERS_H

#include "../state.h"

namespace ex::gui {

    struct GuiNodeWindow {
        const char *uuid    = nullptr;
        const char *name    = nullptr;
        TreeNode   *node    = nullptr;
        bool       *visible = nullptr;

        bool docked = false;
        bool pinned = false;
    };

    struct GuiNodeComponent {
        bool (*draw_tree_node)(TreeNode *node, bool nested, bool selected) = nullptr;
        void (*draw_tree_menu)(TreeNode *node, bool nested, bool selected) = nullptr;
        bool (*draw_node_window)(GuiNodeWindow *win)                       = nullptr;
        const char *(*get_node_name)()                                     = nullptr;
        const uint32_t (*get_dock_id)()                                    = nullptr;
    };
}

#endif //GUI_MAPPERS_H
