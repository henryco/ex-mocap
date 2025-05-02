//
// Created by xd on 4/9/25.
//

#ifndef EX_NODES_CMP_IMAGE_GUI_H
#define EX_NODES_CMP_IMAGE_GUI_H

#include "../../../imgui/gui_nodes.h"

namespace ex::gui::node::image {

    unsigned long int get_node_type_id();

    bool draw_tree_node(TreeNode *node, bool nested, bool selected, bool collapse, bool expand);
    bool draw_node_window(GuiNodeWindow *window);
    void update_tree_node(const TreeNode *before, TreeNode *after);
    const char *get_node_preview();
    const char *get_node_name();
    GuiNodeParcel *store(const TreeNode *node);
    void restore(TreeNode *node, GuiNodeParcel *parcel);

    static constexpr GuiNodeMapper gui_mappers = {
        &get_node_name,
        &get_node_preview,
        &draw_tree_node,
        nullptr,
        &draw_node_window,
        &update_tree_node,
        nullptr,
        &store,
        &restore
    };
}

#endif //EX_NODES_CMP_IMAGE_GUI_H
