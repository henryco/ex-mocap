//
// Created by xd on 2/24/25.
//

#ifndef EX_IMGUI_OUTPUT_DISPLAY_H
#define EX_IMGUI_OUTPUT_DISPLAY_H

#include "../gui_nodes.h"

namespace ex::gui::node::out::display {

    unsigned long int get_node_type_id();

    bool draw_tree_node(TreeNode *node, bool nested, bool selected, bool collapse, bool expand);
    bool draw_node_window(GuiNodeWindow *window);
    const char *get_node_name();

    static constexpr GuiNodeMapper gui_mappers = { draw_tree_node, nullptr, draw_node_window, get_node_name };

}

#endif //EX_IMGUI_OUTPUT_DISPLAY_H
