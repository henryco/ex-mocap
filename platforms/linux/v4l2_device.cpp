//
// Created by xd on 29/11/24.
//

#include "v4l2_device.h"
#include "../device.h"

#include <filesystem>
#include <fstream>
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdarg.h>

namespace ex::platform::dev {

    static ulong fnv1aHash(const char *str, int size) {
        constexpr ulong FNV_OFFSET_BASIS = 14695981039346656037ull;
        constexpr ulong FNV_PRIME        = 1099511628211ull;

        ulong hash = FNV_OFFSET_BASIS;
        while (size > 0) {
            hash ^= static_cast<ulong>(*str);
            hash *= FNV_PRIME;
            ++str;
            --size;
        }
        return hash;
    }

    static void abort__(const char* format, ...) noexcept(true) {
        const char* prefix = "Error: ";
        char buffer[256];

        size_t prefix_len = strlen(prefix);
        memcpy(buffer, prefix, prefix_len);

        va_list args;
        va_start(args, format);
        vsnprintf(buffer + prefix_len, sizeof(buffer) - prefix_len, format, args);
        va_end(args);

        fprintf(stderr, "%s\n", buffer);
        ::abort();
    }

    namespace v4l2 {

        static int8_t default_comparator(const V4L2_QueryCtrl *aa, const V4L2_QueryCtrl *bb) {
            const unsigned int a = aa->type;
            const unsigned int b = bb->type;
            return (a > b) - (a < b);
        }

        static ex::data::array<v4l2::V4L2Device> devices_list;
        static bool initialized = false;

        static void copy_string_(const char *src, char dst[], const int dst_size) {
            if (src == nullptr)
                return;
            ::strncpy(dst, src, dst_size - 1);
            dst[dst_size - 1] = '\0';
        }


        static void read_file(const std::string &filePath, char dst[], const int dst_size) {
            std::ifstream file(filePath);
            if (!file.is_open()) {
                copy_string_("", dst, dst_size);
                return;
            }
            std::string content;
            std::getline(file, content);
            copy_string_(content.c_str(), dst, dst_size);
        }

        static void fourcc_to_chars(const uint32_t fourcc, char *out) {
            out[0] = fourcc & 0xFF;
            out[1] = (fourcc >> 8) & 0xFF;
            out[2] = (fourcc >> 16) & 0xFF;
            out[3] = (fourcc >> 24) & 0xFF;
            out[4] = '\0';
        }

        ex::data::array<v4l2::V4L2Device> list_usb_camera_devices() {
            ex::data::array<v4l2::V4L2Device> cameras;

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
                {
                    const std::string tmp_ = "/dev/" + device_name;
                    copy_string_(tmp_.c_str(), device.device_id, 64);
                }

                // Follow symbolic link to actual device path
                if (std::filesystem::exists(dev_path)) {
                    const std::string tmp_ = std::filesystem::canonical(dev_path).string();
                    copy_string_(tmp_.c_str(), device.uuid, 128);
                }

                // Resolve name
                if (std::filesystem::exists(name_path)) {
                    read_file(name_path, device.name, 64);
                } else if (std::filesystem::exists(interface_path)) {
                    read_file(interface_path, device.name, 64);
                } else {
                    read_file(device.device_id, device.name, 64);
                }

                // Resolve index
                if (std::filesystem::exists(index_path)) {
                    read_file(index_path, device.index, 64);
                }

                cameras.push(device);
            }

            return cameras;
        }

        ex::data::array<V4L2_QueryCtrl> get_camera_props(const char *device) {
            ex::data::array<V4L2_QueryCtrl> properties;

            const int file_descriptor = open(device, O_RDWR);

            if (file_descriptor == -1) {
                std::cerr << "Cannot open video device: " << device << std::endl;
                return {};
            }

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
                    properties.sort_insert(queryctrl, default_comparator);
                }

                queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
            }

            if (errno != EINVAL) {
                std::cerr << "fails for reasons other than reaching the end of the control list, error no: [" << errno << "]" << std::endl;
                close(file_descriptor);
                return {};
            }

            close(file_descriptor);

            // Sorting, important!
//            std::sort(properties.begin(), properties.end(),
//                      [](const V4L2_QueryCtrl &a, const V4L2_QueryCtrl &b) {
//                          return a.type > b.type;
//                      });

            return properties;
        }

        bool set_camera_prop(const char *device_id, const uint prop_id, const int prop_value) {
            // not the most efficient way to pass data, but it's ok
            return set_camera_prop(device_id, {
                                                      .id = prop_id,
                                                      .value = prop_value
                                              });
        }

        bool set_camera_prop(const char *device_id, V4L2_Control control) {
            // not the most efficient way to pass data, but it's ok.

            const int file_descriptor = open(device_id, O_RDWR);
            if (file_descriptor == -1) {
                std::cerr << "Cannot open video device: " << device_id << '\n';
                return false;
            }

            bool result = false;
            if (ioctl(file_descriptor, VIDIOC_S_CTRL, &control) == -1) {
                std::cerr << "[" << device_id << "] Cannot set control value: " << control.id << '\n';
            } else {
                result = true;
            }

            close(file_descriptor);
            return result;
        }

        ex::data::array<bool> set_camera_prop(const char *device, const ex::data::array<V4L2_Control> &controls) {
            const int file_descriptor = open(device, O_RDWR);

            ex::data::array<bool> results(controls.size);

            if (file_descriptor == -1) {
                std::cerr << "Cannot open video device: " << device << '\n';
                return results;
            }

            for (int i = 0; i < controls.size; ++i) {
                auto &control = controls[i];
                if (ioctl(file_descriptor, VIDIOC_S_CTRL, &control) == -1) {
                    std::cerr << "[" << device << "] Cannot set control value: " << control.id << '\n';
                    results[i] = false;
                } else {
                    results[i] = true;
                }
            }

            close(file_descriptor);
            return results;
        }

        const char *get_device_id(const char *device_uuid_) {
            const std::string device_uuid = device_uuid_;
            if (device_uuid.starts_with("/dev/video")) {
                return device_uuid_;
            }

            const auto split_idx = device_uuid.find(':');
            if (split_idx == std::string::npos) {
                abort__("invalid device uuid: %s", device_uuid.c_str());
                return nullptr;
            }

            const auto index = device_uuid.substr(0, split_idx);
            const auto uuid  = device_uuid.substr(split_idx + 1);

            for (v4l2::V4L2Device &device: v4l2::devices_list) {
                if (::strcmp(device.uuid, uuid.c_str()) == 0 &&
                    ::strcmp(device.index, index.c_str()) == 0) {
                    return device.device_id;
                }
            }

            return nullptr;
        }
    }

    ex::data::array<DeviceCapability> list_capabilities(const VideoDevice &device) {
        return list_capabilities(device.uuid);
    }

    ex::data::array<DeviceCapability> list_capabilities(const char *device_uuid) {
        const int file_descriptor = open(v4l2::get_device_id(device_uuid), O_RDWR);
        if (file_descriptor == -1) {
            perror("Opening video device");
            return {};
        }

        // Query device capabilities
        v4l2_capability cap;
        if (ioctl(file_descriptor, VIDIOC_QUERYCAP, &cap) == -1) {
            perror("Querying capabilities");
            close(file_descriptor);
            return {};
        }

        ex::data::array<DeviceCapability> vec;

        v4l2_fmtdesc fmt;
        ::memset(&fmt, 0, sizeof(fmt));
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        while (ioctl(file_descriptor, VIDIOC_ENUM_FMT, &fmt) == 0) {
            char format[5];
            v4l2::fourcc_to_chars(fmt.pixelformat, format);

            // Enumerate resolutions for each format
            v4l2_frmsizeenum frmsize;
            ::memset(&frmsize, 0, sizeof(frmsize));
            frmsize.pixel_format = fmt.pixelformat;

            while (ioctl(file_descriptor, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
                if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {

                    // Enumerate frame intervals for each resolution
                    v4l2_frmivalenum frmival;
                    ::memset(&frmival, 0, sizeof(frmival));
                    frmival.pixel_format = fmt.pixelformat;
                    frmival.width        = frmsize.discrete.width;
                    frmival.height       = frmsize.discrete.height;

                    while (ioctl(file_descriptor, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) == 0) {
                        if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
                            DeviceCapability capability = {};

                            ::memcpy(capability.format_4cc, format, 4);
                            v4l2::copy_string_(reinterpret_cast<const char *>(fmt.description), capability.description, 32);

                            capability.width  = frmsize.discrete.width;
                            capability.height = frmsize.discrete.height;
                            capability.refresh_rate =
                                    static_cast<double>(frmival.discrete.denominator) / frmival.discrete.numerator;
                            capability.uuid = 0;
                            capability.uuid = fnv1aHash(reinterpret_cast<const char *>(&capability), sizeof(DeviceCapability));
                            vec.push(capability);
                        }
                        frmival.index++;
                    }
                }
                frmsize.index++;
            }
            fmt.index++;
        }

        close(file_descriptor);
        return vec;
    }

    int get_capture_api() {
        return 200; // V4L2
    }

    ex::data::array<VideoDevice> list_devices() {
        if (!v4l2::initialized)
            refresh();

        ex::data::array<VideoDevice> devices;
        devices.reserve(v4l2::devices_list.size);
        for (const v4l2::V4L2Device &item: v4l2::devices_list) {
            VideoDevice dev;
            std::string tmp_ = std::string(item.index) + ":" + std::string(item.uuid);
            v4l2::copy_string_(tmp_.c_str(), dev.uuid, 128);
            v4l2::copy_string_(item.name, dev.name, 64);
            devices.push(dev);
        }
        return devices;
    }

    void refresh() {
        v4l2::devices_list = v4l2::list_usb_camera_devices();
    }

    int get_device_index(const VideoDevice &device) {
        return get_device_index(device.uuid);
    }

    int get_device_index(const char *id_) {
        const std::string id = id_;
        if (id.starts_with("/dev/video")) {
            return std::stoi(id.substr(10));
        }

        const auto split_idx = id.find(':');
        if (split_idx == std::string::npos) {
            return -1;
        }

        const auto index = id.substr(0, split_idx);
        const auto uuid  = id.substr(split_idx + 1);

        for (const v4l2::V4L2Device &device: v4l2::devices_list) {
            if (::strcmp(device.uuid, uuid.c_str()) == 0 &&
                ::strcmp(device.index, index.c_str()) == 0) {
                const std::string tmp_ = device.device_id;
                return std::stoi(tmp_.substr(10));
            }
        }

        return -1;
    }

    ex::data::array<VideoControl> list_controls(const VideoDevice &device) {
        return list_controls(device.uuid);
    }

    ex::data::array<VideoControl> list_controls(const char *device_uuid) {
        const auto device_id = v4l2::get_device_id(device_uuid);
        const auto camera_props = v4l2::get_camera_props(device_id);

        ex::data::array<VideoControl> props;
        props.reserve(camera_props.size);
        for (const v4l2::V4L2_QueryCtrl &p: camera_props) {
            VideoControl control;
            control.id            = p.id;
            control.default_value = p.default_value;
            v4l2::copy_string_(reinterpret_cast<const char *>(p.name), control.name, 32);
            control.value = p.value;
            control.min   = p.minimum;
            control.max   = p.maximum;
            control.step  = p.step;
            props.push(control);
        }
        return props;
    }

    bool set_control_value(const VideoDevice &device, const uint prop_id, const int value) {
        return set_control_value(device.uuid, prop_id, value);
    }

    bool set_control_value(const char *device_uuid, const uint prop_id, const int value) {
        const auto device_id = v4l2::get_device_id(device_uuid);
        return v4l2::set_camera_prop(device_id, prop_id, value);
    }

    void reset_controls(const VideoDevice &device) {
        reset_controls(device.uuid);
    }

    void reset_controls(const char *device_uuid) {
        const auto controls = list_controls(device_uuid);
        for (const VideoControl &control: controls)
            set_control_value(device_uuid, control.id, control.default_value);
    }
}
