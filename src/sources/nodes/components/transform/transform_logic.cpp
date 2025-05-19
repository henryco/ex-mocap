//
// Created by xd on 18/05/25.
//

#include "../../../../headers/nodes/components/transform/transform_logic.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/logic/logic_nodes.h"
#include "../../../../headers/cl/cl_kernel.h"

namespace ex::logic::node::transform {

    struct buffers_t {
        ocl::Image2D buffer[2];
        unsigned     index;
    };

    struct worker_t {
        ocl::NBPromise   nb_promise;
        cl_command_queue queue_p;
    };

    struct internal_t {
        buffers_t  input;
        buffers_t  output;
        config_t   state;
        worker_t   worker;
        id_type_16 image_id;
        bool       enabled;
    };

    struct serial_t {
        config_t   config;
        id_type_16 id;
    };

    static void buffers_swap(buffers_t *b) {
        b->index = (b->index + 1) % 2;
    }

    static void buffers_release(buffers_t *b) {
        b->buffer[0].release();
        b->buffer[1].release();
        b->index = 0;
    }

    static internal_t *create_internal() {
        internal_t *instance = static_cast<internal_t *>(::malloc(sizeof(internal_t)));
        ::memset(instance, 0, sizeof(internal_t));
        return instance;
    }

    static void destroy_internal(internal_t *ptr) {
        if (!ptr) return;

        buffers_release(&ptr->input);
        buffers_release(&ptr->output);

        ptr->worker.nb_promise.release();

        if (ptr->worker.queue_p != nullptr) {
            clReleaseCommandQueue(ptr->worker.queue_p);
            ptr->worker.queue_p = nullptr;
        }

        ::free(ptr);
    }

    limbo::graph::BaseNode *TransformNode::instance(TreeNode *from) {
        auto *instance = new TransformNode();
        instance->set_node_id(from->id_node);
        instance->set_image_id(from->id_node);
        return instance;
    }

    limbo::graph::BaseNode *TransformNode::relocate(TreeNode *, limbo::graph::BaseNode *node) {
        return node;
    }

    void TransformNode::destroy(TreeNode *, limbo::graph::BaseNode *node) {
        node->detach_all();
        delete node;
    }

    void TransformNode::set_image_id(const id_type_16 id) {
        internal->image_id = id;
        // TODO
    }

    id_type_16 TransformNode::get_image_id() const {
        return internal->image_id;
    }

    void TransformNode::enable() {
        internal->enabled = true;
    }

    void TransformNode::disable() {
        internal->enabled = false;
        reset();
    }

    void TransformNode::reset() {
        // TODO
    }

    io::byte_array TransformNode::serialize() const {
        constexpr int total_size = sizeof(serial_t);

        serial_t parcel;
        parcel.id     = internal->image_id;
        parcel.config = config;

        unsigned char *bytes = static_cast<unsigned char *>(::malloc(total_size));
        ::memcpy(bytes, &parcel, total_size);

        return io::byte_array::from(bytes, total_size, &::free);
    }

    void TransformNode::deserialize(const io::byte_array &array) {
        if (array.empty())
            return;

        const serial_t *serial = reinterpret_cast<serial_t *>(array.data);
        ::memcpy(&internal->image_id, &serial->id, sizeof(id_type_16));
        ::memcpy(&config, &serial->config, sizeof(serial_t));
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
