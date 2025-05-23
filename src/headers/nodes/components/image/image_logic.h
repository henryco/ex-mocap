//
// Created by xd on 4/9/25.
//

#ifndef EX_NODES_CMP_IMAGE_LOGIC_H
#define EX_NODES_CMP_IMAGE_LOGIC_H

#include "../../../cl/cl_image.h"
#include "../../../io/byte_serializer.h"
#include "../../../logic/logic_nodes.h"

namespace ex::logic::node::image {

    struct image_state {
        char       source[255];
        id_type_16 image_id;
        bool       selected;
    };

    struct image_source {
        unsigned char *data_ptr = nullptr;
        unsigned short width    = 0;
        unsigned short height   = 0;
        unsigned char  channels = 0;
    };

    struct ImageNode final : limbo::graph::BaseNode,
                             limbo::graph::id::name<"image_source_node">,
                             limbo::graph::io::out<ocl::img_promise, 1, "Loaded image">,
                             ex::io::bytes::serializable
    {
        cl_command_queue  queue_;
        ocl::ImagePromise promise_;
        ocl::Image2D      image_;
        image_source      source_;

        image_state       state;

        limbo::graph::UpdateState updates_;

        static limbo::graph::BaseNode *instance(TreeNode *from);
        static limbo::graph::BaseNode *relocate(TreeNode *tree, limbo::graph::BaseNode *node);
        static void                    destroy(TreeNode *tree, limbo::graph::BaseNode *node);

        void set_image_id(id_type_16 id);

        void set_image(const char *image_path);

        unsigned get_image_w() const;

        unsigned get_image_h() const;

        io::byte_array serialize() const override;

        void deserialize(const io::byte_array &) override;

        void on_input(int in_idx, int out_idx, int clock_i, void *data) override;

        void *on_output(int in_idx, int out_idx, int clock_i) override;

        void on_update(int clock_i) override;

        ~ImageNode() override;

        ImageNode();
    };

}

#endif //EX_NODES_CMP_IMAGE_LOGIC_H
