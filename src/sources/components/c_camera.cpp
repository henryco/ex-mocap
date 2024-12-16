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
            case CAM_ENABLE:
                enabled_ = data;
                return true;
            case CAM_FREEZE:
                freeze_ = data;
                return true;
            default:
                return true;
        }
    }

    bool CameraNode::on_input(const int data, const int input_index, const int _, const int clock_i) {
        switch (input_index) {
            case CAM_WIDTH:
                return update_value(&config_.width, data);
            case CAM_HEIGHT:
                return update_value(&config_.height, data);
            case CAM_BUFFER:
                return update_value(&config_.buffer, data);
            case CAM_FPS:
                return update_value(&config_.fps, data);
            default:
                return true;
        }
    }

    bool CameraNode::on_input(const char *data, const int input_index, const int _, const int clock_i) {
        switch (input_index) {
            case CAM_CODEC:
                return update_value(&config_.codec, std::string(data));
            case CAM_UUID:
                return update_value(&device_uuid, std::string(data));
            default:
                return true;
        }
    }

    xm::ocl::iop::ClImagePromise CameraNode::on_output(xm::ocl::iop::ClImagePromise *_, const int __, const int clock_i) {
        if (!enabled_)
            return {};
        if (!initialized || (updated_ && !freeze_)) {
            camera_.close();
            camera_.open(device_uuid, config_);
            initialized = true;
            updated_ = false;
        }
        grab_frame(clock_i);
        return last_frame_;
    }

    void CameraNode::grab_frame(const int clock_i) {
        if (clock_i == last_clock_)
            return;
        last_clock_ = clock_i;
        last_frame_ = camera_.capture();
    }

} // namespace ex::cmp
