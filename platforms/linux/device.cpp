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
        std::string device_path;  // e.g., /dev/video0
        std::string usb_bus_info; // e.g., Bus 001 Device 002
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
            std::string device_path = entry.path().string();
            if (device_path.find("/dev/video") == std::string::npos) {
                continue; // Skip non-video devices
            }

            // Find corresponding sysfs entry for the video device
            std::string device_name   = entry.path().filename();
            std::string sys_device_path = "/sys/class/video4linux/" + device_name;
            if (!std::filesystem::exists(sys_device_path)) {
                continue;
            }

            // Follow symbolic link to actual device path
            std::string real_device_path = std::filesystem::canonical(sys_device_path).string();

            // Traverse upward to find USB attributes
            std::filesystem::path current_path = real_device_path;
            while (current_path.has_parent_path()) {
                current_path = current_path.parent_path();

                // Check for USB device attributes
                if (std::filesystem::exists(current_path / "busnum") &&
                    std::filesystem::exists(current_path / "devnum")) {

                    // Read bus and device numbers
                    std::string bus_num = read_file((current_path / "busnum").string());
                    std::string dev_num = read_file((current_path / "devnum").string());

                    if (!bus_num.empty() && !dev_num.empty()) {
                        CameraDevice cam;
                        cam.device_path = device_path;
                        cam.usb_bus_info = "Bus " + bus_num + " Device " + dev_num;
                        cameras.push_back(cam);
                    }
                    break; // Found USB parent device
                }
            }
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
                std::cout << "  Path: " << camera.device_path << ", USB: " << camera.usb_bus_info << std::endl;
            }
        }
    }

}
