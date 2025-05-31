//
// Created by xd on 31/05/25.
//

#ifndef EX_MOTION_MORPH_GUI_310525_170221_H
#define EX_MOTION_MORPH_GUI_310525_170221_H

#include "../../../imgui/gui_nodes.h"

namespace ex::gui::node::morph {

    unsigned long int get_node_type_id();

    const char *get_node_name();
    const char *get_node_preview();
    bool draw_tree_node(TreeNode *node, bool nested, bool selected, bool collapse, bool expand);
    bool draw_node_window(GuiNodeWindow *window);
    GuiNodeParcel *store(const TreeNode *node);
    void restore(TreeNode *node, GuiNodeParcel *parcel);
    void update_tree_node(const TreeNode *before, TreeNode *after);

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

#endif // EX_MOTION_MORPH_GUI_310525_170221_H
