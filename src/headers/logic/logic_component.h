//
// Created by xd on 19/12/24.
//

#ifndef MAPPERS_H
#define MAPPERS_H

#include "../../../extra/nodes/nodes.h"
#include "../state.h"

namespace ex {

    struct LogicNodeComponent {
        void (*from_base)(ex::State &state, limbo::nodes::BaseNode *from, ex::TreeNode *to) = nullptr;
        void (*from_tree)(ex::State &state, ex::TreeNode *from, limbo::nodes::BaseNode *to) = nullptr;
        limbo::nodes::BaseNode *(*instance)(const ex::State &state, ex::TreeNode *from)     = nullptr;
        limbo::nodes::BaseNode *(*relocate)(const ex::State &state, ex::TreeNode *tree,
                                            limbo::nodes::BaseNode *node)                   = nullptr;
    };

}

#endif //MAPPERS_H
