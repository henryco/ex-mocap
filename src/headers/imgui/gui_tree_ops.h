//
// Created by xd on 17/01/25.
//

#ifndef GUI_TREE_OPS_H
#define GUI_TREE_OPS_H

namespace ex::gui::tree {

    void copy_selected(State &state, GuiState &gui, selection_node *selection);
    void cut_selected(State &state, GuiState &gui, selection_node *selection);
    void delete_selected(State &state, GuiState &gui, selection_node *selection);

    void copy_node(State &state, GuiState &gui, TreeNode *node);
    void cut_node(State &state, GuiState &gui, TreeNode *node);
    void ref_node(State &state, GuiState &gui, TreeNode *node);

    void paste_after_node(State &state, GuiState &gui, TreeNode *node);
    void paste_into_node(State &state, GuiState &gui, TreeNode *node);

    void duplicate_node(State &state, GuiState &gui, TreeNode *node);

    void enable_node(State &state, GuiState &gui, TreeNode *node);
    void disable_node(State &state, GuiState &gui, TreeNode *node);

    void delete_node(State &state, GuiState &gui, TreeNode *node);

}

#endif //GUI_TREE_OPS_H
