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

    struct GuiNodeParcel {
        unsigned long  size           = 0;
        unsigned char *data           = nullptr;
        void (*free)(GuiNodeParcel *) = nullptr;
    };

    struct GuiNodeMapper {

        /**
         * Node name
         */
        const char *(*get_node_name)()                                                                 = nullptr;

        /**
         * Preview to be shown in context menu / drag & drop / etc.
         */
        const char *(*get_node_preview)()                                                              = nullptr;

        /**
         * TreeNode view in work-tree
         */
        bool (*draw_tree_node)(TreeNode *node, bool nested, bool selected, bool collapse, bool expand) = nullptr;

        /**
         * TreeNode context menu
         */
        void (*draw_tree_menu)(TreeNode *node, bool nested, bool selected)                             = nullptr;

        /**
         * Window content (main content)
         */
        bool (*draw_node_window)(GuiNodeWindow *win)                                                   = nullptr;

        /**
         * Callback when TreeNode is updated
         */
        void (*update_tree_node)(const TreeNode *before, TreeNode *after)                              = nullptr;

        /**
         * Default dock id
         */
        const uint32_t (*get_dock_id)()                                                                = nullptr;

        /**
         * Serialize data (copy/duplicate/etc)
         */
        GuiNodeParcel *(*store)(TreeNode *node)                                                        = nullptr;

        /**
         * Deserialize data (paste/duplicate/etc)
         */
        void (*restore)(TreeNode *node, GuiNodeParcel *parcel)                                         = nullptr;
    };
}

#endif //EX_IMGUI_NODE_COMPONENTS_H
