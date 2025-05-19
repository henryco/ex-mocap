//
// Created by xd on 18/05/25.
//

#include "../../../../../kernels/transform.h"

#include "../../../../headers/nodes/components/transform/transform_logic.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/logic/logic_nodes.h"
#include "../../../../headers/cl/cl_kernel.h"

namespace ex::logic::node::transform {

    namespace {
        struct kernels_t {
            cl_program program;
            cl_kernel  kernel;
            unsigned   size_l;
            unsigned   counter;
        };

        kernels_t *kernel = nullptr;

        void create_kernel() {
            if (kernel == nullptr) {
                kernel = static_cast<kernels_t *>(::malloc(sizeof(kernels_t)));
                const ex::State &state = ex::logic::globals::state();
                kernel->program = xm::ocl::build_program(
                    state.cl.context, state.cl.device, ocl_kernel_transform_data,
                    ocl_kernel_transform_data_size, "transform.cl");
                kernel->kernel = xm::ocl::build_kernel(kernel->program, "kernel_transform");
                kernel->size_l = xm::ocl::optimal_local_size(state.cl.device, kernel->kernel);
                kernel->counter = 0;
            }
            kernel->counter++;
        }

        void release_kernel() {
            if (kernel == nullptr)
                return;
            if (--kernel->counter > 0)
                return;
            if (kernel->kernel != nullptr)
                ::clReleaseKernel(kernel->kernel);
            if (kernel->program != nullptr)
                ::clReleaseProgram(kernel->program);
            ::memset(kernel, 0, sizeof(kernels_t));
            ::free(kernel);
            kernel = nullptr;
        }
    }

    struct updates_t {
        limbo::graph::UpdateState state;
    };

    struct buffers_t {
        ocl::img_promise input;   // managed outside
        ocl::img_promise output;  // managed by none
        ocl::image2d_t   image;   // managed by this
    };

    struct internal_t {
        buffers_t  buffers;
        config_t   state;
        id_type_16 image_id;
        updates_t  updates;
        bool       enabled;
    };

    struct serial_t {
        config_t   config;
        id_type_16 id;
        bool       on;
    };

    static internal_t *create_internal() {
        internal_t *instance = static_cast<internal_t *>(::malloc(sizeof(internal_t)));
        ::memset(instance, 0, sizeof(internal_t));
        return instance;
    }

    static void destroy_internal(internal_t *ptr) {
        if (!ptr)
            return;
        ptr->buffers.image.release();
        ::free(ptr);
    }

    limbo::graph::BaseNode *TransformNode::instance(TreeNode *from) {
        auto *instance = new TransformNode();
        instance->set_node_id(from->id_node);
        instance->set_image_id(from->id_node);
        instance->enable();
        return instance;
    }

    limbo::graph::BaseNode *TransformNode::relocate(TreeNode *, limbo::graph::BaseNode *node) {
        if (auto *t = dynamic_cast<TransformNode *>(node))
            t->reset();
        return node;
    }

    void TransformNode::destroy(TreeNode *, limbo::graph::BaseNode *node) {
        node->detach_all();
        delete node;
    }

    void TransformNode::set_image_id(const id_type_16 id) {
        internal->image_id = id;
    }

    id_type_16 TransformNode::get_image_id() const {
        return internal->image_id;
    }

    void TransformNode::enable() {
        internal->enabled = true;
        reset();
    }

    void TransformNode::disable() {
        internal->enabled = false;
        reset();
    }

    void TransformNode::reset() {
        internal->buffers.image.release();
        internal->buffers.output.image = internal->buffers.image;
        internal->updates.state.refresh();
    }

    io::byte_array TransformNode::serialize() const {
        constexpr int total_size = sizeof(serial_t);

        serial_t parcel;
        parcel.id     = internal->image_id;
        parcel.on     = internal->enabled;
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

        if (serial->on) {
            enable();
            reset();
        } else {
            disable();
        }
    }

    void TransformNode::on_input(const int in_idx, const int out_idx, const int clock_i, void *data) {
        if (!internal->enabled)
            return;

        const ocl::img_promise *promise = static_cast<const ocl::img_promise *>(data);
        if (promise == nullptr) {
            reset();
            return;
        }

        internal->buffers.input = *promise;
    }

    void TransformNode::on_update(const int clock_i) {
        if (!internal->enabled)
            return;

        static const ex::State &state = ex::logic::globals::state();

        ocl::img_promise *input = &internal->buffers.input;
        ocl::image2d_t   *image = &internal->buffers.image;

        if (input->empty()) {
            reset();
            return;
        }

        if (input->image.header.fresh || image->detached())
            internal->updates.state.refresh();

        if (!internal->updates.state.update()) {
            internal->buffers.output.image.header.fresh = false;
            return;
        }

        if (input->image.size() != image->size() || image->detached() || image->empty()) {
            image->release();
            *image = ex::ocl::Image2D::allocate_like(input->image, state.cl.context);
        }

        {
            // TODO REPLACE WITH KERNELS
            ocl::promise_t promise = ex::ocl::ops::copy_data(*input, *image);
            internal->buffers.output.image              = *image;
            internal->buffers.output.queue              = promise.queue;
            internal->buffers.output.image.header.fresh = true;
            // TODO REPLACE WITH KERNELS
        }
    }

    void *TransformNode::on_output(const int in_idx, const int out_idx, const int clock_i) {
        if (!internal->enabled)
            return nullptr;

        ocl::img_promise *target = &internal->buffers.output;
        target->image.id         = internal->image_id;
        return target;
    }

    TransformNode::~TransformNode() {
        destroy_internal(internal);
        internal = nullptr;
        release_kernel();
    }

    TransformNode::TransformNode() :
        internal(create_internal()) {
        ::memcpy(&config, &internal->state, sizeof(config_t));
        create_kernel();
        reset();
    }
}
