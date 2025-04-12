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

    static bool noop_(void *) {
        return true;
    }

    void ImageNode::set_image(const char *image_path) {
        const ex::State &state_ = ex::logic::globals::state();

        ::strncpy(state.source, image_path, sizeof(state.source));
        state.source[sizeof(state.source) - 1] = '\0';
        state.selected = true;

        if (source.data_ptr != nullptr) {
            stbi_image_free(source.data_ptr);
            source.data_ptr = nullptr;
        }

        if (queue == nullptr) {
            queue = xm::ocl::create_queue_device(state_.cl.context, state_.cl.device, true, false);
        }

        // RGBA
        source.data_ptr = stbi_load(image_path, &source.width, &source.height, &source.channels, 0);

        if (source.channels > 1) {
            for (int i = 0; i < source.width * source.height; ++i) {
                unsigned char *pix = source.data_ptr + i * source.channels;
                unsigned char  tmp = pix[0];
                pix[0]             = pix[2];
                pix[2]             = tmp;
            }
        }

        image = {
            .image_promise = xm::ocl::iop::from_bytes(
                source.data_ptr,
                source.width,
                source.height,
                source.channels,
                1,
                state_.cl.context,
                state_.cl.device,
                queue),
            .sync_  = &noop_,
            .ctx___ = nullptr
        };
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
        if (source.data_ptr == nullptr) {
            *(static_cast<proto::image **>(data)) = nullptr;
            return;
        }
        *(static_cast<proto::image **>(data)) = &image;
    }

    ImageNode::~ImageNode() {
        if (source.data_ptr != nullptr) {
            stbi_image_free(source.data_ptr);
            source.data_ptr = nullptr;
        }
        if (queue != nullptr) {
            clReleaseCommandQueue(queue);
            queue = nullptr;
        }
    }

    ImageNode::ImageNode() :
        queue(nullptr),
        state({ .source = {}, .selected = false }) {
    }
}

