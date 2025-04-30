//
// Created by xd on 4/30/25.
//

#ifndef EX_NODES_CMP_CAMERA_LOGIC_H
#define EX_NODES_CMP_CAMERA_LOGIC_H

#include "../../../cl/cl_image.h"
#include "../../../io/byte_serializer.h"
#include "../../../logic/logic_nodes.h"

#include "../../../../platforms/device.h"

namespace ex::logic::node::camera {

    using v_device_t = ex::platform::dev::VideoDevice;

    struct camera_state {
        v_device_t device;
        id_type_16 image_id;
        bool       selected;
    };

    struct CameraNode final : limbo::graph::BaseNode,
                              limbo::graph::id::name<"camera_node">,
                              limbo::graph::io::out<ocl::img_promise, 1, "Frame image">,
                              ex::io::bytes::serializable
    {
        camera_state      state;

        cl_command_queue  queue_;
        ocl::ImagePromise promise_;

        static limbo::graph::BaseNode *instance(TreeNode *from);
        static limbo::graph::BaseNode *relocate(TreeNode *tree, limbo::graph::BaseNode *node);
        static void                    destroy(TreeNode *tree, limbo::graph::BaseNode *node);

        void set_image_id(id_type_16 id);

        void set_device(const v_device_t &device);

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
