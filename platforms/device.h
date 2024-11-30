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

    void refresh();

    std::vector<VideoDevice> list_devices();

    int get_device_index(const std::string &id);

    int get_device_index(const VideoDevice &device);
}

#endif // EX_MOTION_DEVICE_H
