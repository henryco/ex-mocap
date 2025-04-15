//
// Created by xd on 4/9/25.
//

#include "../../../../headers/logic/nodes/components/image.h"
#include "../../../../headers/logic/nodes/logic_nodes.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/cl/cl_kernel.h"

// STB INCLUDES ---------------------------------
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
// STB INCLUDES ---------------------------------

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
        const ex::State &state_ = ex::logic::globals::state();

        ::strncpy(state.source, image_path, sizeof(state.source));
        state.source[sizeof(state.source) - 1] = '\0';
        state.selected = true;

        if (source_.data_ptr != nullptr) {
            stbi_image_free(source_.data_ptr);
            source_.data_ptr = nullptr;
        }

        if (queue_ == nullptr) {
            queue_ = xm::ocl::create_queue_device(state_.cl.context, state_.cl.device, true, false);
        }

        // RGBA
        source_.data_ptr = stbi_load(image_path, &source_.width, &source_.height, &source_.channels, 0);

        if (source_.channels > 1) {
            for (int i = 0; i < source_.width * source_.height; ++i) {
                unsigned char *pix = source_.data_ptr + i * source_.channels;
                unsigned char  tmp = pix[0];
                pix[0]             = pix[2];
                pix[2]             = tmp;
            }
        }

        image_.release();

        ocl::ImagePromise promise = ex::ocl::ops::from_bytes(
            source_.data_ptr, source_.width, source_.height,
            source_.channels, 1, state_.cl.context, queue_);
        promise.finalize();

        promise_ = promise;
        image_   = promise.image;
    }

    io::byte_array ImageNode::serialize() const {
        data::byte_array array = data::byte_array::from(nullptr, 0);
        // TODO
        return array;
    }

    void ImageNode::deserialize(const io::byte_array &array) const {
        // TODO
    }

    void ImageNode::on_input(const int in_idx, const int out_idx, const int clock_i, const void *data) {
        // TODO
    }

    void ImageNode::on_output(const int in_idx, const int out_idx, const int clock_i, void *data) {
        if (data == GRAPH_SKIP)
            return;
        if (source_.data_ptr == nullptr) {
            *(static_cast<ocl::img_promise *>(data)) = nullptr;
            return;
        }
        *(static_cast<ocl::img_promise *>(data)) = &promise_;
    }

    ImageNode::~ImageNode() {
        image_.release();

        if (source_.data_ptr != nullptr) {
            stbi_image_free(source_.data_ptr);
            source_.data_ptr = nullptr;
        }

        if (queue_ != nullptr) {
            clReleaseCommandQueue(queue_);
            queue_ = nullptr;
        }
    }

    ImageNode::ImageNode() :
        queue_(nullptr),
        image_(ocl::Image2D::blank()),
        state({ .source = {}, .selected = false }) {
    }
}

