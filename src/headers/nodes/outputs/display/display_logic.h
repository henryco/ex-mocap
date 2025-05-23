//
// Created by xd on 2/24/25.
//

#ifndef EX_NODES_OUTPUT_DISPLAY_LOGIC_H
#define EX_NODES_OUTPUT_DISPLAY_LOGIC_H

#include "../../../cl/cl_image.h"
#include "../../../logic/logic_nodes.h"

namespace ex::logic::node::out {
    namespace ln = limbo::graph;

    struct DisplayOutputNode final : limbo::graph::BaseNode,
                                     limbo::graph::id::name<"display_output_node">,
                                     limbo::graph::io::in<ocl::img_promise, 1, "Image to display">
    {
        const char   *display_n = nullptr;
        unsigned int  display_i = 0;

        void *prev_ = nullptr;

        // GL/CL/ETC interop internal
        unsigned char   *data_ = nullptr;
        unsigned long    size_ = 0;
        unsigned int     i_w_  = 0;
        unsigned int     i_h_  = 0;

        ocl::Image2D     image_;
        ocl::NBPromise   promise_worker_;
        cl_command_queue queue_;

        limbo::graph::UpdateState updates_;
        limbo::graph::UpdateState supplies_;
        // GL/CL/ETC interop internal

        bool enabled = true;

        static limbo::graph::BaseNode *instance(TreeNode *from);
        static limbo::graph::BaseNode *relocate(TreeNode *tree, limbo::graph::BaseNode *node);
        static void                    destroy(TreeNode *tree, limbo::graph::BaseNode *node);

        void register_image(const char *name);

        void rename_image(const char *name);

        void reset();

        void disable();

        void enable();

        void on_input(int in_idx, int out_idx, int clock_i, void *data) override;

        void *on_output(int in_idx, int out_idx, int clock_i) override;

        void on_update(int clock_i) override;

        ~DisplayOutputNode() override;

        DisplayOutputNode();
    };

}

#endif //EX_NODES_OUTPUT_DISPLAY_LOGIC_H
