//
// Created by xd on 2/25/25.
//

#include "../../../../headers/nodes/outputs/display/display_logic.h"
#include "../../../../headers/cl/cl_kernel.h"
#include "../../../../headers/io/display_out.h"
#include "../../../../headers/logic/logic_globals.h"

#include <iostream>

namespace ex::logic::node::out {

    struct internal_t {
        const char   *display_n;
        unsigned int  display_i;
        id_type_32    prev_;

        // GL/CL/ETC interop internal
        unsigned char *data_;
        unsigned long  size_;
        unsigned int   i_w_;
        unsigned int   i_h_ ;

        ocl::Image2D     image_;
        ocl::NBPromise   promise_worker_;
        cl_command_queue queue_;

        limbo::graph::UpdateState updates_;
        limbo::graph::UpdateState supplies_;
        // GL/CL/ETC interop internal

        bool enabled;
    };

    static internal_t *create_internal() {
        const static ex::State &state = ex::logic::globals::state();

        internal_t *mem = static_cast<internal_t *>(::malloc(sizeof(internal_t)));
        ::memset(mem, 0, sizeof(internal_t));

        mem->enabled = true;
        mem->image_  = ocl::Image2D::blank();
        mem->queue_  = xm::ocl::create_queue_device(state.cl.context, state.cl.device, true, false);

        return mem;
    }

    static void release_internal(internal_t *ptr) {
        ptr->image_.release();
        ptr->promise_worker_.release();
        if (ptr->display_i != 0)
            ex::display::io.remove_output(ptr->display_i);
        if (ptr->data_ != nullptr) {
            ::free(ptr->data_);
            ptr->data_ = nullptr;
        }
        if (ptr->queue_ != nullptr) {
            clReleaseCommandQueue(ptr->queue_);
            ptr->queue_ = nullptr;
        }
        ::free(ptr);
    }

    void enqueue_worker_(DisplayOutputNode &this_) {
        if (this_.internal->data_ == nullptr ||
            this_.internal->size_ != this_.internal->image_.size() ||
            this_.internal->i_w_ != this_.internal->image_.cols ||
            this_.internal->i_h_ != this_.internal->image_.rows
        ) {
            if (this_.internal->data_ != nullptr)
                ::free(this_.internal->data_);
            this_.internal->size_ = this_.internal->image_.size();
            this_.internal->data_ = static_cast<unsigned char *>(::malloc(this_.internal->size_));
            this_.internal->i_w_  = this_.internal->image_.cols;
            this_.internal->i_h_  = this_.internal->image_.rows;
            ex::display::io.clear(this_.internal->display_i);
        }

        this_.internal->promise_worker_.release();
        this_.internal->promise_worker_ = ex::ocl::ops::to_bytes(
                this_.internal->image_, this_.internal->data_, this_.internal->queue_).non_blocking();
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
        if (internal->display_i != 0)
            ex::display::io.remove_output(internal->display_i);
        internal->display_n = name;
        internal->display_i = ex::display::io.add_output(internal->display_n);
        internal->updates_.refresh();
    }

    void DisplayOutputNode::rename_image(const char *name) {
        ex::display::io.rename_output(internal->display_i, name);
    }

    id_type_16 DisplayOutputNode::get_input_id() const {
        return internal->prev_;
    }

    unsigned DisplayOutputNode::get_input_w() const {
        return internal->i_w_;
    }

    unsigned DisplayOutputNode::get_input_h() const {
        return internal->i_h_;
    }

    void DisplayOutputNode::reset() {
        ex::display::io.clear(internal->display_i);
        internal->promise_worker_.release();
        internal->image_.release();
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

        if (!internal->promise_worker_.completed())
            return;

        if (img.header.fresh || img.id != internal->prev_ || internal->image_.detached())
            internal->updates_.refresh();

        if (!internal->updates_.fresh())
            return;

        if (img.id != internal->prev_ ||
            img.size() != internal->image_.size() ||
            img.cols != internal->image_.cols ||
            img.rows != internal->image_.rows ||
            internal->image_.detached())
        {
            internal->image_.release();
            internal->image_ = ex::ocl::Image2D::allocate_like(img, state.cl.context);
            internal->prev_  = img.id;
        }

        // copy to internal cl_mem buffer
        ex::ocl::ops::copy_data(*promise, internal->image_)
            .finalize();

        internal->updates_.refresh();

        enqueue_worker_(*this);
    }

    void DisplayOutputNode::on_update(const int clock_i) {

        if (!internal->enabled)
            return;

        internal->updates_.update();

        if (!internal->supplies_.update()) {
            reset();
            return;
        }

        if (internal->promise_worker_.empty() || !internal->promise_worker_.completed())
            return;

        internal->promise_worker_.release();

        if (!internal->image_.empty() && !internal->image_.detached() && internal->data_ != nullptr) {
            ex::display::io.draw_bytes_bgra_8(internal->display_i, internal->data_,
                                              internal->image_.cols, internal->image_.rows,
                                              internal->image_.header.channels);
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
