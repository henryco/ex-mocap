//
// Created by xd on 5/2/25.
//

#ifndef EX_NODES_CMP_SYNC_LOGIC_H
#define EX_NODES_CMP_SYNC_LOGIC_H

#include "../../../logic/logic_nodes.h"

namespace ex::logic::node::sync {

    struct SyncNode final : limbo::graph::BaseNode,
                            limbo::graph::id::name<"sync_node">,
                            limbo::graph::io::out<int, 1, "Sync">
    {

        static limbo::graph::BaseNode *instance(TreeNode *from);
        static limbo::graph::BaseNode *relocate(TreeNode *tree, limbo::graph::BaseNode *node);
        static void                    destroy(TreeNode *tree, limbo::graph::BaseNode *node);

        void on_input(int in_idx, int out_idx, int clock_i, const void *data) override;

        void on_update(int clock_i) override;

        void *on_output(int in_idx, int out_idx, int clock_i) override;

        ~SyncNode() override;

        SyncNode();

    };

}

#endif //EX_NODES_CMP_SYNC_LOGIC_H
