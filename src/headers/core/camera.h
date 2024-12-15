//
// Created by xd on 15/12/24.
//

#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/videoio.hpp>
#include <string>

#include "../../../platforms/device.h"
#include "../utils/log_utils.h"
#include "../cl/ocl_data.h"

namespace ex::camera {

    struct config {
        std::string uuid;
        std::string codec;
        int         width;
        int         height;
        int         buffer;
        int         fps;
        int         x;
        int         y;
        int         w;
        int         h;
        bool        flip_x;
        bool        flip_y;
        bool        rotate;
    };

    class Camera {
    private:
        static inline const auto log =
            logs::get_logger("camera");

        cv::VideoCapture cap_;
        std::string      device_id;
        config           config_;
        bool             fast = false;

    public:
        Camera() = default;

        ~Camera() = default;

        int get_capture_api() const;

        uint get_device_index() const;

        config const &get_config() const;

        void open(const config &config);

        void close();

        xm::ocl::Image2D capture();

        void set_control(platform::dev::control_id id, int value);

        std::vector<platform::dev::VideoControl> get_controls() const;

        void reset_controls();

        void set_fast_mode(bool fast = true);
    };

}

#endif //CAMERA_H
