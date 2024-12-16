//
// Created by xd on 15/12/24.
//

#include "../../headers/core/camera.h"

namespace ex::camera {

    int fourCC(const char *name) {
        return cv::VideoWriter::fourcc(
                name[0],
                name[1],
                name[2],
                name[3]);
    }

    void Camera::open(std::string const &device_uuid) {
        close();
        cap_.open(platform::dev::get_device_index(device_uuid),
            platform::dev::get_device_index(device_uuid));
        device_id = device_uuid;
        cap_called_ = false;
    }

    void Camera::open(std::string const &device_uuid, const config &prop) {
        close();
        std::vector<int> params;

        if (prop.codec.size() >= 4) {
            params.push_back(cv::CAP_PROP_FOURCC);
            params.push_back(fourCC(prop.codec.c_str()));
        }

        if (prop.width > 0 && prop.height > 0) {
            params.push_back(cv::CAP_PROP_FRAME_WIDTH);
            params.push_back(prop.width);

            params.push_back(cv::CAP_PROP_FRAME_HEIGHT);
            params.push_back(prop.height);
        }

        if (prop.fps > 0) {
            params.push_back(cv::CAP_PROP_FPS);
            params.push_back(prop.fps);
        }

        if (prop.buffer >= 0) {
            params.push_back(cv::CAP_PROP_BUFFERSIZE);
            params.push_back(prop.buffer);
        }

        if (params.size() > 0) {
            cap_.open(platform::dev::get_device_index(device_uuid),
            platform::dev::get_device_index(device_uuid),
            params);
        }

        else {
            cap_.open(platform::dev::get_device_index(device_uuid),
                platform::dev::get_device_index(device_uuid));
        }

        config_ = prop;
        device_id = device_uuid;
        cap_called_ = false;
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

    std::vector<platform::dev::DeviceCapability> Camera::get_capabilities() {
        if (!cap_called_) {
            capabilities_ = platform::dev::list_capabilities(device_id);
            cap_called_ = true;
        }
        return capabilities_;
    }

    void Camera::close() {
        cap_called_ = false;
        cap_.release();
    }

} // namespace ex::camera
