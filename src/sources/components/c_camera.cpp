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

    bool CameraNode::on_input(const bool data, const int input_index, const int _, const int clock_i) {
        switch (input_index) {
            case CAM_FAST:
                camera_.set_fast_mode(data);
                return true;
            case CAM_FLIP_H:
                config_.flip_x = data;
                return true;
            case CAM_FLIP_V:
                config_.flip_y = data;
                return true;
            case CAM_ROTATE:
                config_.rotate = data;
                return true;
            default:
                return true;
        }
    }

    bool CameraNode::on_input(const int data, const int input_index, const int _, const int clock_i) {
        switch (input_index) {
            case CAM_WIDTH:
                config_.width = data;
                return true;
            case CAM_HEIGHT:
                config_.height = data;
                return true;
            case CAM_BUFFER:
                config_.buffer = data;
                return true;
            case CAM_FPS:
                config_.fps = data;
                return true;
            case CAM_X:
                config_.x = data;
                return true;
            case CAM_Y:
                config_.y = data;
                return true;
            case CAM_W:
                config_.w = data;
                return true;
            case CAM_H:
                config_.h = data;
                return true;
            default:
                return true;
        }
    }

    bool CameraNode::on_input(const char *data, const int input_index, const int _, const int clock_i) {
        switch (input_index) {
            case CAM_CODEC:
                config_.codec = std::string(data);
                return true;
            case CAM_UUID:
                config_.uuid = std::string(data);
                return true;
            default:
                return true;
        }
    }

    xm::ocl::Image2D CameraNode::on_output(xm::ocl::Image2D *null_ptr, int output_index, int clock_i) {
        return camera_.capture();
    }

} // namespace ex::cmp
