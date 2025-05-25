//
// Created by xd on 2/25/25.
//

#include "../../../../headers/nodes/outputs/display/display_logic.h"
#include "../../../../headers/cl/cl_kernel.h"
#include "../../../../headers/io/display_out.h"
#include "../../../../headers/logic/logic_globals.h"

#include <iostream>

namespace ex::logic::node::out {

    using control_t = limbo::graph::UpdateState;

    #define buffers_n 2
    struct buffers_t {
        ocl::Image2D image[buffers_n];
        unsigned     index;
    };

    struct bytes_t {
        unsigned char *data[buffers_n];
        unsigned int   size;
        unsigned short width;
        unsigned short height;
    };

    struct display_t {
        const char *name;
        unsigned    id;
    };

    struct workload_t {
        ocl::NBPromise   promise;
        ocl::img_promise input;
        cl_command_queue queue;
    };

    struct internal_t {
        buffers_t  buffers;
        workload_t workload;
        display_t  display;
        bytes_t    bytes;

        id_type_32 prev_;
        control_t  supplies_;

        bool       enabled;
    };

    static void init_buffers(buffers_t *b) {
        for (int i = 0; i < buffers_n; ++i)
            b->image[i] = ocl::Image2D::blank();
        b->index = 0;
    }

    static void alloc_buffers(buffers_t *b, const ocl::Image2D *temp) {
        const static ex::State &state = ex::logic::globals::state();
        for (int i = 0; i < buffers_n; ++i) {
            b->image[i].release();
            b->image[i] = ocl::Image2D::allocate_like(*temp, state.cl.context);
        }
        b->index = 0;
    }

    static void alloc_bytes(bytes_t *b, const unsigned w, const unsigned h, const unsigned c) {
        b->size   = w * h * c;
        b->width  = w;
        b->height = h;

        for (int i = 0; i < buffers_n; ++i) {
            if (b->data[i] != nullptr)
                ::free(b->data[i]);
            b->data[i] = static_cast<unsigned char *>(::malloc(b->size));
        }
    }

    static unsigned swap_buffers(buffers_t *b) {
        b->index = (b->index + 1) % buffers_n;
        return b->index;
    }

    static void reset_bytes(bytes_t *b) {
        for (int i = 0; i < buffers_n; ++i) {
            if (b->data[i] != nullptr)
                ::free(b->data[i]);
            b->data[i] = nullptr;
        }
    }

    static void reset_buffers(buffers_t *b) {
        for (int i = 0; i < buffers_n; ++i)
            b->image[i].release();
        b->index = 0;
    }

    static internal_t *create_internal() {
        const static ex::State &state = ex::logic::globals::state();

        internal_t *mem = static_cast<internal_t *>(::malloc(sizeof(internal_t)));
        ::memset(mem, 0, sizeof(internal_t));

        mem->enabled = true;
        mem->workload.queue = xm::ocl::create_queue_device(state.cl.context, state.cl.device, true, false);
        init_buffers(&mem->buffers);

        return mem;
    }

    static void release_internal(internal_t *ptr) {
        reset_buffers(&ptr->buffers);
        reset_bytes(&ptr->bytes);

        ptr->workload.promise.release();

        if (ptr->display.id != 0)
            ex::display::io.remove_output(ptr->display.id);

        if (ptr->workload.queue != nullptr) {
            clReleaseCommandQueue(ptr->workload.queue);
            ptr->workload.queue = nullptr;
        }
        ::free(ptr);
    }

    ln::BaseNode *DisplayOutputNode::instance(TreeNode *n) {
        const auto node = new DisplayOutputNode();
        node->set_node_id(n->id_node);
        node->register_image(n->name);
        return node;
    }

    ln::BaseNode *DisplayOutputNode::relocate(TreeNode *, ln::BaseNode *node) {
        return node;
    }

    void DisplayOutputNode::destroy(TreeNode *, ln::BaseNode *node) {
        node->detach_all();
        delete node;
    }

    void DisplayOutputNode::register_image(const char *name) {
        if (internal->display.id != 0)
            ex::display::io.remove_output(internal->display.id);
        internal->display.name = name;
        internal->display.id   = ex::display::io.add_output(internal->display.name);
    }

    void DisplayOutputNode::rename_image(const char *name) {
        ex::display::io.rename_output(internal->display.id, name);
    }

    id_type_16 DisplayOutputNode::get_input_id() const {
        return internal->prev_;
    }

    unsigned DisplayOutputNode::get_input_w() const {
        return internal->bytes.width;
    }

    unsigned DisplayOutputNode::get_input_h() const {
        return internal->bytes.width;
    }

    void DisplayOutputNode::reset() {
        ex::display::io.clear(internal->display.id);
        internal->workload.promise.release();
        reset_buffers(&internal->buffers);
    }

    void DisplayOutputNode::disable() {
        internal->enabled = false;
        reset();
    }

    void DisplayOutputNode::enable() {
        internal->enabled = true;
    }

    void DisplayOutputNode::on_input(const int in_idx, const int out_idx, const int clock_i, void *data) {
        const static ex::State &state = ex::logic::globals::state();

        if (!internal->enabled)
            return;

        const ocl::img_promise *promise = static_cast<const ocl::img_promise *>(data);
        if (promise == nullptr || promise->empty()) {
            reset();
            return;
        }

        internal->workload.input = *promise;
        internal->supplies_.refresh();
    }

    void DisplayOutputNode::on_update(const int clock_i) {
        if (!internal->enabled)
            return;

        if (!internal->supplies_.update()) {
            reset();
            return;
        }

        const unsigned   index = internal->buffers.index;
        ocl::img_promise input = internal->workload.input;
        ocl::image2d_t   image = internal->buffers.image[index];

        bool should_step = false;
        if (input.image.header.fresh || input.image.id != internal->prev_ || image.detached())
            should_step = true;

        if (!internal->workload.promise.empty())
            should_step = true;

        if (!should_step)
            return;

        if (input.image.id != internal->prev_ ||
            input.image.size() != image.size() ||
            input.image.cols != image.cols ||
            input.image.rows != image.rows ||
            image.detached())
        {
            const unsigned channels = ocl::channel_count(input.image.header.channels);
            const unsigned width = input.image.cols;
            const unsigned height = input.image.rows;
            alloc_bytes(&internal->bytes, width, height, channels);
            alloc_buffers(&internal->buffers, &input.image);
            internal->prev_ = input.image.id;
        }

        if (!internal->workload.promise.completed())
            return;

        ocl::image2d_t &curr_image = internal->buffers.image[index];
        const unsigned  next_index = swap_buffers(&internal->buffers);
        ocl::image2d_t &next_image = internal->buffers.image[next_index];

        ocl::ops::copy_data(input, next_image);

        internal->workload.promise.release();
        internal->workload.promise =
                ocl::ops::to_bytes(curr_image, internal->bytes.data[next_index], internal->workload.queue)
                        .non_blocking();

        if (internal->bytes.data[index] != nullptr) {
            const unsigned channels = internal->bytes.size / (internal->bytes.width * internal->bytes.height);
            ex::display::io.draw_bytes_bgra_8(internal->display.id, internal->bytes.data[index],
                                              internal->bytes.width, internal->bytes.height,
                                              channels);
        }
    }

    void *DisplayOutputNode::on_output(const int in_idx, const int out_idx, const int clock_i) {
        return nullptr;
    }

    DisplayOutputNode::~DisplayOutputNode() {
        if (internal != nullptr)
            release_internal(internal);
        internal = nullptr;
    }

    DisplayOutputNode::DisplayOutputNode():
        internal(create_internal()) {
    }
}
