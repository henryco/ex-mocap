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

    using v_device_t  = ex::platform::dev::VideoDevice;
    using v_devcap_t  = ex::platform::dev::DeviceCapability;
    using v_control_t = ex::platform::dev::VideoControl;

    struct capture_t;

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
        ex::data::array<v_devcap_t>  capabilities;
        ex::data::array<v_control_t> controls;

        limbo::graph::UpdateState updates_;

        capture_t        *capture;
        camera_state      state;
        ocl::ImagePromise promises_[2];
        unsigned int      b_index_;
        cl_command_queue  queue_;
        int               buffer_size;
        bool              enabled;

        static limbo::graph::BaseNode *instance(TreeNode *from);
        static limbo::graph::BaseNode *relocate(TreeNode *tree, limbo::graph::BaseNode *node);
        static void                    destroy(TreeNode *tree, limbo::graph::BaseNode *node);

        const ex::data::array<v_devcap_t> &list_capabilities() const;

        const ex::data::array<v_control_t> &list_controls(bool refresh = false);

        void close();

        void open();

        void set_image_id(id_type_16 id);

        void set_device_buff(int buffer_size);

        void set_device_mode(const v_devcap_t &cap);

        void set_device(const v_device_t &device);

        void set_control(unsigned int id, int value);

        void reset_controls();

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
