//
// Created by xd on 30/11/24.
//

#ifndef EX_MOTION_V4L2_DEVICE_H
#define EX_MOTION_V4L2_DEVICE_H

#include <linux/videodev2.h>
#include <cstdint>
#include <string>
#include <vector>

namespace ex::platform::dev::v4l2 {

    struct v4l2_queryctrl_ext : public v4l2_queryctrl {
        __s32 value;
    };

    struct serial_v4l2_control {
        std::uint32_t id;
        int32_t value;
    };

    typedef struct v4l2_queryctrl_ext V4L2_QueryCtrl;
    typedef struct v4l2_control V4L2_Control;

    struct V4L2Device {
        std::string device_id;
        std::string index;
        std::string uuid;
        std::string name;
    };

    std::vector<V4L2Device> list_usb_camera_devices();

}

#endif // EX_MOTION_V4L2_DEVICE_H
