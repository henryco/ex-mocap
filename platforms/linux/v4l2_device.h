//
// Created by xd on 30/11/24.
//

#ifndef EX_MOTION_V4L2_DEVICE_H
#define EX_MOTION_V4L2_DEVICE_H

#include <linux/videodev2.h>
#include <string>
#include <vector>

namespace ex::platform::dev::v4l2 {

    struct v4l2_queryctrl_ext : v4l2_queryctrl {
        __s32 value;
    };

    typedef v4l2_queryctrl_ext V4L2_QueryCtrl;
    typedef v4l2_control V4L2_Control;

    struct V4L2Device {
        std::string device_id;
        std::string index;
        std::string uuid;
        std::string name;
    };

    std::vector<V4L2Device> list_usb_camera_devices();

    std::string get_device_id(const std::string &device_uuid);

    /**
     * @brief Retrieves camera properties for a given camera ID.
     *
     * This function retrieves the video4linux2 (v4l2) camera properties for a specified camera ID.
     *
     * @param device_id The ID of the camera.
     * @return The camera properties in a structure.
     *
     * @note This function requires the v4l2 library to be installed.
     * In general, it is already installed in most of linux distributions.
     */
    std::vector<V4L2_QueryCtrl> get_camera_props(const std::string &device_id);

    /**
     * @brief Sets the camera property for a given device.
     *
     * This function allows setting the camera property identified by `prop_id` to the specified `prop_value`
     * for the camera device identified by `device_id`.
     *
     * @param device_id   The ID of the camera device.
     * @param prop_id     The ID of the camera property to set.
     * @param prop_value  The value to set for the camera property.
     *
     * @return True if the camera property was set successfully, false otherwise.
     */
    bool set_camera_prop(const std::string &device_id, uint prop_id, int prop_value);

    /**
     * @brief Sets the camera property for a given device using v4l2 library.
     *
     * @param device_id The ID of the camera device.
     * @param control The v4l2 control to be set.
     *
     * @return True if the camera property was successfully set, false otherwise.
     */
    bool set_camera_prop(const std::string &device_id, V4L2_Control control);

    /**
     * @brief Sets the camera properties for the specified device.
     *
     * This function allows you to configure multiple camera properties at once
     * by providing a list of `V4L2_Control` objects.
     *
     * @param device_id The ID of the camera device.
     * @param controls A vector of `V4L2_Control` objects representing the camera properties to be set.
     * @return Vector of booleans representing results of setting each of properties.
     *
     * @see V4L2_Control
     */
    std::vector<bool> set_camera_prop(const std::string &device_id, std::vector<V4L2_Control> controls);

}

#endif // EX_MOTION_V4L2_DEVICE_H
