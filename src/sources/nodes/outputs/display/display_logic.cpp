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
        unsigned char *data;
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
        cl_command_queue queue;
    };

    struct internal_t {
        buffers_t  buffers;
        workload_t workload;
        display_t  display;
        bytes_t    bytes;

        id_type_32 prev_;
        control_t  updates_;
        control_t  supplies_;

        bool       enabled;
    };

    static void init_buffers(buffers_t *b) {
        for (int i = 0; i < buffers_n; ++i)
            b->image[0] = ocl::Image2D::blank();
        b->index = 0;
    }

    static void swap_buffers(buffers_t *b) {
        b->index = (b->index + 1) % buffers_n;
    }

    static void reset_buffers(buffers_t *b) {
        for (int i = 0; i < buffers_n; ++i)
            b->image[0].release();
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
        ptr->workload.promise.release();
        if (ptr->display.id != 0)
            ex::display::io.remove_output(ptr->display.id);
        if (ptr->bytes.data != nullptr) {
            ::free(ptr->bytes.data);
            ptr->bytes.data = nullptr;
        }
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
        internal->display.id   = ex::display::io.add_output(internal->display.name);
        internal->display.name = name;
        internal->updates_.refresh();
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
        internal->updates_.refresh();
    }

    void DisplayOutputNode::on_input(const int in_idx, const int out_idx, const int clock_i, void *data) {
        const static ex::State &state = ex::logic::globals::state();

        if (!internal->enabled)
            return;

        const ocl::img_promise *promise = static_cast<const ocl::img_promise *>(data);
        if (promise == nullptr) {
            reset();
            internal->updates_.refresh();
            return;
        }

        const ocl::Image2D &img = promise->image;
        if (img.empty()) {
            reset();
            internal->updates_.refresh();
            return;
        }

        internal->supplies_.refresh();

        if (!internal->workload.promise.completed())
            return;

        const unsigned index = internal->buffers.index;
        ocl::Image2D  &image = internal->buffers.image[index];

        if (img.header.fresh || img.id != internal->prev_ || image.detached())
            internal->updates_.refresh();

        if (!internal->updates_.fresh())
            return;

        if (img.id != internal->prev_ ||
            img.size() != image.size() ||
            img.cols != image.cols ||
            img.rows != image.rows ||
            image.detached())
        {
            image.release();
            image = ex::ocl::Image2D::allocate_like(img, state.cl.context);
            internal->prev_ = img.id;
        }

        // TODO copy to internal cl_mem buffer

        internal->updates_.refresh();

        // TODO UNLOAD DATA TO HOST
    }

    void DisplayOutputNode::on_update(const int clock_i) {
        if (!internal->enabled)
            return;

        internal->updates_.update();

        if (!internal->supplies_.update()) {
            reset();
            return;
        }

        if (internal->workload.promise.empty() || !internal->workload.promise.completed())
            return;

        internal->workload.promise.release();

        if (internal->bytes.data != nullptr) {
            const unsigned channels = internal->bytes.size / (internal->bytes.width * internal->bytes.height);
            ex::display::io.draw_bytes_bgra_8(internal->display.id, internal->bytes.data,
                                              internal->bytes.width, internal->bytes.height,
                                              channels);
        }
        swap_buffers(&internal->buffers);
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
