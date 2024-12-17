//
// Created by xd on 15/12/24.
//

#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/videoio.hpp>
#include <string>

#include "../../../platforms/device.h"
#include "../cl/ocl_interop.h"
#include "../utils/log_utils.h"

namespace ex::camera {
    using dev_caps = std::vector<platform::dev::DeviceCapability>;

    struct config {
        std::string codec  = "";
        int         width  = -1;
        int         height = -1;
        int         buffer = -1;
        int         fps    = -1;
    };

    class Camera {
    private:
        static inline const auto log =
            logs::get_logger("camera");

        cl_context   ocl_context = nullptr;
        cl_device_id ocl_device  = nullptr;

        cl_command_queue queue       = nullptr;
        bool             cap_called_ = false;
        bool             initialized = false;

        cv::VideoCapture cap_;
        dev_caps         capabilities_;
        std::string      device_id;
        config           config_;

    public:
        Camera(cl_context context, cl_device_id device);

        ~Camera();

        int get_capture_api() const;

        uint get_device_index() const;

        config const &get_config() const;

        void open(std::string const &device_uuid);

        void open(std::string const &device_uuid, const config &config);

        void close();

        xm::ocl::iop::ClImagePromise capture();

        bool grab();

        void set_control(platform::dev::control_id id, int value);

        std::vector<platform::dev::VideoControl> get_controls() const;

        void reset_controls();

        std::vector<platform::dev::DeviceCapability> get_capabilities();
    };

}

#endif //CAMERA_H
