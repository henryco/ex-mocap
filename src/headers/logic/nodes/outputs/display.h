//
// Created by xd on 2/24/25.
//

#ifndef EX_LOGIC_OUTPUT_DISPLAY_H
#define EX_LOGIC_OUTPUT_DISPLAY_H

#include "../../../core/image_protocol.h"
#include "../logic_nodes.h"

namespace ex::logic::node::out {
    namespace ln = limbo::nodes;

    struct DisplayOutputNode : ln::Node<DisplayOutputNode>,
                               ln::id::name<"display_output_node">,
                               ln::io::in<proto::image, 1, "Image to display">,
                               ln::ReactiveIn<proto::image>,
                               ln::ReactiveOutAnyCallback
    {
        const char  *display_n = nullptr;
        unsigned int display_w = 0;
        unsigned int display_h = 0;
        unsigned int display_c = 3;
        unsigned int display_d = 8;

        static ln::BaseNode *instance(TreeNode *from);
        static ln::BaseNode *relocate(TreeNode *tree, ln::BaseNode *node);
        static void          destroy(TreeNode *tree, ln::BaseNode *node);

        void register_image(const char *name, uint w, uint h, uint channels = 3, uint bit_depth = 8);

        bool on_input(proto::image data, int input_index, int output_index, int clock_i,
                      limbo::nodes::Output<proto::image> *io) override;

        void *out_any(int output_index, int clock_i, std::size_t *size) override;

        void *out_any(int output_index, int clock_i, std::size_t *size, IN *io) override;

        DisplayOutputNode();

        ~DisplayOutputNode() override;
    };

}

#endif //EX_LOGIC_OUTPUT_DISPLAY_H
