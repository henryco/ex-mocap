//
// Created by henryco on 14/02/25.
//

#ifndef GUI_TREE_MENU_H
#define GUI_TREE_MENU_H

#include "../../state.h"

namespace ex::gui::tree {

    bool show_context_menu_group();

    bool show_context_menu_element(TreeNode *node, bool last, bool nested, bool selected);

    bool show_context_menu_general();

}

#endif //GUI_TREE_MENU_H
