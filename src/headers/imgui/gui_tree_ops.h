//
// Created by xd on 17/01/25.
//

#ifndef GUI_TREE_OPS_H
#define GUI_TREE_OPS_H

namespace ex::gui::tree {
    void rename_node(State &state, GuiState &gui, TreeNode *node, const char *name);

    void duplicate_node(State &state, GuiState &gui, TreeNode *node);

    void ref_node(State &state, GuiState &gui, TreeNode *node);

    void copy_selected(State &state, GuiState &gui, selection_node *selection);
    void copy_node(State &state, GuiState &gui, TreeNode *node);

    void cut_selected(State &state, GuiState &gui, selection_node *selection);
    void cut_node(State &state, GuiState &gui, TreeNode *node);

    void paste_after_node(State &state, GuiState &gui, TreeNode *node);
    void paste_into_node(State &state, GuiState &gui, TreeNode *node);

    void move_after_node(State &state, GuiState &gui, TreeNode *node, TreeNode *move);
    void move_into_node(State &state, GuiState &gui, TreeNode *node, TreeNode *move);

    void group_selected(State &state, GuiState &gui, selection_node *selection);
    void ungroup_selected(State &state, GuiState &gui, selection_node *selection);

    void delete_selected(State &state, GuiState &gui, selection_node *selection);
    void delete_node(State &state, GuiState &gui, TreeNode *node);

    void enable_selected(State &state, GuiState &gui, selection_node *selection);
    void enable_node(State &state, GuiState &gui, TreeNode *node);

    void disable_selected(State &state, GuiState &gui, selection_node *selection);
    void disable_node(State &state, GuiState &gui, TreeNode *node);

    void pick_inspected_node(State &state, GuiState &gui, TreeNode *node);
    void show_inspected_node(State &, GuiState &gui);

}

#endif //GUI_TREE_OPS_H
