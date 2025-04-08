//
// Created by xd on 2/25/25.
//

#include "../../../../headers/logic/nodes/outputs/display.h"
#include "../../../../headers/display/display.h"

#include <iostream>

namespace ex::logic::node::out {

    ln::BaseNode *DisplayOutputNode::instance(TreeNode *n) {
        const auto node = new DisplayOutputNode();
        node->register_image(n->name, 0, 0);
        return node;
    }

    ln::BaseNode *DisplayOutputNode::relocate(TreeNode *, ln::BaseNode *node) {
        return node;
    }

    void DisplayOutputNode::destroy(TreeNode *, ln::BaseNode *node) {
        node->detach_all();
        delete node;
    }

    void DisplayOutputNode::register_image(const char *name, const uint w, const uint h, const uint channels,
                                           const uint bit_depth) {
        if (display_i != 0)
            ex::display::io.remove_output(display_i);

        display_n = name;
        display_w = w;
        display_h = h;
        display_c = channels;
        display_d = bit_depth;

        display_i = ex::display::io.add_output(display_n, display_w, display_h, display_c, display_d);
    }

    void DisplayOutputNode::rename_image(const char *name) {
        if (display_i != 0)
            ex::display::io.remove_output(display_i);
        display_n = name;
        display_i = ex::display::io.add_output(display_n, display_w, display_h, display_c, display_d);
    }

    void DisplayOutputNode::on_input(int in_idx, int out_idx, int clock_i, const void *data) {
        // TODO
    }

    void DisplayOutputNode::on_output(int in_idx, int out_idx, int clock_i, void *data) {
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
