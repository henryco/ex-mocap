//
// Created by xd on 29/11/24.
//

#ifndef EX_MOTION_DEVICE_H
#define EX_MOTION_DEVICE_H

#include "../extra/data/struct/array.h"

namespace ex::platform::dev {

    using control_id = uint;

    struct VideoControl {
        control_id id;
        char       name[64];
        int        min;
        int        max;
        int        step;
        int        default_value;
        int        value;
    };

    struct VideoDevice {
        char uuid[64];
        char name[64];
    };

    struct DeviceCapability {
        char description[64];
        char format_4cc[4];
        int  refresh_rate;
        int  width;
        int  height;
    };

    int get_capture_api();

    void refresh();

    ex::data::array<VideoDevice> list_devices();

    ex::data::array<VideoControl> list_controls(const char *device_uuid);

    ex::data::array<VideoControl> list_controls(const VideoDevice &device);

    ex::data::array<DeviceCapability> list_capabilities(const char *device_uuid);

    bool set_control_value(const char *device_uuid, control_id prop_id, int value);

    bool set_control_value(const VideoDevice &device, control_id prop_id, int value);

    int get_device_index(const char *device_uuid);

    int get_device_index(const VideoDevice &device);

    void reset_controls(const char *device_uuid);

    void reset_controls(const VideoDevice &device);
}

#endif // EX_MOTION_DEVICE_H
