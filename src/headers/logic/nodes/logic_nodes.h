//
// Created by xd on 19/12/24.
//

#ifndef EX_LOGIC_NODE_COMPONENT_H
#define EX_LOGIC_NODE_COMPONENT_H

#include "../../../../extra/graph/graph.h"
#include "../../entity.h"

namespace ex::logic {
    namespace ln = limbo::graph;

    struct NodeMapper {
        ln::BaseNode *(*instance)(ex::TreeNode *from)                     = nullptr;
        ln::BaseNode *(*relocate)(ex::TreeNode *tree, ln::BaseNode *node) = nullptr;
        void (*destroy)(ex::TreeNode *tree, ln::BaseNode *node)           = nullptr;
    };

}

#endif //EX_LOGIC_NODE_COMPONENT_H
