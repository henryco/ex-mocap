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
#include "../../../../../extra/worker/worker.h"

namespace ex::logic::node::camera {

    namespace lw = limbo::worker;

    struct result_t {
        bool  empty;
        bool  error;
    };

    // 320 bytes
    struct alignas(64) capture_t {
        cv::Mat            bitmap; //
        cv::VideoCapture   camera; // regularly touched by worker
        ocl::ImagePromise *target; //
        // -----------------------
        alignas(64)
        result_t result;           // touched by host and worker
        // -----------------------
        alignas(64)
        lw::channel_p channel;     // touched by host, never by worker
    };

    namespace {
        lw::worker_p worker = nullptr;
        const auto   log    = logs::get_logger("node_camera");

        int fourCC(const char *name) {
            return cv::VideoWriter::fourcc(
                    name[0],
                    name[1],
                    name[2],
                    name[3]);
        }

        void init_state(camera_state *state) {
            ::memset(state, 0, sizeof(camera_state));
            state->buffering = 2; // default - double buffering
        }

        void worker_routine(void *arg) {
//            const auto t0 = std::chrono::high_resolution_clock::now();
            capture_t *capture = static_cast<capture_t *>(arg);

            if (!capture->camera.grab()) {
                capture->result.error = false;
                capture->result.empty = true;
                return;
            }

            if (!capture->camera.retrieve(capture->bitmap)) {
                capture->result.error = false;
                capture->result.empty = true;
                return;
            }

//            const auto t1 = std::chrono::high_resolution_clock::now();
//            const auto dt = (t1 - t0).count();
//            std::cout << dt << '\n';

            ocl::ops::from_bytes(capture->bitmap.data, *capture->target);
            capture->result.error = false;
            capture->result.empty = false;
        }

        void start_worker() {
            if (worker != nullptr)
                return;
            worker = lw::create_worker(100, 1000000);
        }

        void stop_wait(CameraNode *that) {
            const lw::status_t status = lw::status(that->capture->channel);
            if ((status & lw::status_t::finished) == 0) {
                lw::stop(that->capture->channel);
                while ((lw::status(that->capture->channel) & lw::status_t::finished) == 0) {
                    ::usleep(100);
                }
            }
        }

        void enqueue(CameraNode *that) {
            that->capture->result.error = false;
            that->capture->result.empty = true;
            that->capture->target = &(that->promises_[(that->b_index_ + 1) % 2]);
            lw::enqueue(that->capture->channel, &worker_routine, that->capture);
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
        enabled = false;
        stop_wait(this);

        if (!capture)
            return;

        if (capture->camera.isOpened())
            capture->camera.release();

        capture->bitmap.release();
        capture->result.error = false;
        capture->result.empty = true;
    }

    void CameraNode::open() {
        close();

        if (!capture)
            return;
        if (!state.device.selected)
            return;
        if (!state.mode.selected)
            return;

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

        if (state.buffering > 0) {
            params.push_back(cv::CAP_PROP_BUFFERSIZE);
            params.push_back(state.buffering);
        }

        capture->bitmap.release();

        if (params.size() > 0) {
            enabled = capture->camera.open(platform::dev::get_device_index(state.device.device),
            platform::dev::get_capture_api(), params);
        } else {
            enabled = capture->camera.open(platform::dev::get_device_index(state.device.device),
                platform::dev::get_capture_api());
        }

        if (enabled) {
            enqueue(this);
        }

        log->info("Camera device: [{}] open: {}", state.device.device.uuid, enabled);
    }

    void CameraNode::set_image_id(const id_type_16 id) {
        promises_[0].image.id = static_cast<id_type_16>(id);
        promises_[1].image.id = static_cast<id_type_16>(id);
        state.image_id        = static_cast<id_type_16>(id);
    }

    void CameraNode::set_device_buff(const short buffer_size) {
        close();
        this->state.buffering = buffer_size;
    }

    void CameraNode::set_device(const v_device_t &device) {
        close();

        ::memset(&state.device, 0, sizeof(device_t));
        ::memset(&state.mode, 0, sizeof(mode_t));
        capabilities.clean();
        controls.clean();

        if (ex::platform::dev::get_device_index(device.uuid) != -1) {
            state.device.device   = device;
            state.device.selected = true;
            capabilities          = ex::platform::dev::list_capabilities(device);
        }
    }

    void CameraNode::set_device_mode(const v_devcap_t &cap) {
        close();

        ::memset(&state.mode, 0, sizeof(mode_t));
        controls.clean();

        static char buff_form[5];
        ::memset(buff_form, '\0', 5);
        ::memcpy(buff_form, cap.format_4cc, 4);

        state.mode.mode     = cap;
        state.mode.selected = true;
        snprintf(state.mode.description, 95,
            "%s (%s) - %d fps [%d x %d]", cap.description, buff_form,
                 cap.refresh_rate, cap.width, cap.height);
        state.mode.description[95] = '\0';

        controls = ex::platform::dev::list_controls(state.device.device);

        const ex::State &state_ = ex::logic::globals::state();
        if (queue_ == nullptr) {
            queue_ = xm::ocl::create_queue_device(state_.cl.context, state_.cl.device, true, false);
        }

        b_index_ = 0;
        promises_[0].image.release();
        promises_[1].image.release();
        promises_[0].queue    = queue_;
        promises_[1].queue    = queue_;
        promises_[0].image    = ocl::Image2D::allocate(cap.width, cap.height, 3, 1, state_.cl.context);
        promises_[1].image    = ocl::Image2D::allocate(cap.width, cap.height, 3, 1, state_.cl.context);
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
        // TODO write to bytes
        return io::byte_array::from(nullptr, 0);
    }

    void CameraNode::deserialize(const io::byte_array &) {
        // TODO read from bytes
    }

    void CameraNode::on_input(const int in_idx, const int out_idx, const int clock_i, const void *data) {
        // TODO process synchronizer
    }

    void CameraNode::on_update(const int clock_i) {
        if (!enabled)
            return;

        const lw::status_t status = lw::status(capture->channel);

        if ((status & lw::status_t::success) != 0) {
            // TODO fps timing
            if (!capture->result.empty && !capture->result.error) {
                b_index_ = (b_index_ + 1) % 2; // swap buffers
                updates_.refresh();
            }
            enqueue(this);
        }

        promises_[b_index_].image.header.fresh = updates_.update();

        if ((status & lw::status_t::failure) != 0)
            abort_("capture worker error");
    }

    void *CameraNode::on_output(const int in_idx, const int out_idx, const int clock_i) {
        if (!enabled)
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

            if (capture->channel != nullptr)
                lw::close_channel(capture->channel);
            capture->channel = nullptr;

            delete capture;
            capture = nullptr;
        }
    }

    CameraNode::CameraNode() :
        capture(new capture_t()), // new bruh
        state({}),
        b_index_(0),
        queue_(nullptr),
        enabled(false)
    {
        init_state(&state);
        start_worker();
        capture->result.error = false;
        capture->result.empty = true;
        capture->channel = lw::create_channel(worker);
        if (capture->channel == nullptr)
            abort_("Cannot create worker channel (not enough slots?)");
    }

}
