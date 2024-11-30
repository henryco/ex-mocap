//
// Created by xd on 29/11/24.
//

#include "v4l2_device.h"
#include "../device.h"

#include <filesystem>
#include <fstream>
#include <vector>

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

    void set_control_value(const VideoDevice &device, uint prop_id, int value) {
        set_control_value(device.uuid, prop_id, value);
    }

    void set_control_value(const std::string &device_uuid, uint prop_id, int value) {
        // TODO
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
