//
// Created by xd on 29/11/24.
//

#ifndef EX_MOTION_DEVICE_H
#define EX_MOTION_DEVICE_H

#include <string>
#include <vector>

namespace ex::platform::dev {

    using control_id = uint;

    struct VideoControl {
        control_id id;
        std::string name;
        int min;
        int max;
        int step;
        int default_value;
        int value;
    };

    struct VideoDevice {
        std::string uuid;
        std::string name;
    };

    struct DeviceCapability {
        std::string description;
        std::string format_4cc;
        int refresh_rate;
        int width;
        int height;
    };

    int get_capture_api();

    void refresh();

    std::vector<VideoDevice> list_devices();

    std::vector<VideoControl> list_controls(const std::string &device_uuid);

    std::vector<VideoControl> list_controls(const VideoDevice &device);

    std::vector<DeviceCapability> list_capabilities(const std::string &device_uuid);

    bool set_control_value(const std::string &device_uuid, control_id prop_id, int value);

    bool set_control_value(const VideoDevice &device, control_id prop_id, int value);

    int get_device_index(const std::string &device_uuid);

    int get_device_index(const VideoDevice &device);

    std::string get_device_id(const std::string &device_uuid);

    void reset_controls(const std::string &device_uuid);

    void reset_controls(const VideoDevice &device);
}

#endif // EX_MOTION_DEVICE_H
