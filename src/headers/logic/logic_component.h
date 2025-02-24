//
// Created by xd on 19/12/24.
//

#ifndef MAPPERS_H
#define MAPPERS_H

#include "../../../extra/nodes/nodes.h"
#include "../state.h"

namespace ex {
    namespace ln = limbo::nodes;

    struct LogicNodeComponent {
        ln::BaseNode *(*instance)(ex::TreeNode *from)                     = nullptr;
        ln::BaseNode *(*relocate)(ex::TreeNode *tree, ln::BaseNode *node) = nullptr;
        void (*destroy)(ex::TreeNode *tree, ln::BaseNode *node)           = nullptr;
    };

}

#endif //MAPPERS_H
