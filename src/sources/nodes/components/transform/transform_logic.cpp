//
// Created by xd on 18/05/25.
//

#include "../../../../headers/nodes/components/transform/transform_logic.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/logic/logic_nodes.h"
#include "../../../../headers/cl/cl_kernel.h"

namespace ex::logic::node::transform {

    struct internal_t {
        config_t         state;
        cl_command_queue queue_p;
    };

    static internal_t *create_internal() {
        internal_t *instance = static_cast<internal_t *>(::malloc(sizeof(internal_t)));
        ::memset(instance, 0, sizeof(internal_t));
        return instance;
    }

    static void destroy_internal(internal_t *ptr) {
        if (!ptr) return;
        ::free(ptr);
    }

    limbo::graph::BaseNode *TransformNode::instance(TreeNode *from) {
        auto *instance = new TransformNode();
        instance->set_node_id(from->id_node);
        return instance;
    }

    limbo::graph::BaseNode *TransformNode::relocate(TreeNode *, limbo::graph::BaseNode *node) {
        return node;
    }

    void TransformNode::destroy(TreeNode *tree, limbo::graph::BaseNode *node) {
        // TODO
        node->detach_all();
        delete node;
    }

    io::byte_array TransformNode::serialize() const {
        // TODO
        return io::byte_array::from(nullptr, 0);
    }

    void TransformNode::deserialize(const io::byte_array &) {
        // TODO
    }

    void TransformNode::on_input(const int in_idx, const int out_idx, const int clock_i, void *data) {
        // TODO
    }

    void TransformNode::on_update(const int clock_i) {
        // TODO
    }

    void *TransformNode::on_output(const int in_idx, const int out_idx, const int clock_i) {
        // TODO
        return nullptr;
    }

    TransformNode::~TransformNode() {
        destroy_internal(internal);
        internal = nullptr;
    }

    TransformNode::TransformNode() :
        internal(create_internal()) {
        ::memcpy(&config, &internal->state, sizeof(config_t));
    }
}
