//
// Created by xd on 19/12/24.
//

#ifndef MAPPERS_H
#define MAPPERS_H

#include "../nodes/nodes.h"
#include "../state.h"

namespace ex {
    struct DataMappers {
        void (*from_base)(nodes::BaseNode *from, TreeNode *to) = nullptr;
        void (*from_tree)(TreeNode *from, nodes::BaseNode *to) = nullptr;
        nodes::BaseNode *(*instance)(const State &state)       = nullptr;
    };
}

namespace ex::map::camera {
    void             from_base(nodes::BaseNode *from, TreeNode *to);
    void             from_tree(TreeNode *from, nodes::BaseNode *to);
    nodes::BaseNode *instance(const State &state);

    static constexpr DataMappers mappers = { from_base, from_tree, instance };
}

#endif //MAPPERS_H
