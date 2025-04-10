//
// Created by xd on 4/9/25.
//

#ifndef EX_LOGIC_CMP_IMAGE_H
#define EX_LOGIC_CMP_IMAGE_H

#include "../../../core/image_protocol.h"
#include "../logic_nodes.h"

namespace ex::logic::node::cmp::image {

    struct image_state {
        char source[255];
        bool selected;
    };

    struct ImageNode final : limbo::graph::BaseNode,
                             limbo::graph::id::name<"image_source_node">,
                             limbo::graph::io::out<proto::image, 1, "Loaded image">
    {
        image_state state;

        static limbo::graph::BaseNode *instance(TreeNode *from);
        static limbo::graph::BaseNode *relocate(TreeNode *tree, limbo::graph::BaseNode *node);
        static void                    destroy(TreeNode *tree, limbo::graph::BaseNode *node);

        void set_image(const char *image_path);

        void on_input(int in_idx, int out_idx, int clock_i, const void *data) override;

        void on_output(int in_idx, int out_idx, int clock_i, void *data) override;

        ~ImageNode() override;

        ImageNode();
    };

}

#endif //EX_LOGIC_CMP_IMAGE_H
