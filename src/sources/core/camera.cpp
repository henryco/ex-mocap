//
// Created by xd on 15/12/24.
//

#include "../../headers/core/camera.h"

namespace ex::camera {

    void Camera::open(const config &config) {
        // TODO
    }

    xm::ocl::Image2D Camera::capture() {
        // TODO
        return xm::ocl::Image2D();
    }

    int  Camera::get_capture_api() const {
        return platform::dev::get_capture_api();
    }

    uint Camera::get_device_index() const {
        return platform::dev::get_device_index(device_id);
    }

    config const &Camera::get_config() const {
        return config_;
    }

    void Camera::set_control(const platform::dev::control_id id, const int value) {
        platform::dev::set_control_value(device_id, id, value);
    }

    std::vector<platform::dev::VideoControl> Camera::get_controls() const {
        return platform::dev::list_controls(device_id);
    }

    void Camera::reset_controls() {
        platform::dev::reset_controls(device_id);
    }

    void Camera::set_fast_mode(const bool fast) {
        this->fast = fast;
    }

    void Camera::close() {
        cap_.release();
    }

} // namespace ex::camera
