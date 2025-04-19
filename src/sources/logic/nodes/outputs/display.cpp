//
// Created by xd on 2/25/25.
//

#include "../../../../headers/logic/nodes/outputs/display.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/display/display.h"
#include "../../../../headers/cl/ocl_interop.h"
#include "../../../../headers/cl/cl_kernel.h"

#include <iostream>

namespace ex::logic::node::out {

    void enqueue_worker_(DisplayOutputNode &this_) {
        if (this_.data_ == nullptr ||
            this_.size_ != this_.image_.size() ||
            this_.i_w_ != this_.image_.cols ||
            this_.i_h_ != this_.image_.rows
        ) {
            if (this_.data_ != nullptr)
                ::free(this_.data_);
            this_.size_ = this_.image_.size();
            this_.data_ = static_cast<unsigned char *>(::malloc(this_.size_));
            this_.i_w_  = this_.image_.cols;
            this_.i_h_  = this_.image_.rows;
            ex::display::io.clear(this_.display_i);
        }

        this_.promise_worker_.release();
        this_.promise_worker_ = ex::ocl::ops::to_bytes(this_.image_, this_.data_, this_.queue_).non_blocking();
    }

    ln::BaseNode *DisplayOutputNode::instance(TreeNode *n) {
        const auto node = new DisplayOutputNode();
        node->set_id(n->id_local);
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
        if (display_i != 0)
            ex::display::io.remove_output(display_i);
        display_n = name;
        display_i = ex::display::io.add_output(display_n);
        updates_.refresh();
    }

    void DisplayOutputNode::rename_image(const char *name) {
        ex::display::io.rename_output(display_i, name);
    }

    void DisplayOutputNode::reset() {
        ex::display::io.clear(display_i);
        promise_worker_.release();
        image_.release();
    }

    void DisplayOutputNode::disable() {
        enabled = false;
        reset();
    }

    void DisplayOutputNode::enable() {
        enabled = true;
        updates_.refresh();
    }

    void DisplayOutputNode::on_input(const int in_idx, const int out_idx, const int clock_i, const void *data) {
        const static ex::State &state = ex::logic::globals::state();

        if (!enabled)
            return;

        const ocl::img_promise *promise = static_cast<const ocl::img_promise *>(data);
        if (promise == nullptr) {
            reset();
            updates_.refresh();
            return;
        }

        const ocl::Image2D &img = promise->image;
        if (img.empty()) {
            reset();
            updates_.refresh();
            return;
        }

        supplies_.refresh();

        if (!promise_worker_.completed())
            return;

        if (img.header.fresh || image_.detached())
            updates_.refresh();

        if (!updates_.fresh())
            return;

        if (img.size() != image_.size() || image_.detached()) {
            image_.release();
            image_ = ex::ocl::Image2D::allocate_like(img, state.cl.context);
        }

        // copy to internal cl_mem buffer
        ex::ocl::ops::copy_data(*promise, image_)
            .finalize();

        updates_.refresh();

        enqueue_worker_(*this);
    }

    void DisplayOutputNode::on_update(const int clock_i) {

        if (!enabled) {
            reset();
            return;
        }

        updates_.update();

        if (!supplies_.update()) {
            reset();
            return;
        }

        if (promise_worker_.empty() || !promise_worker_.completed())
            return;

        promise_worker_.release();

        if (!image_.empty() && !image_.detached() && data_ != nullptr) {
            ex::display::io.draw_bytes_bgra_8(display_i, data_, image_.cols, image_.rows, image_.header.channels);
        }
    }

    void *DisplayOutputNode::on_output(const int in_idx, const int out_idx, const int clock_i) {
        return nullptr;
    }

    DisplayOutputNode::~DisplayOutputNode() {
        std::cout << "DSTR DisplayOutputNode::DisplayOutputNode()\n";
        image_.release();
        promise_worker_.release();
        if (display_i != 0)
            ex::display::io.remove_output(display_i);
        if (data_ != nullptr)
            ::free(data_);
        if (queue_ != nullptr) {
            clReleaseCommandQueue(queue_);
            queue_ = nullptr;
        }
    }

    DisplayOutputNode::DisplayOutputNode():
        image_(ocl::Image2D::blank()) {
        const static ex::State &state = ex::logic::globals::state();
        queue_ = xm::ocl::create_queue_device(state.cl.context, state.cl.device, true, false);
        std::cout << "CNST DisplayOutputNode::DisplayOutputNode()\n";
    }

} // namespace ex::logic::node::out
