//
// Created by xd on 15/12/24.
//

#include "../../headers/components/c_camera.h"

namespace ex::cmp {

    CameraNode::~CameraNode() {
        camera_.close();
    }

    camera::Camera *CameraNode::get() {
        return &camera_;
    }

    bool CameraNode::on_input(bool data, int input_index, int output_index, int clock_i) {
        // TODO
        return true;
    }

    xm::ocl::Image2D CameraNode::on_output(xm::ocl::Image2D *null_ptr, int output_index, int clock_i) {
        return camera_.capture();
    }

    bool CameraNode::on_input(int data, int input_index, int output_index, int clock_i) {
        return true;
    }

    bool CameraNode::on_input(const char *data, int input_index, int output_index, int clock_i) {
        return true;
    }

} // namespace ex::cmp
