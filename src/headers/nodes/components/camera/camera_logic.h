//
// Created by xd on 4/30/25.
//

#ifndef EX_NODES_CMP_CAMERA_LOGIC_H
#define EX_NODES_CMP_CAMERA_LOGIC_H

#include "../../../cl/cl_image.h"
#include "../../../io/byte_serializer.h"
#include "../../../logic/logic_nodes.h"

#include "../../../../platforms/device.h"

namespace ex::logic::node::sync {

    struct sync_t;

    struct SyncNode final : limbo::graph::BaseNode,
                            limbo::graph::id::name<"sync_node">,
                            limbo::graph::io::out<sync_t, 1, "Synchronizer">
    {
        // ---- internal ----

        struct internal_t *internal;

        // ---- mapper properties ----

        static limbo::graph::BaseNode *instance(TreeNode *from);
        static limbo::graph::BaseNode *relocate(TreeNode *tree, limbo::graph::BaseNode *node);
        static void                    destroy(TreeNode *tree, limbo::graph::BaseNode *node);

        // ---- sync properties ----

        void restart();

        void enable();

        void disable();

        // ---- node properties ----

        void on_input(int in_idx, int out_idx, int clock_i, void *data) override;

        void on_update(int clock_i) override;

        void *on_output(int in_idx, int out_idx, int clock_i) override;

        ~SyncNode() override;

        SyncNode();
    };
}

namespace ex::logic::node::camera {

    using v_device_t  = ex::platform::dev::VideoDevice;
    using v_devcap_t  = ex::platform::dev::DeviceCapability;
    using v_control_t = ex::platform::dev::VideoControl;

    struct internal_t;
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
        device_t   device;
        mode_t     mode;
        short      buffering;
        id_type_16 image_id;
    };

    struct timings_t {
        unsigned long t0;
        float frame_rate;
    };

    struct CameraNode final : limbo::graph::BaseNode,
                              limbo::graph::id::name<"camera_node">,
                              limbo::graph::io::in<sync::sync_t, 1, "Synchronizer">,
                              limbo::graph::io::out<ocl::img_promise, 1, "Frame image">,
                              ex::io::bytes::serializable
    {
        ex::data::array<v_devcap_t>  capabilities;
        ex::data::array<v_control_t> controls;

        // ---- internal ----

        internal_t *internal;
        capture_t  *capture;


        // ---- exposed ----

        camera_state state;
        timings_t    timings;


        // ---- mapper properties ----

        static limbo::graph::BaseNode *instance(TreeNode *from);
        static limbo::graph::BaseNode *relocate(TreeNode *tree, limbo::graph::BaseNode *node);
        static void                    destroy(TreeNode *tree, limbo::graph::BaseNode *node);


        // ---- global properties ----

        static const ex::data::array<v_device_t> &list_camera_devices(bool refresh = false);


        // ---- camera properties ----

        const ex::data::array<v_devcap_t> &list_capabilities() const;

        const ex::data::array<v_control_t> &list_controls(bool refresh = false);

        void close();

        void reset();

        void open();

        void set_image_id(id_type_16 id);

        void set_device_buff(short buffer_size);

        void set_device_mode(const v_devcap_t &cap);

        void set_device(const v_device_t &device);

        void set_control(unsigned int id, int value);

        void reset_controls();


        // ---- node properties ----

        io::byte_array serialize() const override;

        void deserialize(const io::byte_array &) override;

        void on_input(int in_idx, int out_idx, int clock_i, void *data) override;

        void on_update(int clock_i) override;

        void *on_output(int in_idx, int out_idx, int clock_i) override;

        ~CameraNode() override;

        CameraNode();
    };

}

#endif //EX_NODES_CMP_CAMERA_LOGIC_H
