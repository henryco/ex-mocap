//
// Created by xd on 4/30/25.
//

#ifndef EX_NODES_CMP_CAMERA_LOGIC_H
#define EX_NODES_CMP_CAMERA_LOGIC_H

#include "../../../cl/cl_image.h"
#include "../../../io/byte_serializer.h"
#include "../../../logic/logic_nodes.h"

namespace ex::logic::node::camera {

    struct CameraNode final : limbo::graph::BaseNode,
                              limbo::graph::id::name<"camera_node">,
                              limbo::graph::io::out<ocl::img_promise, 1, "Frame image">,
                              ex::io::bytes::serializable
    {

        cl_command_queue  queue_;
        ocl::ImagePromise promise_;

        static limbo::graph::BaseNode *instance(TreeNode *from);
        static limbo::graph::BaseNode *relocate(TreeNode *tree, limbo::graph::BaseNode *node);
        static void                    destroy(TreeNode *tree, limbo::graph::BaseNode *node);

        void set_image_id(id_type_16 id);

        io::byte_array serialize() const override;

        void deserialize(const io::byte_array &) override;

        void on_input(int in_idx, int out_idx, int clock_i, const void *data) override;

        void on_update(int clock_i) override;

        void *on_output(int in_idx, int out_idx, int clock_i) override;

        ~CameraNode() override;

        CameraNode();
    };

}

#endif //EX_NODES_CMP_CAMERA_LOGIC_H
