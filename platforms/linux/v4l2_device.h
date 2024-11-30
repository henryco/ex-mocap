//
// Created by xd on 30/11/24.
//

#ifndef EX_MOTION_V4L2_DEVICE_H
#define EX_MOTION_V4L2_DEVICE_H

#include <string>
#include <vector>

namespace ex::platform::dev::v4l2 {

    struct V4L2Device {
        std::string device_id;
        std::string index;
        std::string uuid;
        std::string name;
    };

    std::vector<V4L2Device> list_usb_camera_devices();

}

#endif // EX_MOTION_V4L2_DEVICE_H
