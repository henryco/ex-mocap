//
// Created by xd on 29/05/25.
//

#include "../../../../../kernels/filter_conv.h"

#include "../../../../headers/nodes/components/blur/blur_logic.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/cl/cl_kernel.h"

namespace ex::logic::node::blur {

    static const auto log = logs::get_logger("node_blur");

    struct updates_t {
        id_type_16                prev_h;
        limbo::graph::UpdateState state;
        limbo::graph::UpdateState input;
    };

    struct buffers_t {
        ocl::img_promise input;   // managed outside
        ocl::img_promise output;  // managed by none
        ocl::image2d_t   image;   // managed by this
    };

    struct kernels_t {
        cl_program program;
        cl_kernel  kernel;
        ::size_t   size_l;
        bool       enabled;
    };

    struct internal_t {
        buffers_t  buffers;
        config_t   state;
        kernels_t  kernels;
        id_type_16 image_id;
        updates_t  updates;
        bool       enabled;
    };

    struct serial_t {
        config_t   config;
        id_type_16 id;
        bool       on;
    };

    static void create_kernels(kernels_t *kernel) {
        log->debug("create blur cl kernels");
        kernel->kernel  = nullptr;
        kernel->program = nullptr;
        kernel->enabled = false;
        log->debug("blur cl kernels created");
    }

    static void update_kernels(internal_t *internal) {
        const ex::State &state = ex::logic::globals::state();
        log->debug("update blur cl kernels");

        kernels_t *kernel = &internal->kernels;
        config_t  *config = &internal->state;

        if (kernel->kernel != nullptr) {
            ::clReleaseKernel(kernel->kernel);
            kernel->kernel = nullptr;
        }

        if (kernel->program != nullptr) {
            ::clReleaseProgram(kernel->program);
            kernel->program = nullptr;
        }

        kernel->enabled = false;

        int         buff_l = 0;
        static char buff[512];
        ::memset(buff, 0, 512);

        // TODO COMPILE-TIME PARAMS

        buff[buff_l + 1] = '\0';

        log->debug("blur cl kernel parameters: {}", buff);

        ocl::image2d_t *image = &internal->buffers.image;
        image->release();
        *image = ex::ocl::Image2D::allocate(
                internal->buffers.input.image.cols,
                internal->buffers.input.image.rows,
                ocl::C_MODE::BGRA,
                ocl::C_DEPT::UINT_N8,
                state.cl.context);

        if (!kernel->enabled) {
            log->debug("blur cl kernels disabled (skip)");
            return;
        }

//        kernel->program = xm::ocl::build_program(
//                state.cl.context,
//                state.cl.device,
//                ocl_kernel_transform_data,
//                ocl_kernel_transform_data_size,
//                "transform.cl",
//                buff);
//        kernel->kernel = xm::ocl::build_kernel(kernel->program, "kernel_transform");
//        kernel->size_l = xm::ocl::optimal_local_size(state.cl.device, kernel->kernel);

        log->debug("blur cl kernels updated");
    }

    static void release_kernels(kernels_t *kernel) {
        if (kernel == nullptr)
            return;

        log->debug("release blur cl kernels");

        if (kernel->kernel != nullptr)
            ::clReleaseKernel(kernel->kernel);
        if (kernel->program != nullptr)
            ::clReleaseProgram(kernel->program);
        ::memset(kernel, 0, sizeof(kernels_t));

        log->debug("blur cl kernels released");
    }

    static void init_config(config_t *) {
        // TODO
    }

    static internal_t *create_internal() {
        log->debug("create blur node");

        internal_t *instance = static_cast<internal_t *>(::malloc(sizeof(internal_t)));
        if (instance == nullptr)
            abort_("cannot create blur node instance, nullptr");

        ::memset(instance, 0, sizeof(internal_t));
        create_kernels(&instance->kernels);
        init_config(&instance->state);

        log->debug("blur node created");
        return instance;
    }

    static void destroy_internal(internal_t *ptr) {
        if (!ptr)
            return;

        log->debug("destroying blur node");
        ptr->buffers.output.finalize(true);
        ptr->buffers.image.release();
        release_kernels(&ptr->kernels);
        ::free(ptr);
        log->debug("blur node destroyed");
    }

    static void reset(internal_t *internal) {
        internal->buffers.image.release();
        internal->updates.state.refresh();
        internal->buffers.output.image = internal->buffers.image;
    }

    limbo::graph::BaseNode *BlurNode::instance(TreeNode *from) {
        auto *instance = new BlurNode();
        instance->set_node_id(from->id_node);
        instance->set_image_id(from->id_node);
        instance->enable();
        return instance;
    }

    limbo::graph::BaseNode *BlurNode::relocate(TreeNode *, limbo::graph::BaseNode *node) {
        if (auto *t = dynamic_cast<BlurNode *>(node))
            reset(t->internal);
        return node;
    }

    void BlurNode::destroy(TreeNode *, limbo::graph::BaseNode *node) {
        node->detach_all();
        delete node;
    }

    void BlurNode::set_config(const config_t &) {
    }

    void BlurNode::set_image_id(id_type_16 id) {
        internal->image_id = id;
    }

    id_type_16 BlurNode::get_image_id() const {
        return internal->image_id;
    }

    id_type_16 BlurNode::get_input_id() const {
        return internal->buffers.input.image.id;
    }

    unsigned BlurNode::get_image_w() const {
        return internal->buffers.input.image.cols;
    }

    unsigned BlurNode::get_image_h() const {
        return internal->buffers.input.image.rows;
    }

    void BlurNode::enable() {
        reset(internal);
        internal->enabled = true;
    }

    void BlurNode::disable() {
        internal->enabled = false;
        reset(internal);
    }

    io::byte_array BlurNode::serialize() const {
        constexpr int total_size = sizeof(serial_t);

        serial_t parcel;
        parcel.id     = internal->image_id;
        parcel.on     = internal->enabled;
        parcel.config = internal->state;

        unsigned char *bytes = static_cast<unsigned char *>(::malloc(total_size));
        ::memcpy(bytes, &parcel, total_size);

        return io::byte_array::from(bytes, total_size, &::free);
    }

    void BlurNode::deserialize(const io::byte_array &array) {
        if (array.empty())
            return;

        const serial_t *serial = reinterpret_cast<serial_t *>(array.data);
        ::memcpy(&internal->image_id, &serial->id, sizeof(id_type_16));
        ::memcpy(&internal->state, &serial->config, sizeof(config_t));
        ::memcpy(&config, &serial->config, sizeof(config_t));

        if (serial->on) {
            enable();
        } else {
            disable();
        }
    }

    void BlurNode::on_input(const int in_idx, const int out_idx, const int clock_i, void *data) {
        if (!internal->enabled)
            return;

        ocl::img_promise *promise = static_cast<ocl::img_promise *>(data);
        if (promise == nullptr) {
            reset(internal);
            return;
        }

        if (promise->empty()) {
            reset(internal);
            return;
        }

        if (internal->buffers.input.image.cols != promise->image.cols ||
            internal->buffers.input.image.rows != promise->image.rows ||
            internal->updates.prev_h != promise->image.id) {

            internal->updates.prev_h  = promise->image.id;
            internal->buffers.input = *promise;

            if (!promise->empty()) {
                internal->updates.input.refresh();
                internal->updates.state.refresh();
                update_kernels(internal);
                ::memcpy(&config, &internal->state, sizeof(config_t));
            }
            return;
        }

        internal->updates.input.refresh();
        internal->buffers.input = *promise;
    }

    void BlurNode::on_update(int clock_i) {
        if (!internal->enabled)
            return;

        if (!internal->updates.input.update()) {
            reset(internal);
            return;
        }

        static const ex::State &state = ex::logic::globals::state();

        ocl::img_promise *input = &internal->buffers.input;
        ocl::image2d_t   *image = &internal->buffers.image;

        if (input->image.header.fresh || image->detached())
            internal->updates.state.refresh();

        if (!internal->updates.state.update()) {
            internal->buffers.output.image.header.fresh = false;
            return;
        }

        if (image->detached() || image->empty()) {
            update_kernels(internal);
            ::memcpy(&config, &internal->state, sizeof(config_t));
        }

        if (!internal->kernels.enabled) {
            internal->buffers.output.image              = input->image;
            internal->buffers.output.queue              = input->queue;
            internal->buffers.output.image.header.fresh = true;
            return;
        }

        const ::size_t pref_size = internal->kernels.size_l;
        ::size_t       l_size[2] = { pref_size, pref_size };
        ::size_t       g_size[2] = { xm::ocl::optimal_global_size(image->cols, pref_size),
                                     xm::ocl::optimal_global_size(image->rows, pref_size) };

        cl_mem buffer_in  = input->image.handle;
        cl_mem buffer_out = image->handle;

        xm::ocl::set_kernel_arg(internal->kernels.kernel, 0, sizeof(cl_mem), &buffer_in);
        xm::ocl::set_kernel_arg(internal->kernels.kernel, 1, sizeof(cl_mem), &buffer_out);

        // TODO RUNTIME PARAMETERS

        xm::ocl::enqueue_kernel_fast(
                input->queue, internal->kernels.kernel,
                2,
                g_size,
                l_size);

        internal->buffers.output = { *image, input->queue, false };
        internal->buffers.output.image.header.fresh = true;
    }

    void *BlurNode::on_output(int in_idx, int out_idx, int clock_i) {
        if (!internal->enabled)
            return nullptr;

        ocl::img_promise *target = &internal->buffers.output;
        target->image.id         = internal->image_id;
        return target;
    }

    BlurNode::~BlurNode() {
        destroy_internal(internal);
        internal = nullptr;
    }

    BlurNode::BlurNode() :
        internal(create_internal()) {
        ::memcpy(&config, &internal->state, sizeof(config_t));
        reset(internal);
    }
}