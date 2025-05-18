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
        ocl::NBPromise   nb_promise;
        cl_command_queue queue_p;

        bool enabled;
    };

    struct serial_t {
        config_t config;
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

    void TransformNode::destroy(TreeNode *, limbo::graph::BaseNode *node) {
        node->detach_all();
        delete node;
    }

    void TransformNode::enable() {
        internal->enabled = true;
    }

    void TransformNode::disable() {
        internal->enabled = false;
        reset();
    }

    void TransformNode::reset() {

    }

    io::byte_array TransformNode::serialize() const {
        constexpr int total_size = sizeof(serial_t);

        serial_t parcel;
        parcel.config = config;

        unsigned char *bytes = static_cast<unsigned char *>(::malloc(total_size));
        ::memcpy(bytes, &parcel, total_size);

        return io::byte_array::from(bytes, total_size, &::free);
    }

    void TransformNode::deserialize(const io::byte_array &array) {
        if (array.empty())
            return;

        const serial_t *serial = reinterpret_cast<serial_t *>(array.data);
        memcpy(&config, &serial->config, sizeof(serial_t));
    }

    void TransformNode::on_input(const int in_idx, const int out_idx, const int clock_i, void *data) {
        if (!internal->enabled)
            return;
        // TODO
    }

    void TransformNode::on_update(const int clock_i) {
        if (!internal->enabled)
            return;
        // TODO
    }

    void *TransformNode::on_output(const int in_idx, const int out_idx, const int clock_i) {
        if (!internal->enabled)
            return nullptr;
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
