//
// Created by xd on 19/12/24.
//

#ifndef MAPPERS_H
#define MAPPERS_H

#include "../nodes/nodes.h"
#include "../state.h"

namespace ex {
    using node_ = nodes::BaseNode;

    struct DataMappers {
        void (*from_base)(State &state, node_ *from, TreeNode *to)          = nullptr;
        void (*from_tree)(State &state, TreeNode *from, node_ *to)          = nullptr;
        node_ *(*instance)(const State &state, TreeNode *from)              = nullptr;
        node_ *(*relocate)(const State &state, TreeNode *tree, node_ *node) = nullptr;
    };
}

namespace ex::map::camera {
    void   from_base(State &state, node_ *from, TreeNode *to);
    void   from_tree(State &state, TreeNode *from, node_ *to);
    node_ *instance(const State &state, TreeNode *from);
    node_ *relocate(const State &state, TreeNode *tree, node_ *node);

    static constexpr DataMappers mappers = { from_base, from_tree, instance };
}

#endif //MAPPERS_H
