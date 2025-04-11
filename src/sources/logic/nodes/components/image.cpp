//
// Created by xd on 4/9/25.
//

#include "../../../../headers/logic/nodes/components/image.h"
#include "../../../../headers/logic/nodes/logic_nodes.h"
#include "../../../../headers/logic/logic_globals.h"

namespace ex::logic::node::cmp::image {

    limbo::graph::BaseNode *ImageNode::instance(TreeNode *node) {
        ImageNode *ptr = new ImageNode();
        ptr->set_id(node->id_local);
        return ptr;
    }

    limbo::graph::BaseNode *ImageNode::relocate(TreeNode *, limbo::graph::BaseNode *node) {
        return node;
    }

    void ImageNode::destroy(TreeNode *, limbo::graph::BaseNode *node) {
        node->detach_all();
        delete node;
    }

    void ImageNode::set_image(const char *image_path) {
        ::strncpy(state.source, image_path, sizeof(state.source));
        state.source[sizeof(state.source) - 1] = '\0';
        state.selected = true;

        // TODO
    }

    io::byte_array ImageNode::serialize() const {
        data::byte_array array = data::byte_array::from(nullptr, 0);
        // TODO
        return array;
    }

    void ImageNode::deserialize(const io::byte_array &array) const {
        // TODO
    }

    void ImageNode::on_input(int in_idx, int out_idx, int clock_i, const void *data) {
        // TODO
    }

    void ImageNode::on_output(int in_idx, int out_idx, int clock_i, void *data) {
        // TODO
    }

    ImageNode::~ImageNode() {
        // TODO
    }

    ImageNode::ImageNode() : state({ .source = {}, .selected = false }) {
        // TODO
    }
}

