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

    ln::BaseNode *DisplayOutputNode::instance(TreeNode *n) {
        const auto node = new DisplayOutputNode();
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
    }

    void DisplayOutputNode::rename_image(const char *name) {
        if (display_i != 0)
            ex::display::io.remove_output(display_i);
        display_n = name;
        display_i = ex::display::io.add_output(display_n);
    }

    void DisplayOutputNode::on_input(const int in_idx, const int out_idx, const int clock_i, const void *data) {
        const static ex::State &state = ex::logic::globals::state();

        const ocl::img_promise *promise = static_cast<const ocl::img_promise *>(data);
        if (promise == nullptr) {
            return;
        }

        const ocl::Image2D &img = promise->image;
        if (img.empty()) {
            ex::display::io.clear(display_i);
            return;
        }

        if (!promise_unload_.completed())
            return;

        if (img.size() != image_.size()) {
            image_.release();
            image_ = ex::ocl::Image2D::allocate_like(img, state.cl.context);
        }

        // copy to internal cl_mem buffer
        ex::ocl::ops::copy_data(*promise, image_)
            .finalize();

        if (!state.gl.sharing) {
            if (size_ != img.size() || data_ == nullptr || i_w_ != img.cols || i_h_ != img.rows) {
                if (data_ != nullptr)
                    ::free(data_);
                size_ = img.size();
                data_ = static_cast<unsigned char *>(::malloc(size_));
                i_w_  = img.cols;
                i_h_  = img.rows;
                ex::display::io.clear(display_i);
            }

            promise_unload_.release();
            promise_unload_ = ex::ocl::ops::to_bytes(image_, data_, queue_).non_blocking();
            return;
        }

        // TODO Cl / GL INTEROP
    }

    void DisplayOutputNode::on_update(int clock_i) {
        const static ex::State &state = ex::logic::globals::state();
        if (promise_unload_.empty() || !promise_unload_.completed())
            return;

        if (!state.gl.sharing) {
            if (!image_.empty() && data_ != nullptr) {
                ex::display::io.draw_bytes_bgra_8(display_i, data_, image_.cols, image_.rows, image_.header.channels);
            }
        }

        else {
            // TODO Cl / GL INTEROP
        }
    }

    void *DisplayOutputNode::on_output(const int in_idx, const int out_idx, const int clock_i) {
        return nullptr;
    }

    DisplayOutputNode::~DisplayOutputNode() {
        std::cout << "DSTR DisplayOutputNode::DisplayOutputNode()\n";
        image_.release();
        promise_unload_.release();
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
