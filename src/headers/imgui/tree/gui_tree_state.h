//
// Created by xd on 2/13/25.
//

#ifndef GUI_TREE_STATE_H
#define GUI_TREE_STATE_H

#include "../../entity.h"

namespace ex::gui::tree {

    struct selection_node {
        selection_node *root     = nullptr;
        selection_node *next     = nullptr;
        selection_node *prev     = nullptr;
        TreeNode       *selected = nullptr;

        int index = -1;

        selection_node        *copy() const;
        static selection_node *copy(selection_node *src);
        static selection_node *from_node(TreeNode *node);
        static bool            single(selection_node *node);
        static void            remove(selection_node *node, selection_node **next);
        static void            free(selection_node *node, bool unselect = false);
    };

    struct node_buffer {
        selection_node *selection = nullptr;
        bool            move      = false;
        bool            ref       = false;

        ~node_buffer();

        static void create(node_buffer **buffer, selection_node *selection, bool move = false, bool ref = false);
        static void create(node_buffer **buffer, TreeNode *node, bool move = false, bool ref = false);
        static void remove(node_buffer **buffer);
    };

    struct TreeState {
        selection_node *selection = nullptr;
        node_buffer    *buffer    = nullptr;

        bool force_collapse = false;
        bool force_expand   = false;

        ~TreeState();

        static void reset(TreeState *state);
        static void reset(TreeState &state);
    };

    TreeState &state();
}

#endif //GUI_TREE_STATE_H
