//
// Created by xd on 2/25/25.
//

#include "../../../../headers/logic/nodes/outputs/display.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/display/display.h"
#include "../../../../headers/cl/ocl_interop.h"

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
        const ocl::img_promise promise = *(static_cast<const ocl::img_promise *>(data));

        if (promise == nullptr) {
            ex::display::io.clear(display_i);
            return;
        }

        const static ex::State &state = ex::logic::globals::state();
        if (state.gl.sharing) {
            ex::display::io.clear(display_i);
            // TODO
            return;
        }

        if (promise->image.empty()) {
            ex::display::io.clear(display_i);
            return;
        }

        const auto &img = promise->image;
        if (size_ != promise->image.size() || data_ == nullptr || i_w_ != img.cols || i_h_ != img.rows) {
            if (data_ != nullptr)
                ::free(data_);
            size_ = promise->image.size();
            data_ = static_cast<unsigned char *>(::malloc(size_));
            i_w_  = img.cols;
            i_h_  = img.rows;
            ex::display::io.clear(display_i);
        }

        promise->finalize();

        auto out_promise = ex::ocl::ops::to_bytes(*promise, data_);
        out_promise.finalize();

        ex::display::io.draw_bytes_bgra_8(display_i, data_, img.cols, img.rows, img.header.channels);
    }

    void DisplayOutputNode::on_output(const int in_idx, const int out_idx, const int clock_i, void *data) {
    }

    DisplayOutputNode::~DisplayOutputNode() {
        std::cout << "DSTR DisplayOutputNode::DisplayOutputNode()\n";
        if (display_i != 0)
            ex::display::io.remove_output(display_i);
        if (data_ != nullptr)
            ::free(data_);
    }

    DisplayOutputNode::DisplayOutputNode() {
        std::cout << "CNST DisplayOutputNode::DisplayOutputNode()\n";
    }

} // namespace ex::logic::node::out
