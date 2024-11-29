//
// Created by xd on 29/11/24.
//

#include "../device.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

namespace ex::platform::dev {

    struct CameraDevice {
        std::string device_id;
        std::string index;
        std::string uuid;
        std::string name;
    };

    std::string read_file(const std::string &filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return "";
        }
        std::string content;
        std::getline(file, content);
        return content;
    }

    std::vector<CameraDevice> list_usb_camera_devices() {
        std::vector<CameraDevice> cameras;

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

            CameraDevice device;
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

    void list() {
        std::vector<CameraDevice> cameras = list_usb_camera_devices();

        if (cameras.empty()) {
            std::cout << "No camera devices found." << std::endl;
        } else {
            std::cout << "Camera devices found:" << std::endl;
            for (const auto& camera : cameras) {
                std::cout << "DEVICE: " << camera.device_id << ", UUID: " << camera.uuid << ", NAME: " << camera.name << ", IDX: " << camera.index << std::endl;
            }
        }
    }

}
