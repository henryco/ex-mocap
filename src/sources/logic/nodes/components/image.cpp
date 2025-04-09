//
// Created by xd on 4/9/25.
//

#include "../../../../headers/logic/nodes/components/image.h"
#include "../../../../headers/logic/nodes/logic_nodes.h"
#include "../../../../headers/logic/logic_globals.h"

namespace ex::logic::node::cmp::image {

    limbo::graph::BaseNode *ImageNode::instance(TreeNode *) {
        return new ImageNode();
    }

    limbo::graph::BaseNode *ImageNode::relocate(TreeNode *, limbo::graph::BaseNode *node) {
        return node;
    }

    void ImageNode::destroy(TreeNode *, limbo::graph::BaseNode *node) {
        node->detach_all();
        delete node;
    }

    void ImageNode::on_input(int in_idx, int out_idx, int clock_i, const void *data) {
        // TODO
    }

    void ImageNode::on_output(int in_idx, int out_idx, int clock_i, void *data) {
        // TODO
    }

    ImageNode::~ImageNode() {
        ex::logic::globals::logger()->debug("~ImageNode()");
        // TODO
    }

    ImageNode::ImageNode() {
        ex::logic::globals::logger()->debug("ImageNode()");
        // TODO
    }
}

