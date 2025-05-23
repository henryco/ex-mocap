//
// Created by xd on 4/9/25.
//

#include "../../../../headers/nodes/components/image/image_logic.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/logic/logic_nodes.h"
#include "../../../../headers/cl/cl_kernel.h"

// STB INCLUDES ---------------------------------
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
// STB INCLUDES ---------------------------------

namespace ex::logic::node::image {

    limbo::graph::BaseNode *ImageNode::instance(TreeNode *node) {
        ImageNode *ptr = new ImageNode();
        ptr->set_node_id(node->id_node);
        ptr->set_image_id(node->id_node);
        return ptr;
    }

    limbo::graph::BaseNode *ImageNode::relocate(TreeNode *, limbo::graph::BaseNode *node) {
        return node;
    }

    void ImageNode::destroy(TreeNode *, limbo::graph::BaseNode *node) {
        node->detach_all();
        delete node;
    }

    void ImageNode::set_image_id(const id_type_16 id) {
        state.image_id    = static_cast<id_type_16>(id);
        promise_.image.id = static_cast<id_type_16>(id);
        image_.id         = static_cast<id_type_16>(id);
    }

    struct serial_parcel {
        image_state  state;
        unsigned int w, h, c, s;
    };

    io::byte_array ImageNode::serialize() const {
        unsigned int  image_size = source_.channels * source_.width * source_.height;
        unsigned long total_size = sizeof(serial_parcel) + image_size;

        serial_parcel parcel;
        parcel.state = state;
        parcel.w = source_.width;
        parcel.h = source_.height;
        parcel.c = source_.channels;
        parcel.s = image_size;

        unsigned char *bytes = static_cast<unsigned char *>(::malloc(total_size));
        ::memcpy(bytes, &parcel, sizeof(serial_parcel));
        if (source_.data_ptr != nullptr) {
            ::memcpy(bytes + sizeof(serial_parcel), source_.data_ptr, image_size);
        }

        return io::byte_array::from(bytes, total_size, &::free);
    }

    void ImageNode::deserialize(const io::byte_array &array) {
        const ex::State &state_ = ex::logic::globals::state();

        if (array.empty())
            return;

        if (source_.data_ptr != nullptr) {
            stbi_image_free(source_.data_ptr);
            source_.data_ptr = nullptr;
        }

        if (queue_ == nullptr) {
            queue_ = xm::ocl::create_queue_device(state_.cl.context, state_.cl.device, true, false);
        }

        serial_parcel *parcel = reinterpret_cast<serial_parcel *>(array.data);
        state = parcel->state;

        if (parcel->s <= 0)
            return;

        source_.data_ptr = static_cast<unsigned char *>(stbi__malloc(parcel->s));
        source_.channels = parcel->c;
        source_.height   = parcel->h;
        source_.width    = parcel->w;

        ::memcpy(source_.data_ptr, array.data + sizeof(serial_parcel), parcel->s);

        image_.release();

        ocl::ImagePromise promise = ex::ocl::ops::from_bytes(
            source_.data_ptr, source_.width, source_.height,
            source_.channels, 1, state_.cl.context, queue_);
        promise.finalize();

        promise_          = promise;
        promise_.image.id = state.image_id;
        image_            = promise_.image;
        updates_.refresh();
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
        int w_, h_, c_;
        source_.data_ptr = stbi_load(image_path, &w_, &h_, &c_, 4);
        source_.width    = w_;
        source_.height   = h_;
        source_.channels = c_;

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

        promise_          = promise;
        promise_.image.id = state.image_id;
        image_            = promise_.image;
        updates_.refresh();
    }

    unsigned ImageNode::get_image_w() const {
        return image_.cols;
    }

    unsigned ImageNode::get_image_h() const {
        return image_.rows;
    }

    void ImageNode::on_input(int, int, int, void *) {
    }

    void *ImageNode::on_output(const int in_idx, const int out_idx, const int clock_i) {
        if (source_.data_ptr == nullptr)
            return nullptr;
        return &promise_;
    }

    void ImageNode::on_update(int clock_i) {
        promise_.image.header.fresh = updates_.update();
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
        state({ .source = {}, .image_id = 0, .selected = false }) {
    }
}

