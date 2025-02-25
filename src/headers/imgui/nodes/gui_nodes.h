//
// Created by xd on 02/01/25.
//

#ifndef EX_IMGUI_NODE_COMPONENTS_H
#define EX_IMGUI_NODE_COMPONENTS_H

#include "../../state.h"

namespace ex::gui {

    struct GuiNodeWindow {
        const char *uuid    = nullptr;
        const char *name    = nullptr;
        TreeNode   *node    = nullptr;
        bool       *visible = nullptr;

        bool docked = false;
        bool pinned = false;
    };

    struct GuiNodeMapper {
        bool (*draw_tree_node)(TreeNode *node, bool nested, bool selected, bool collapse, bool expand) = nullptr;
        void (*draw_tree_menu)(TreeNode *node, bool nested, bool selected)                             = nullptr;
        void (*update_tree_node)(TreeNode *node)                                                       = nullptr;
        bool (*draw_node_window)(GuiNodeWindow *win)                                                   = nullptr;
        const char *(*get_node_name)()                                                                 = nullptr;
        const uint32_t (*get_dock_id)()                                                                = nullptr;
    };
}

#endif //EX_IMGUI_NODE_COMPONENTS_H
