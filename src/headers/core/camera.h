//
// Created by xd on 15/12/24.
//

#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/videoio.hpp>

#include "../../../extra/data/struct/array.h"
#include "../../../platforms/device.h"
#include "../cl/ocl_interop.h"
#include "../utils/log_utils.h"

namespace ex::camera {
    using dev_caps = ex::data::array<platform::dev::DeviceCapability>;

    struct config {
        char codec[4] = "";
        int  width    = -1;
        int  height   = -1;
        int  buffer   = -1;
        int  fps      = -1;
    };

    class Camera {
    private:
        cl_context   ocl_context = nullptr;
        cl_device_id ocl_device  = nullptr;

        cl_command_queue queue       = nullptr;
        bool             cap_called_ = false;
        bool             initialized = false;
        bool             grab_called = false;

        cv::VideoCapture cap_;
        dev_caps         capabilities_;
        const char      *device_id;
        config           config_;

    public:
        Camera(cl_context context, cl_device_id device);

        ~Camera();

        int get_capture_api() const;

        uint get_device_index() const;

        config const &get_config() const;

        void open(const char *device_uuid);

        void open(const char *device_uuid, const config &config);

        void close();

        xm::ocl::iop::ClImagePromise capture(bool force_grab = false);

        bool grab();

        void set_control(platform::dev::control_id id, int value);

        ex::data::array<platform::dev::VideoControl> get_controls() const;

        void reset_controls();

        ex::data::array<platform::dev::DeviceCapability> get_capabilities();
    };

}

#endif //CAMERA_H
