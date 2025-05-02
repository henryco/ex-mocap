//
// Created by xd on 5/2/25.
//

#include "../../../../headers/nodes/components/sync/sync_logic.h"
#include "../../../../headers/logic/logic_nodes.h"

namespace ex::logic::node::sync {

    limbo::graph::BaseNode *SyncNode::instance(TreeNode *from) {
        SyncNode *node = new SyncNode();
        node->set_id(from->id_node);
        return node;
    }

    limbo::graph::BaseNode *SyncNode::relocate(TreeNode *, limbo::graph::BaseNode *node) {
        return node;
    }

    void SyncNode::destroy(TreeNode *, limbo::graph::BaseNode *node) {
        node->detach_all();
        delete node;
    }

    void SyncNode::on_input(int in_idx, int out_idx, int clock_i, const void *data) {

    }

    void SyncNode::on_update(int clock_i) {

    }

    void *SyncNode::on_output(int in_idx, int out_idx, int clock_i) {
        return nullptr;
    }

    SyncNode::~SyncNode() {

    }

    SyncNode::SyncNode() {

    }

}
