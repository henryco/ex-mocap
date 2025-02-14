//
// Created by xd on 17/01/25.
//

#ifndef GUI_TREE_OPS_H
#define GUI_TREE_OPS_H

#include "gui_tree_state.h"

namespace ex::gui::tree {

    void rename_node(TreeNode *node, const char *name);

    void duplicate_node(TreeNode *node);

    void ref_node(TreeNode *node);

    void add_node(TreeNode *add);

    void add_into_node(TreeNode *node, TreeNode *add);
    void add_after_node(TreeNode *node, TreeNode *add);

    void copy_selected(selection_node *selection);
    void copy_node(TreeNode *node);

    void cut_selected(selection_node *selection);
    void cut_node(TreeNode *node);

    void paste_after_node(TreeNode *node);
    void paste_into_node(TreeNode *node);

    void move_after_node(TreeNode *node, TreeNode *move);
    void move_into_node(TreeNode *node, TreeNode *move);

    void group_selected(selection_node *selection);
    void ungroup_selected(selection_node *selection);

    void delete_selected(selection_node *selection);
    void delete_node(TreeNode *node);

    void enable_selected(selection_node *selection);
    void enable_node(TreeNode *node);

    void disable_selected(selection_node *selection);
    void disable_node(TreeNode *node);

    void pick_inspected_node(TreeNode *node);
    void show_inspected_node();

}

#endif //GUI_TREE_OPS_H
