//
// Created by xd on 01/06/25.
//

#include "../../../../../kernels/chroma_key.h"
#include "../../../../../kernels/morph.h"
#include "../../../../../kernels/blur.h"

#include "../../../../headers/nodes/components/chromakey/chromakey_logic.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/cl/cl_kernel.h"

namespace ex::logic::node::chromakey {

    #define HARD_CAP (20U)
    #define CAP_SIZE ((HARD_CAP * 2) + 1)

    static const auto log = logs::get_logger("node_chromakey");

    struct updates_t {
        id_type_16                prev_h;
        limbo::graph::UpdateState state;
        limbo::graph::UpdateState input;
    };

    struct buffers_t {
        ocl::img_promise input;   // managed outside
        ocl::img_promise output;  // managed by none

        ocl::image2d_t   aux_1;   // managed by this
        ocl::image2d_t   aux_2;   // managed by this
        ocl::image2d_t   image;   // managed by this
    };

    struct kernels_t {
        cl_program program_chroma;
        cl_program program_morph;
        cl_program program_blur;

        cl_kernel  kernel_chroma[3];
        cl_kernel  kernel_morph[2];
        cl_kernel  kernel_blur[2];

        ::size_t   size_l_chroma;
        ::size_t   size_l_morph;
        ::size_t   size_l_blur;

        cl_mem     conv_k;

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

    static unsigned short mimp(const unsigned u) {
        return u > 0 ? u : 1;
    }

    static void create_g_kernel_1d(const unsigned size, const float sigma, float *out_kernel) {
        const int half = static_cast<int>(size / 2);
        double sum = 0.f;

        for (int i = 0, x = -half; x <= half; ++x, ++i) {
            const double e = -(x * x) / (2.f * sigma * sigma);
            const double f = ::exp(e);
            out_kernel[i] = f;
            sum += f;
        }

        for (int i = 0; i < size; ++i)
            out_kernel[i] /= sum;
    }

    static void create_kernels(kernels_t *kernel) {
        log->debug("create chromakey cl kernels");
        kernel->enabled = false;
        log->debug("chromakey cl kernels created");
    }

    static void release_kernels(kernels_t *kernel) {
        if (kernel == nullptr)
            return;

        log->debug("release chromakey cl kernels");

        for (int i = 0; i < 3; ++i) {
            if (kernel->kernel_chroma[i] != nullptr)
                ::clReleaseKernel(kernel->kernel_chroma[i]);
        }

        for (int i = 0; i < 2; ++i) {
            if (kernel->kernel_morph[i] != nullptr)
                ::clReleaseKernel(kernel->kernel_morph[i]);
        }

        for (int i = 0; i < 2; ++i) {
            if (kernel->kernel_blur[i] != nullptr)
                ::clReleaseKernel(kernel->kernel_blur[i]);
        }

        if (kernel->program_chroma != nullptr)
            ::clReleaseProgram(kernel->program_chroma);

        if (kernel->program_morph != nullptr)
            ::clReleaseProgram(kernel->program_morph);

        if (kernel->program_blur != nullptr)
            ::clReleaseProgram(kernel->program_blur);

        if (kernel->conv_k != nullptr)
            ::clReleaseMemObject(kernel->conv_k);

        ::memset(kernel, 0, sizeof(kernels_t));

        log->debug("chromakey cl kernels released");
    }

    static void update_kernels(internal_t *internal) {
        const ex::State &state = ex::logic::globals::state();
        log->debug("update chromakey cl kernels");

        kernels_t *kernel = &internal->kernels;
        config_t  *config = &internal->state;

        release_kernels(kernel);

        int         buff_l = 0;
        static char buff[512];
        ::memset(buff, 0, 512);

        const unsigned short image_w = internal->buffers.input.image.cols;
        const unsigned short image_h = internal->buffers.input.image.rows;
        const unsigned short mask_w  = static_cast<float>(image_w) / config->upscale.scale;
        const unsigned short mask_h  = static_cast<float>(image_h) / config->upscale.scale;

        ocl::image2d_t *image = &internal->buffers.image;
        image->release();

        ocl::image2d_t *aux_1 = &internal->buffers.aux_1;
        aux_1->release();

        ocl::image2d_t *aux_2 = &internal->buffers.aux_2;
        aux_2->release();

        *image = ex::ocl::Image2D::allocate(
                image_w,
                image_h,
                ocl::C_MODE::BGRA,
                ocl::C_DEPT::UINT_N8,
                state.cl.context);

        *aux_1 = ex::ocl::Image2D::allocate(
                mask_w,
                mask_h,
                ocl::C_MODE::BGRA,
                ocl::C_DEPT::UINT_N8,
                state.cl.context);

        *aux_2 = ex::ocl::Image2D::allocate(
                mask_w,
                mask_h,
                ocl::C_MODE::BGRA,
                ocl::C_DEPT::UINT_N8,
                state.cl.context);

        // blur -> mask -> morph -> apply

        const bool has_scale = config->upscale.scale > 1.f;
        const bool has_blur  = config->before.blur && config->before.power > 0;
        const bool has_morph = config->refine.morph != mode_e::NONE && config->refine.power > 0;

        if (has_blur) {
            ::memset(buff, 0, 512);

            buff_l += sprintf(buff + buff_l, " -DUSE_KERN_H_S=%hhu ", config->before.power);
            if (has_scale) {
                buff_l += sprintf(buff + buff_l, " -DUSE_RESIZE=1 ");
                if (config->upscale.linear)
                    buff_l += sprintf(buff + buff_l, " -DUSE_RESIZE_LIN=1 ");
            }

            static thread_local float ker_buff[CAP_SIZE * CAP_SIZE];

            const unsigned int side_size = (config->before.power * 2) + 1;
            const cl_mem_flags flags     = CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR;

            create_g_kernel_1d(side_size, 5, ker_buff);

            cl_int err;
            kernel->conv_k = ::clCreateBuffer(state.cl.context, flags,  sizeof(float) * side_size, ker_buff, &err);
            if (err != CL_SUCCESS)
                abort_("Cannot create kernel buffer: " << err);

            if (logs::get_level() < spdlog::level::info) {
                for (int i = 0; i < side_size; ++i)
                    log->debug("blur kernel_item [{}]: {}", i, ker_buff[i]);
            }

            buff[buff_l + 1] = '\0';

            log->debug("chromakey(blur) cl kernel parameters: {}", buff);

            kernel->program_blur = xm::ocl::build_program(
                    state.cl.context,
                    state.cl.device,
                    ocl_kernel_blur_data,
                    ocl_kernel_blur_data_size,
                    "blur.cl",
                    buff);

            kernel->kernel_blur[0] = xm::ocl::build_kernel(kernel->program_blur, "kernel_blur_horizontal");
            kernel->kernel_blur[1] = xm::ocl::build_kernel(kernel->program_blur, "kernel_blur_vertical");
            kernel->size_l_blur    = xm::ocl::optimal_local_size(state.cl.device, kernel->kernel_blur[0]);
        }

        if (has_morph) {

        }

        if (config->upscale.linear) {
            buff_l += sprintf(buff + buff_l, " -DUSE_RESIZE_LIN=1 ");
        }

        buff_l += sprintf(buff + buff_l, " -DUSE_IMAGE_W=%hu ", mimp(image_w - 1));
        buff_l += sprintf(buff + buff_l, " -DUSE_IMAGE_H=%hu ", mimp(image_h - 1));

        // TODO

        buff[buff_l + 1] = '\0';

        log->debug("chromakey cl kernel parameters: {}", buff);

        if (!kernel->enabled) {
            log->debug("chromakey cl kernels disabled (skip)");
            return;
        }

        log->debug("chromakey cl kernels updated");
    }

    static void init_config(config_t *c) {
        c->upscale.linear = false;
        c->upscale.scale  = 2.f;

        c->before.blur  = false;
        c->before.power = 2;

        c->refine.morph = mode_e::NONE;
        c->refine.power = 2;

        c->replace = { 255, 0, 255, 255 };

        c->key.lo = { 85, 225, 127 };
        c->key.lo = { 85, 225, 190 };
    }

    static internal_t *create_internal() {
        log->debug("create chromakey node");

        internal_t *instance = static_cast<internal_t *>(::malloc(sizeof(internal_t)));
        if (instance == nullptr)
            abort_("cannot create chromakey node instance, nullptr");

        ::memset(instance, 0, sizeof(internal_t));
        create_kernels(&instance->kernels);
        init_config(&instance->state);

        log->debug("chromakey node created");
        return instance;
    }

    static void destroy_internal(internal_t *ptr) {
        if (!ptr)
            return;

        log->debug("destroying chromakey node");
        ptr->buffers.output.finalize(true);
        ptr->buffers.aux_1.release();
        ptr->buffers.aux_2.release();
        ptr->buffers.image.release();
        release_kernels(&ptr->kernels);
        ::free(ptr);
        log->debug("chromakey node destroyed");
    }

    static void reset(internal_t *internal) {
        internal->buffers.aux_1.release();
        internal->buffers.aux_2.release();
        internal->buffers.image.release();
        internal->updates.state.refresh();
        internal->buffers.output.image = internal->buffers.image;
    }

    limbo::graph::BaseNode *ChromakeyNode::instance(TreeNode *from) {
        auto *instance = new ChromakeyNode();
        instance->set_node_id(from->id_node);
        instance->set_image_id(from->id_node);
        instance->enable();
        return instance;
    }

    limbo::graph::BaseNode *ChromakeyNode::relocate(TreeNode *, limbo::graph::BaseNode *node) {
        if (auto *t = dynamic_cast<ChromakeyNode *>(node))
            reset(t->internal);
        return node;
    }

    void ChromakeyNode::destroy(TreeNode *, limbo::graph::BaseNode *node) {
        node->detach_all();
        delete node;
    }

    void ChromakeyNode::set_config(const config_t &conf) {
        config_t copy;
        ::memcpy(&copy, &conf, sizeof(config_t));

        // TODO

        if (0 == ::memcmp(&copy, &internal->state, sizeof(config_t)))
            return;

        ::memcpy(&internal->state, &copy, sizeof(config_t));
        ::memcpy(&config, &copy, sizeof(config_t));
        reset(internal);
    }

    void ChromakeyNode::set_image_id(id_type_16 id) {
        internal->image_id = id;
    }

    id_type_16 ChromakeyNode::get_image_id() const {
        return internal->image_id;
    }

    id_type_16 ChromakeyNode::get_input_id() const {
        return internal->buffers.input.image.id;
    }

    unsigned ChromakeyNode::get_image_w() const {
        return internal->buffers.input.image.cols;
    }

    unsigned ChromakeyNode::get_image_h() const {
        return internal->buffers.input.image.rows;
    }

    void ChromakeyNode::enable() {
        reset(internal);
        internal->enabled = true;
    }

    void ChromakeyNode::disable() {
        internal->enabled = false;
        reset(internal);
    }

    io::byte_array ChromakeyNode::serialize() const {
        constexpr int total_size = sizeof(serial_t);

        serial_t parcel;
        parcel.id     = internal->image_id;
        parcel.on     = internal->enabled;
        parcel.config = internal->state;

        unsigned char *bytes = static_cast<unsigned char *>(::malloc(total_size));
        ::memcpy(bytes, &parcel, total_size);

        return io::byte_array::from(bytes, total_size, &::free);
    }

    void ChromakeyNode::deserialize(const io::byte_array &array) {
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

    void ChromakeyNode::on_input(const int in_idx, const int out_idx, const int clock_i, void *data) {
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

    void ChromakeyNode::on_update(int clock_i) {
        if (!internal->enabled)
            return;

        if (!internal->updates.input.update()) {
            reset(internal);
            return;
        }

        static const ex::State &state = ex::logic::globals::state();

        ocl::img_promise *input = &internal->buffers.input;
        ocl::image2d_t   *aux_1 = &internal->buffers.aux_1;
        ocl::image2d_t   *aux_2 = &internal->buffers.aux_2;
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

        const ::size_t pref_size = internal->kernels.size_l_chroma;
        ::size_t       l_size[2] = { pref_size, pref_size };
        ::size_t       g_size[2] = { xm::ocl::optimal_global_size(image->cols, pref_size),
                                     xm::ocl::optimal_global_size(image->rows, pref_size) };

        cl_mem buffer_in  = input->image.handle;
        cl_mem buffer_out = image->handle;

        // TODO
    }

    void *ChromakeyNode::on_output(int in_idx, int out_idx, int clock_i) {
        if (!internal->enabled)
            return nullptr;

        ocl::img_promise *target = &internal->buffers.output;
        target->image.id         = internal->image_id;
        return target;
    }

    ChromakeyNode::~ChromakeyNode() {
        destroy_internal(internal);
        internal = nullptr;
    }

    ChromakeyNode::ChromakeyNode() :
        internal(create_internal()) {
        ::memcpy(&config, &internal->state, sizeof(config_t));
        reset(internal);
    }
}