//
// Created by xd on 29/11/24.
//

#include "v4l2_device.h"
#include "../device.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include <sys/ioctl.h>
#include <algorithm>
#include <unistd.h>
#include <fcntl.h>

namespace ex::platform::dev {

    namespace v4l2 {
        static std::vector<v4l2::V4L2Device> devices_list;
        static bool initialized = false;

        std::string read_file(const std::string &filePath) {
            std::ifstream file(filePath);
            if (!file.is_open()) {
                return "";
            }
            std::string content;
            std::getline(file, content);
            return content;
        }

        std::vector<v4l2::V4L2Device> list_usb_camera_devices() {
            std::vector<v4l2::V4L2Device> cameras;

            // Iterate over all /dev/video* devices
            for (const auto& entry : std::filesystem::directory_iterator("/dev")) {
                const std::string device_path = entry.path().string();
                if (device_path.find("/dev/video") == std::string::npos) {
                    continue; // Skip non-video devices
                }

                // Find corresponding sysfs entry for the video device
                const std::string device_name   = entry.path().filename();
                const std::string sys_device_path = "/sys/class/video4linux/" + device_name;

                if (!std::filesystem::exists(sys_device_path)) {
                    continue;
                }

                const std::filesystem::path sys_path       = sys_device_path;
                const std::filesystem::path dev_path       = sys_path / "device";
                const std::filesystem::path current_path   = sys_device_path;
                const std::filesystem::path name_path      = current_path / "name";
                const std::filesystem::path interface_path = current_path / "interface";
                const std::filesystem::path index_path     = current_path / "index";

                v4l2::V4L2Device device;
                device.device_id = "/dev/" + device_name;

                // Follow symbolic link to actual device path
                if (std::filesystem::exists(dev_path)) {
                    device.uuid = std::filesystem::canonical(dev_path).string();
                }

                // Resolve name
                if (std::filesystem::exists(name_path)) {
                    device.name = read_file(name_path);
                } else if (std::filesystem::exists(interface_path)) {
                    device.name = read_file(interface_path);
                } else {
                    device.name = device.device_id;
                }

                // Resolve index
                if (std::filesystem::exists(index_path)) {
                    device.index = read_file(index_path);
                }

                cameras.push_back(device);
            }

            return cameras;
        }

        std::vector<V4L2_QueryCtrl> get_camera_props(const std::string &device) {
            std::vector<V4L2_QueryCtrl> properties;

            const int file_descriptor = open(device.c_str(), O_RDWR);

            if (file_descriptor == -1) {
                std::cerr << "Cannot open video device: " << device << std::endl;
                return {};
            }

            try {

                V4L2_QueryCtrl queryctrl;
                queryctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;

                while (0 == ioctl(file_descriptor, VIDIOC_QUERYCTRL, &queryctrl)) {
                    if (!(queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)) {

                        if (queryctrl.type != V4L2_CTRL_TYPE_CTRL_CLASS) {
                            V4L2_Control ctr = {.id = queryctrl.id};
                            if (ioctl(file_descriptor, VIDIOC_G_CTRL, &ctr) == -1) {
                                std::cerr << "Cannot retrieve value for control: "
                                          << ctr.id << " [" << device << "]"
                                          << std::endl;
                            }
                            queryctrl.value = ctr.value;
                        }

                        // TODO: query menu items name

                        properties.push_back(queryctrl);
                    }

                    queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
                }

                if (errno != EINVAL) {
                    std::cerr << "fails for reasons other than reaching the end of the control list, error no: [" << errno << "]" << std::endl;
                    close(file_descriptor);
                    return {};
                }

            } catch (...) {
                close(file_descriptor);
                throw;
            }

            close(file_descriptor);

            // Sorting, important!
            std::sort(properties.begin(), properties.end(),
                      [](const V4L2_QueryCtrl &a, const V4L2_QueryCtrl &b) {
                          return a.type > b.type;
                      });

            return properties;
        }

        bool set_camera_prop(const std::string &device_id, uint prop_id, int prop_value) {
            // not the most efficient way to pass data, but it's ok
            return set_camera_prop(device_id, {
                                                      .id = prop_id,
                                                      .value = prop_value
                                              });
        }

        bool set_camera_prop(const std::string &device_id, V4L2_Control control) {
            // not the most efficient way to pass data, but it's ok.
            return set_camera_prop(
                    device_id,
                    std::vector<V4L2_Control>({control})
                            )[0];
        }

        std::vector<bool> set_camera_prop(const std::string &device, std::vector<V4L2_Control> controls) {
            const int file_descriptor = open(device.c_str(), O_RDWR);

            std::vector<bool> results(controls.size());

            if (file_descriptor == -1) {
                std::cerr << "Cannot open video device: " << device << '\n';
                return results;
            }

            try {

                for (int i = 0; i < controls.size(); ++i) {
                    auto &control = controls[i];
                    if (ioctl(file_descriptor, VIDIOC_S_CTRL, &control) == -1) {
                        std::cerr << "[" << device << "] Cannot set control value: " << control.id << '\n';
                        results[i] = false;
                    } else {
                        results[i] = true;
                    }
                }

            } catch (...) {
                close(file_descriptor);
                throw;
            }

            close(file_descriptor);
            return results;
        }
    }

    int get_capture_api() {
        return 200; // V4L2
    }

    std::vector<VideoDevice> list_devices() {
        if (!v4l2::initialized)
            refresh();

        std::vector<VideoDevice> devices;
        devices.reserve(v4l2::devices_list.size());
        for (const auto &item: v4l2::devices_list) {
            devices.emplace_back(item.index + ":" + item.uuid, item.name);
        }
        return devices;
    }

    void refresh() {
        v4l2::devices_list = v4l2::list_usb_camera_devices();
    }

    int get_device_index(const VideoDevice &device) {
        return get_device_index(device.uuid);
    }

    int get_device_index(const std::string &id) {
        if (id.starts_with("/dev/video")) {
            return std::stoi(id.substr(10));
        }

        const auto split_idx = id.find(':');
        if (split_idx == std::string::npos) {
            return -1;
        }

        const auto index = id.substr(0, split_idx);
        const auto uuid  = id.substr(split_idx + 1);

        for (const auto &device: v4l2::devices_list) {
            if (device.uuid == uuid && device.index == index) {
                return std::stoi(device.device_id.substr(10));
            }
        }

        return -1;
    }

    std::vector<VideoControl> list_controls(const VideoDevice &device) {
        return list_controls(device.uuid);
    }

    std::vector<VideoControl> list_controls(const std::string &device_uuid) {
        // TODO
        return {};
    }

    bool set_control_value(const VideoDevice &device, uint prop_id, int value) {
        return set_control_value(device.uuid, prop_id, value);
    }

    bool set_control_value(const std::string &device_uuid, uint prop_id, int value) {
        // TODO
        return false;
    }

    void reset_controls(const VideoDevice &device) {
        reset_controls(device.uuid);
    }

    void reset_controls(const std::string &device_uuid) {
        const auto controls = list_controls(device_uuid);
        for (const auto &control: controls)
            set_control_value(device_uuid, control.id, control.default_value);
    }
}
