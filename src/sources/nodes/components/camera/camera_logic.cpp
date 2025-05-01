//
// Created by xd on 4/30/25.
//

#include "../../../../headers/nodes/components/camera/camera_logic.h"

#include "../../../../../platforms/linux/v4l2_device.h"
#include "../../../../headers/cl/cl_kernel.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/logic/logic_nodes.h"

namespace ex::logic::node::camera {

    namespace {
        const auto log = logs::get_logger("node_camera");
    }

    const ex::data::array<v_device_t> &list_camera_devices(const bool refresh) {
        static ex::data::array<v_device_t> list = ex::platform::dev::list_devices();
        if (refresh) {
            list = ex::platform::dev::list_devices();
        }
        return list;
    }

    const ex::data::array<v_control_t> &CameraNode::list_controls(const bool refresh) {
        if (refresh) {
            if (!state.device.selected)
                return controls;
            if (!state.mode.selected)
                return controls;
            // TODO CHECK OPEN
            controls = ex::platform::dev::list_controls(state.device.device);
        }
        return controls;
    }

    const ex::data::array<v_devcap_t> &CameraNode::list_capabilities() const {
        return capabilities;
    }

    limbo::graph::BaseNode *CameraNode::instance(TreeNode *from) {
        CameraNode *ptr = new CameraNode();
        ptr->set_id(from->id_node);
        ptr->set_image_id(from->id_node);
        return ptr;
    }

    limbo::graph::BaseNode *CameraNode::relocate(TreeNode *, limbo::graph::BaseNode *node) {
        return node;
    }

    void CameraNode::destroy(TreeNode *, limbo::graph::BaseNode *node) {
        node->detach_all();
        delete node;
    }

    void CameraNode::set_image_id(const id_type_16 id) {
        promise_.image.id = static_cast<id_type_16>(id);
        state.image_id    = static_cast<id_type_16>(id);
        // TODO
    }

    void CameraNode::set_device(const v_device_t &device) {
        // TODO release camera

        ::memset(&state.device, 0, sizeof(device_t));
        ::memset(&state.mode, 0, sizeof(mode_t));
        capabilities.clean();
        controls.clean();

        if (ex::platform::dev::get_device_index(device.uuid) != -1) {
            state.device.device   = device;
            state.device.selected = true;
            capabilities          = ex::platform::dev::list_capabilities(device);
        }
    }

    void CameraNode::set_device_mode(const v_devcap_t &cap) {
        // TODO release camera

        ::memset(&state.mode, 0, sizeof(mode_t));
        controls.clean();

        state.mode.mode     = cap;
        state.mode.selected = true;
        snprintf(state.mode.description, 95, "%s (%s) - %d fps [%d x %d]", cap.description, cap.format_4cc,
                 cap.refresh_rate, cap.width, cap.height);
        state.mode.description[95] = '\0';

        // TODO open camera

        controls = ex::platform::dev::list_controls(state.device.device);
    }

    void CameraNode::set_control(const unsigned int id, const int value) {
        if (!state.device.selected)
            return;
        if (!state.mode.selected)
            return;

        // TODO CHECK OPEN

        ex::platform::dev::set_control_value(state.device.device, id, value);
        log->info("set camera settings: {}, {}", id, value);
    }

    void CameraNode::reset_controls() {
        if (!state.device.selected)
            return;
        if (!state.mode.selected)
            return;

        // TODO CHECK OPEN

        ex::platform::dev::reset_controls(state.device.device);
        log->info("reset camera settings");
    }

    io::byte_array CameraNode::serialize() const {
        // TODO
        return io::byte_array::from(nullptr, 0);
    }

    void CameraNode::deserialize(const io::byte_array &) {
        // TODO
    }

    void CameraNode::on_input(const int in_idx, const int out_idx, const int clock_i, const void *data) {
        // TODO
    }

    void CameraNode::on_update(const int clock_i) {
        // TODO
    }

    void *CameraNode::on_output(const int in_idx, const int out_idx, const int clock_i) {
        // TODO
        return &promise_;
    }

    CameraNode::~CameraNode() {
        // TODO
        if (queue_ != nullptr) {
            clReleaseCommandQueue(queue_);
            queue_ = nullptr;
        }
    }

    CameraNode::CameraNode() :
        state({}),
        queue_(nullptr) {
        ::memset(&state, 0, sizeof(camera_state));
    }

}
