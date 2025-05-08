//
// Created by xd on 5/2/25.
//

#ifndef EX_NODES_CMP_SYNC_GUI_H
#define EX_NODES_CMP_SYNC_GUI_H

#include "../../../imgui/gui_nodes.h"

namespace ex::gui::node::sync {

    unsigned long int get_node_type_id();

    bool draw_tree_node(TreeNode *node, bool nested, bool selected, bool collapse, bool expand);
    const char *get_node_preview();
    const char *get_node_name();

    static constexpr GuiNodeMapper gui_mappers = {
        &get_node_name,
        &get_node_preview,
        &draw_tree_node,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    };

}

#endif //EX_NODES_CMP_SYNC_GUI_H
