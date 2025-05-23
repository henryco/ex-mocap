//
// Created by xd on 2/24/25.
//

#ifndef EX_NODES_OUTPUT_DISPLAY_GUI_H
#define EX_NODES_OUTPUT_DISPLAY_GUI_H

#include "../../../imgui/gui_nodes.h"

namespace ex::gui::node::out::display {

    unsigned long int get_node_type_id();

    bool draw_tree_node(TreeNode *node, bool nested, bool selected, bool collapse, bool expand);
    bool draw_node_window(GuiNodeWindow *window);
    void update_tree_node(const TreeNode *before, TreeNode *after);
    const char *get_node_preview();
    const char *get_node_name();

    static constexpr GuiNodeMapper gui_mappers = {
        &get_node_name,
        &get_node_preview,
        &draw_tree_node,
        nullptr,
        &draw_node_window,
        &update_tree_node,
        nullptr
    };

}

#endif //EX_NODES_OUTPUT_DISPLAY_GUI_H
