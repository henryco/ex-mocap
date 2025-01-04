//
// Created by xd on 03/01/25.
//

#ifndef TEST_CMP_H
#define TEST_CMP_H

#include "../gui_mappers.h"

namespace ex::gui::test {

    namespace one {
        bool draw_tree_node(GuiState &gui, const char *name, bool nested, bool selected);
        bool draw_node_window(GuiState &gui, ex::State &state, TreeNode *node);
        static constexpr GuiMapper mapper = { draw_tree_node, draw_node_window };
    }

    namespace two {
        bool draw_tree_node(GuiState &gui, const char *name, bool nested, bool selected);
        bool draw_node_window(GuiState &gui, ex::State &state, TreeNode *node);
        static constexpr GuiMapper mapper = { draw_tree_node, draw_node_window };
    }

    namespace tri {
        bool draw_tree_node(GuiState &gui, const char *name, bool nested, bool selected);
        bool draw_node_window(GuiState &gui, ex::State &state, TreeNode *node);
        static constexpr GuiMapper mapper = { draw_tree_node, draw_node_window };
    }

}

#endif //TEST_CMP_H
