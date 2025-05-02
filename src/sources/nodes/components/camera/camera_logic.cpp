//
// Created by xd on 4/30/25.
//

#include <pthread.h>
#include <opencv2/videoio.hpp>

#include "../../../../headers/nodes/components/camera/camera_logic.h"

#include "../../../../headers/cl/cl_kernel.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/logic/logic_nodes.h"

#include "../../../../../platforms/linux/v4l2_device.h"
#include "../../../../../extra/data/struct/spsc_ring.h"

namespace ex::logic::node::camera {

    struct capture_t {
        cv::Mat          frame;
        cv::VideoCapture camera;
    };

    namespace {
        const auto log = logs::get_logger("node_camera");

        int fourCC(const char *name) {
            return cv::VideoWriter::fourcc(
                    name[0],
                    name[1],
                    name[2],
                    name[3]);
        }

        struct message_from {
            id_type_32 node_id;
            bool       empty;
            bool       error;
        };

        struct message_to_s {
            ocl::ImagePromise promise_dst;
            cv::VideoCapture *cap_ptr;
            cv::Mat          *mat_ptr;
            id_type_32        node_id;
        };

        struct message_to_m {
            // TODO
        };

        // host --> worker
        ex::data::spsc_ring<message_to_s> messages_to_s;
        ex::data::spsc_ring<message_to_m> messages_to_m;

        // host <-- worker
        ex::data::spsc_ring<message_from> messages_from;

        ex::data::array<message_from> unloaded_messages;

        [[noreturn]] void *worker_routine(void *) {
            message_from response_;
            message_to_s message_s;
            message_to_m message_m;

            for (int attempts = 0;;) {

                if (messages_to_s.pop(&message_s)) {

                    if (!message_s.cap_ptr || !message_s.cap_ptr->isOpened()) {
                        response_.empty = true;
                        response_.error = true;
                        goto send_response_solo;
                    }

                    if (!message_s.cap_ptr->grab()) {
                        response_.empty = true;
                        response_.error = false;
                        goto send_response_solo;
                    }

                    if (!message_s.cap_ptr->retrieve(*message_s.mat_ptr)) {
                        response_.empty = true;
                        response_.error = false;
                        goto send_response_solo;
                    }

                    ex::ocl::ops::from_bytes(message_s.mat_ptr->data, message_s.promise_dst);
                    response_.empty = false;
                    response_.error = false;

                send_response_solo:
                    response_.node_id = message_s.node_id;
                    messages_from.push(&response_);
                    attempts = 0;
                }

                if (messages_to_m.pop(&message_m)) {
                    // TODO many (synchronized grab)
                    attempts = 0;
                    continue;
                }

                if (attempts++ > 1000)
                    ::usleep(1000);
            }
        }

        void start_worker() {
            if (static bool started = false; !started) {
                ::pthread_t thread;
                ::pthread_create(&thread, nullptr, &worker_routine, nullptr);
                ::pthread_detach(thread);
                started = true;
            }
        }

    }

    const ex::data::array<v_device_t> &list_camera_devices(const bool refresh) {
        static ex::data::array<v_device_t> list = ex::platform::dev::list_devices();
        if (refresh) {
            list = ex::platform::dev::list_devices();
        }
        return list;
    }

    const ex::data::array<v_control_t> &CameraNode::list_controls(const bool refresh) {
        if (refresh) {
            if (!state.device.selected) {
                controls.clean();
                return controls;
            }
            if (!state.mode.selected) {
                controls.clean();
                return controls;
            }
            controls = ex::platform::dev::list_controls(state.device.device);
        }
        return controls;
    }

    const ex::data::array<v_devcap_t> &CameraNode::list_capabilities() const {
        return capabilities;
    }

    limbo::graph::BaseNode *CameraNode::instance(TreeNode *from) {
        CameraNode *ptr = new CameraNode();
        ptr->set_id(from->id_node);
        ptr->set_image_id(from->id_node);
        return ptr;
    }

    limbo::graph::BaseNode *CameraNode::relocate(TreeNode *, limbo::graph::BaseNode *node) {
        return node;
    }

    void CameraNode::destroy(TreeNode *, limbo::graph::BaseNode *node) {
        node->detach_all();
        delete node;
    }

    void CameraNode::close() {
        if (capture->camera.isOpened())
            capture->camera.release();
        ::memset(&state.device, 0, sizeof(device_t));
        ::memset(&state.mode, 0, sizeof(mode_t));
        capabilities.clean();
        controls.clean();
    }

    void CameraNode::set_image_id(const id_type_16 id) {
        promises_[0].image.id = static_cast<id_type_16>(id);
        promises_[1].image.id = static_cast<id_type_16>(id);
        state.image_id        = static_cast<id_type_16>(id);
    }

    void CameraNode::set_device_buff(const int buffer_size) {
        this->buffer_size = buffer_size;
    }

    void CameraNode::set_device(const v_device_t &device) {
        close();

        if (ex::platform::dev::get_device_index(device.uuid) != -1) {
            state.device.device   = device;
            state.device.selected = true;
            capabilities          = ex::platform::dev::list_capabilities(device);
        }
    }

    void CameraNode::set_device_mode(const v_devcap_t &cap) {
        if (capture->camera.isOpened())
            capture->camera.release();

        ::memset(&state.mode, 0, sizeof(mode_t));
        controls.clean();

        state.mode.mode     = cap;
        state.mode.selected = true;
        snprintf(state.mode.description, 95, "%s (%s) - %d fps [%d x %d]", cap.description, cap.format_4cc,
                 cap.refresh_rate, cap.width, cap.height);
        state.mode.description[95] = '\0';

        controls = ex::platform::dev::list_controls(state.device.device);


        {
            std::vector<int> params;
            if (::strlen(state.mode.mode.format_4cc) >= 4) {
                params.push_back(cv::CAP_PROP_FOURCC);
                params.push_back(fourCC(state.mode.mode.format_4cc));
            }

            if (state.mode.mode.width > 0 && state.mode.mode.height > 0) {
                params.push_back(cv::CAP_PROP_FRAME_WIDTH);
                params.push_back(state.mode.mode.width);

                params.push_back(cv::CAP_PROP_FRAME_HEIGHT);
                params.push_back(state.mode.mode.height);
            }

            if (state.mode.mode.refresh_rate > 0) {
                params.push_back(cv::CAP_PROP_FPS);
                params.push_back(state.mode.mode.refresh_rate);
            }

            if (buffer_size >= 0) {
                params.push_back(cv::CAP_PROP_BUFFERSIZE);
                params.push_back(buffer_size);
            }

            if (params.size() > 0) {
                capture->camera.open(platform::dev::get_device_index(state.device.device),
                platform::dev::get_capture_api(),
                params);
            }

            else {
                capture->camera.open(platform::dev::get_device_index(state.device.device),
                    platform::dev::get_capture_api());
            }

            const bool is_opened = capture->camera.isOpened();
            log->info("Camera device: [{}] open: {}", state.device.device.uuid, is_opened);
        }


        const ex::State &state_ = ex::logic::globals::state();
        if (queue_ == nullptr) {
            queue_ = xm::ocl::create_queue_device(state_.cl.context, state_.cl.device, true, false);
        }

        b_index_ = 0;
        promises_[0].image.release();
        promises_[1].image.release();
        promises_[0].queue = queue_;
        promises_[1].queue = queue_;
        promises_[0].image = ex::ocl::Image2D::allocate(cap.width, cap.height, 3, 1, state_.cl.context);
        promises_[1].image = ex::ocl::Image2D::allocate(cap.width, cap.height, 3, 1, state_.cl.context);
        promises_[0].image.id = state.image_id;
        promises_[1].image.id = state.image_id;
    }

    void CameraNode::set_control(const unsigned int id, const int value) {
        if (!state.device.selected)
            return;
        if (!state.mode.selected)
            return;

        ex::platform::dev::set_control_value(state.device.device, id, value);
        log->info("set camera settings: {}, {}", id, value);
    }

    void CameraNode::reset_controls() {
        if (!state.device.selected)
            return;
        if (!state.mode.selected)
            return;

        ex::platform::dev::reset_controls(state.device.device);
        log->info("reset camera settings");
    }

    io::byte_array CameraNode::serialize() const {
        // TODO
        return io::byte_array::from(nullptr, 0);
    }

    void CameraNode::deserialize(const io::byte_array &) {
        // TODO
    }

    void CameraNode::on_input(const int in_idx, const int out_idx, const int clock_i, const void *data) {
        // TODO
    }

    void CameraNode::on_update(const int clock_i) {
        // TODO
    }

    void *CameraNode::on_output(const int in_idx, const int out_idx, const int clock_i) {
        if (!state.device.selected)
            return nullptr;
        if (!state.mode.selected)
            return nullptr;
        return &promises_[b_index_];
    }

    CameraNode::~CameraNode() {
        promises_[0].image.release();
        promises_[1].image.release();
        if (queue_ != nullptr) {
            clReleaseCommandQueue(queue_);
            queue_ = nullptr;
        }
        if (capture != nullptr) {
            capture->camera.release();
            delete capture;
            capture = nullptr;
        }
    }

    CameraNode::CameraNode() :
        capture(new capture_t()),
        state({}),
        b_index_(0),
        queue_(nullptr),
        buffer_size(1)
    {
        ::memset(&state, 0, sizeof(camera_state));
        start_worker();
    }

}
