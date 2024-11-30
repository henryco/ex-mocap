//
// Created by xd on 29/11/24.
//

#ifndef EX_MOTION_DEVICE_H
#define EX_MOTION_DEVICE_H

#include <string>
#include <vector>

namespace ex::platform::dev {

    struct VideoDevice {
        std::string uuid;
        std::string name;
    };

    std::vector<VideoDevice> list_devices();
}

#endif // EX_MOTION_DEVICE_H
