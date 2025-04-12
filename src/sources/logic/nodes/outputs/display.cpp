//
// Created by xd on 2/25/25.
//

#include "../../../../headers/logic/nodes/outputs/display.h"
#include "../../../../headers/display/display.h"

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
        proto::image *ptr = *(static_cast<proto::image *const *>(data));
        if (ptr != image_ptr) {
            image_ptr = ptr;
            // TODO
        }
    }

    void DisplayOutputNode::on_output(const int in_idx, const int out_idx, const int clock_i, void *data) {
        // TODO
    }

    DisplayOutputNode::~DisplayOutputNode() {
        std::cout << "DSTR DisplayOutputNode::DisplayOutputNode()\n";
        if (display_i != 0)
            ex::display::io.remove_output(display_i);
    }

    DisplayOutputNode::DisplayOutputNode() {
        std::cout << "CNST DisplayOutputNode::DisplayOutputNode()\n";
    }

} // namespace ex::logic::node::out
