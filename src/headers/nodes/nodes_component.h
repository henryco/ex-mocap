//
// Created by xd on 19/12/24.
//

#ifndef MAPPERS_H
#define MAPPERS_H

#include "../state.h"
#include "../../../extra/nodes/nodes.h"

namespace ex {

    struct BaseNodeComponent {
        void (*from_base)(ex::State &state, nodes::BaseNode *from, ex::TreeNode *to)                    = nullptr;
        void (*from_tree)(ex::State &state, ex::TreeNode *from, nodes::BaseNode *to)                    = nullptr;
        nodes::BaseNode *(*instance)(const ex::State &state, ex::TreeNode *from)                        = nullptr;
        nodes::BaseNode *(*relocate)(const ex::State &state, ex::TreeNode *tree, nodes::BaseNode *node) = nullptr;
    };

}

#endif //MAPPERS_H
