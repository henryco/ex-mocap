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
    struct bytes_t {
        unsigned char *data[buffers_n];
        unsigned short index;
        unsigned short width;
        unsigned short height;
        unsigned short channels;
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
        workload_t workload;
        display_t  display;
        bytes_t    bytes;

        id_type_32 prev_;
        control_t  supplies_;
        control_t  updates_;

        bool       enabled;
        bool       direct;
    };

    static void alloc_bytes(bytes_t *b, const unsigned w, const unsigned h, const unsigned c) {
        const unsigned size = w * h * c;

        for (int i = 0; i < buffers_n; ++i) {
            if (b->data[i] != nullptr)
                ::free(b->data[i]);
            b->data[i] = static_cast<unsigned char *>(::malloc(size));
        }

        b->index    = 0;
        b->width    = w;
        b->height   = h;
        b->channels = c;
    }

    static unsigned bytes_size(const bytes_t *b) {
        return ((unsigned) b->width * (unsigned) b->height * (unsigned) b->channels * 1U);
    }

    bool bytes_empty(const bytes_t *b) {
        for (int i = 0; i < buffers_n; ++i) {
            if (b->data[i] == nullptr)
                return true;
        }
        return false;
    }

    static unsigned swap_buffers(bytes_t *b) {
        b->index = (b->index + 1) % buffers_n;
        return b->index;
    }

    static void reset_bytes(bytes_t *b) {
        b->index = 0;
        for (int i = 0; i < buffers_n; ++i) {
            if (b->data[i] != nullptr)
                ::free(b->data[i]);
            b->data[i] = nullptr;
        }
    }

    static internal_t *create_internal() {
        const static ex::State &state = ex::logic::globals::state();

        internal_t *mem = static_cast<internal_t *>(::malloc(sizeof(internal_t)));
        ::memset(mem, 0, sizeof(internal_t));

        mem->workload.queue = xm::ocl::create_queue_device(state.cl.context, state.cl.device, true, false);
        mem->enabled = true;
        mem->direct  = true;

        return mem;
    }

    static void release_internal(internal_t *ptr) {
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

    void DisplayOutputNode::set_direct(const bool v) const {
        internal->direct = v;
    }

    bool DisplayOutputNode::is_direct() const {
        return internal->direct;
    }

    id_type_16 DisplayOutputNode::get_input_id() const {
        return internal->prev_;
    }

    unsigned DisplayOutputNode::get_input_w() const {
        return internal->bytes.width;
    }

    unsigned DisplayOutputNode::get_input_h() const {
        return internal->bytes.height;
    }

    void DisplayOutputNode::reset() {
        ex::display::io.clear(internal->display.id);
        internal->workload.promise.release();
        reset_bytes(&internal->bytes);
        internal->updates_.refresh();
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

        bytes_t          image = internal->bytes;
        const unsigned   index = internal->bytes.index;
        ocl::img_promise input = internal->workload.input;

        const bool has_diff = input.image.id != internal->prev_            ||
                              input.image.size() != bytes_size(&image) ||
                              input.image.cols != image.width              ||
                              input.image.rows != image.height             ||
                              bytes_empty(&image);

        if (internal->workload.promise.completed() && !internal->workload.promise.empty()) {
            internal->workload.promise.release();
            if (internal->direct)
                display::io.flush_bgra_8(internal->display.id);
            else if (internal->bytes.data[index])
                display::io.draw_bytes_bgra_8(internal->display.id, internal->bytes.data[index]);
        }

        if (input.image.header.fresh || has_diff)
            internal->updates_.refresh();

        if (!internal->updates_.update())
            return;

        if (has_diff) {
            const unsigned channels = ocl::channel_count(input.image.header.channels);
            const unsigned width = input.image.cols;
            const unsigned height = input.image.rows;

            alloc_bytes(&internal->bytes, width, height, channels);

            internal->workload.promise.release();
            internal->prev_ = input.image.id;

            display::io.prepare_bgra_8(internal->display.id, internal->bytes.width, internal->bytes.height, channels);
        }

        if (!internal->workload.promise.completed()) {
            internal->updates_.refresh(); // task not completed
            return;
        }

        const unsigned  next_index = swap_buffers(&internal->bytes);
        void           *target_ptr = nullptr;

        if (internal->direct) {
            if (target_ptr = display::io.direct_bgra_8(internal->display.id); !target_ptr) {
                internal->updates_.refresh(); // display not ready
                return;
            }
        }
        else target_ptr = internal->bytes.data[next_index];

        // create wait marker on main image side
        ocl::NBPromise marker_main = ocl::ops::mark(input.queue);

        // depend start of copy on reaching main image side marker
        ocl::NBPromise marker_copy = ocl::ops::barrier(marker_main, internal->workload.queue);

        // enqueue copy
        internal->workload.promise =
                ocl::ops::to_bytes(input.image, target_ptr, internal->workload.queue)
                        .non_blocking();

        // depend end of main image side queue on copy
        ocl::NBPromise marker_post = ocl::ops::barrier(internal->workload.promise, input.queue);

        // cleanup
        marker_copy.release();
        marker_post.release();
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
