//
// Created by xd on 15/12/24.
//

#include "../../headers/core/camera.h"
#include "../../headers/cl/cl_kernel.h"

namespace ex::camera {

    namespace {
        const auto log = logs::get_logger("camera");
    }

    int fourCC(const char *name) {
        return cv::VideoWriter::fourcc(
                name[0],
                name[1],
                name[2],
                name[3]);
    }

    void Camera::open(const char *device_uuid) {
        close();
        queue = xm::ocl::create_queue_device(ocl_context, ocl_device, true, false);
        cap_.open(platform::dev::get_device_index(device_uuid),
            platform::dev::get_device_index(device_uuid));
        device_id = device_uuid;
        cap_called_ = false;
        grab_called = false;
        initialized = true;
    }

    void Camera::open(const char *device_uuid, const config &prop) {
        close();

        log->debug("Open video capture");

        queue = xm::ocl::create_queue_device(ocl_context, ocl_device, true, false);

        std::vector<int> params;
        if (::strlen(prop.codec) >= 4) {
            params.push_back(cv::CAP_PROP_FOURCC);
            params.push_back(fourCC(prop.codec));
        }

        if (prop.width > 0 && prop.height > 0) {
            params.push_back(cv::CAP_PROP_FRAME_WIDTH);
            params.push_back(prop.width);

            params.push_back(cv::CAP_PROP_FRAME_HEIGHT);
            params.push_back(prop.height);
        }

        if (prop.fps > 0) {
            params.push_back(cv::CAP_PROP_FPS);
            params.push_back(prop.fps);
        }

        if (prop.buffer >= 0) {
            params.push_back(cv::CAP_PROP_BUFFERSIZE);
            params.push_back(prop.buffer);
        }

        if (params.size() > 0) {
            cap_.open(platform::dev::get_device_index(device_uuid),
            platform::dev::get_device_index(device_uuid),
            params);
        }

        else {
            cap_.open(platform::dev::get_device_index(device_uuid),
                platform::dev::get_device_index(device_uuid));
        }

        config_ = prop;
        device_id = device_uuid;
        cap_called_ = false;
        grab_called = false;
        initialized = true;
    }

    xm::ocl::iop::ClImagePromise Camera::capture(const bool force_grab) {
        if (!initialized)
            return {};

        if (force_grab && !grab_called)
            return {};

        cv::Mat frame;
        cap_.retrieve(frame);
        grab_called = false;
        return xm::ocl::iop::from_cv_mat(frame, queue);
    }

    bool Camera::grab() {
        grab_called = true;
        return cap_.grab();
    }

    Camera::Camera(const cl_context context, cl_device_id device) :
        ocl_context(context), ocl_device(device), device_id(nullptr) {
    }

    Camera::~Camera() {
        if (queue != nullptr)
            clReleaseCommandQueue(queue);
    }

    int  Camera::get_capture_api() const {
        return platform::dev::get_capture_api();
    }

    uint Camera::get_device_index() const {
        return platform::dev::get_device_index(device_id);
    }

    config const &Camera::get_config() const {
        return config_;
    }

    void Camera::set_control(const platform::dev::control_id id, const int value) {
        platform::dev::set_control_value(device_id, id, value);
    }

    ex::data::array<platform::dev::VideoControl> Camera::get_controls() const {
        return platform::dev::list_controls(device_id);
    }

    void Camera::reset_controls() {
        platform::dev::reset_controls(device_id);
    }

    ex::data::array<platform::dev::DeviceCapability> Camera::get_capabilities() {
        if (!cap_called_) {
            capabilities_ = platform::dev::list_capabilities(device_id);
            cap_called_ = true;
        }
        return capabilities_;
    }

    void Camera::close() {
        log->debug("Close video capture");
        cap_.release();
        cap_called_ = false;
        grab_called = false;
        initialized = false;
        if (queue != nullptr)
            clReleaseCommandQueue(queue);
        queue = nullptr;
    }

} // namespace ex::camera
