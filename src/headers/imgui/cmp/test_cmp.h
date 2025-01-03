//
// Created by xd on 03/01/25.
//

#ifndef TEST_CMP_H
#define TEST_CMP_H

#include "../gui_mappers.h"

namespace ex::gui::test {

    namespace one {
        bool draw_tree_node(GuiState &gui, const char *name, bool nested, bool selected);
        static constexpr GuiMapper mapper = { draw_tree_node };
    }

    namespace two {
        bool draw_tree_node(GuiState &gui, const char *name, bool nested, bool selected);
        static constexpr GuiMapper mapper = { draw_tree_node };
    }

}

#endif //TEST_CMP_H
