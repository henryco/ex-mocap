//
// Created by xd on 15/12/24.
//

#include "../../headers/core/camera.h"

namespace ex::camera {


    Camera::~Camera() {
        close();
    }

    int  Camera::get_capture_api() const {
        return platform::dev::get_capture_api();
    }

    uint Camera::get_device_index() const {
        return 0;
    }

    config const &Camera::get_config() const {
        return {};
    }

    void Camera::open(const config &config) {

    }

    void Camera::close() {

    }

    xm::ocl::Image2D Camera::capture() {
        return xm::ocl::Image2D();
    }

    void Camera::set_control(platform::dev::control_id id, int value) {

    }

    int Camera::get_control(platform::dev::control_id id) const {
        return 0;
    }

    std::vector<platform::dev::VideoControl> Camera::get_controls() const {
        return {};
    }

    void Camera::reset_controls() {

    }

    void Camera::set_fast_mode(bool fast) {

    }


} // namespace ex::camera
