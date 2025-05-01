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
    using v_devcap_t = ex::platform::dev::DeviceCapability;

    struct device_t {
        v_device_t  device;
        bool        selected;
    };

    struct mode_t {
        v_devcap_t mode;
        char       description[96];
        bool       selected;
    };

    struct camera_state {
        device_t    device;
        mode_t      mode;
        id_type_16  image_id;
    };

    struct CameraNode final : limbo::graph::BaseNode,
                              limbo::graph::id::name<"camera_node">,
                              limbo::graph::io::out<ocl::img_promise, 1, "Frame image">,
                              ex::io::bytes::serializable
    {
        ex::data::array<v_devcap_t> capabilities;

        camera_state      state;

        cl_command_queue  queue_;
        ocl::ImagePromise promise_;

        static limbo::graph::BaseNode *instance(TreeNode *from);
        static limbo::graph::BaseNode *relocate(TreeNode *tree, limbo::graph::BaseNode *node);
        static void                    destroy(TreeNode *tree, limbo::graph::BaseNode *node);

        const ex::data::array<v_devcap_t> &list_capabilities() const;

        void set_image_id(id_type_16 id);

        void set_device_mode(const v_devcap_t &cap);

        void set_device(const v_device_t &device);

        io::byte_array serialize() const override;

        void deserialize(const io::byte_array &) override;

        void on_input(int in_idx, int out_idx, int clock_i, const void *data) override;

        void on_update(int clock_i) override;

        void *on_output(int in_idx, int out_idx, int clock_i) override;

        ~CameraNode() override;

        CameraNode();
    };

    const ex::data::array<v_device_t> &list_camera_devices(bool refresh = false);

}

#endif //EX_NODES_CMP_CAMERA_LOGIC_H
