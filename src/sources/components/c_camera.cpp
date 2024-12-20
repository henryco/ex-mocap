//
// Created by xd on 15/12/24.
//

#include "../../headers/components/c_camera.h"

namespace ex::cmp {

    CameraNode::CameraNode(cl_context context, cl_device_id device)
    : camera_(context, device) {}

    CameraNode::CameraNode(const camera::Camera &camera)
    : camera_(camera) {}

    CameraNode::~CameraNode() {
        camera_.close();
    }

    camera::Camera *CameraNode::get() {
        return &camera_;
    }

    bool CameraNode::on_input(const bool data, const int input_index, const int _, const int clock_i) {
        switch (input_index) {
            case CAM_IN_ENABLE:
                enabled_ = data;
                return true;
            default:
                return true;
        }
    }

    promise_i CameraNode::on_output(promise_i *_, const int __, const int clock_i) {
        if (!enabled_)
            return {};
        capture_frame(clock_i);
        return last_frame_;
    }

    bool CameraNode::on_output(bool *_, const int __, const int clock_i) {
        if (!enabled_)
            return false;
        return camera_.grab();
    }

    void CameraNode::capture_frame(const int clock_i) {
        if (clock_i == last_clock_)
            return;
        last_clock_ = clock_i;
        last_frame_ = camera_.capture();
    }

} // namespace ex::cmp
