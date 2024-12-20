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
        void (*from_base)(State &state, node_ *from, TreeNode *to) = nullptr;
        void (*from_tree)(State &state, TreeNode *from, node_ *to) = nullptr;
        node_ *(*update)(State &state, TreeNode *from, node_ *src) = nullptr;
        node_ *(*instance)(const State &state)                     = nullptr;
    };
}

namespace ex::map::camera {
    void   from_base(State &state, node_ *from, TreeNode *to);
    void   from_tree(State &state, TreeNode *from, node_ *to);
    node_ *update(State &state, TreeNode *from, node_ *src);
    node_ *instance(const State &state);

    static constexpr DataMappers mappers = { from_base, from_tree, update, instance };
}

#endif //MAPPERS_H
