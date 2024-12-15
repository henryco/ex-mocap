//
// Created by xd on 15/12/24.
//

#include "../../headers/components/c_camera.h"

namespace ex::cmp {

    CameraNode::~CameraNode() {
        camera_.close();
    }

    const camera::Camera *CameraNode::get() const {
        return &camera_;
    }

    bool CameraNode::on_input(bool data, int input_index, int output_index, int clock_i) {
        // TODO
        return true;
    }

    xm::ocl::Image2D CameraNode::on_output(xm::ocl::Image2D *null_ptr, int output_index, int clock_i) {
        return camera_.capture();
    }

} // namespace ex::cmp
