//
// Created by xd on 2/24/25.
//

#ifndef EX_LOGIC_OUTPUT_DISPLAY_H
#define EX_LOGIC_OUTPUT_DISPLAY_H

#include "../../../core/image_protocol.h"
#include "../logic_nodes.h"

namespace ex::logic::node::out {
    namespace ln = limbo::graph;

    struct DisplayOutputNode final : limbo::graph::BaseNode,
                                     limbo::graph::id::name<"display_output_node">,
                                     limbo::graph::io::in<proto::image *, 1, "Image to display">
    {
        const char   *display_n = nullptr;
        unsigned int  display_i = 0;

        // GL/CL/ETC interop internal
        unsigned char *data_ = nullptr;
        unsigned long  size_ = 0;
        unsigned int   i_w   = 0;
        unsigned int   i_h   = 0;
        // GL/CL/ETC interop internal

        static limbo::graph::BaseNode *instance(TreeNode *from);
        static limbo::graph::BaseNode *relocate(TreeNode *tree, limbo::graph::BaseNode *node);
        static void                    destroy(TreeNode *tree, limbo::graph::BaseNode *node);

        void register_image(const char *name);

        void rename_image(const char *name);

        void on_input(int in_idx, int out_idx, int clock_i, const void *data) override;

        void on_output(int in_idx, int out_idx, int clock_i, void *data) override;

        ~DisplayOutputNode() override;

        DisplayOutputNode();
    };

}

#endif //EX_LOGIC_OUTPUT_DISPLAY_H
