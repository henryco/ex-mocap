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
    namespace lg = limbo::graph;

    enum action_t: unsigned char {
        action_none = 0,
        action_open = 1 << 0,
        action_read = 1 << 1,
        action_sync = 1 << 2,
    };

    // touched by host and worker,
    // but never ever at the same time (guarantee)
    struct alignas(64) result_t {
        action_t type;
        bool     ready;
        bool     error;
    };

    // 64 bytes touched once, dont care
    struct alignas(64) camera_t {
        std::vector<int>  params;
        cv::VideoCapture *camera;
        result_t         *result;
        const unsigned    index;
        const unsigned    api;
    };

    // 192 bytes regularly touched by worker
    struct alignas(64) capture_t {
        cv::Mat            bitmap;
        cv::VideoCapture   camera;
        ocl::ImagePromise *target;
        result_t          *result;
    };

    struct internal_t {
        ocl::img_promise promises[2];
        cl_command_queue queue_p;
        lw::channel_p    channel;
        unsigned int     b_index;
        lg::UpdateState  updates;
        bool             enabled;
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

        void init_internal(internal_t *state) {
            state->queue_p = nullptr;
            state->enabled = false;
            state->b_index = 0;

            state->channel = lw::create_channel(worker);
            if (state->channel == nullptr)
                abort_("Cannot create worker channel (not enough slots?)");
        }

        void init_capture(capture_t *) {
            // nop
        }

        void init_result(result_t *result) {
            result->type  = action_none;
            result->error = false;
            result->ready = false;
        }

        void open_routine(void *arg) {
            camera_t *camera = static_cast<camera_t *>(arg);

            bool success = false;
            if (camera->params.size() > 0)
                success = camera->camera->open(camera->index, camera->api, camera->params);
            else
                success = camera->camera->open(camera->index, camera->api);

            camera->result->type  = action_open;
            camera->result->ready = success;
            camera->result->error = !success;

            camera->result = nullptr;
            delete camera;
        }

        void worker_routine(void *arg) {
            capture_t *capture = static_cast<capture_t *>(arg);

            if (!capture->camera.grab()) {
                capture->result->error = false;
                capture->result->ready = false;
                return;
            }

            if (!capture->camera.retrieve(capture->bitmap)) {
                capture->result->error = false;
                capture->result->ready = false;
                return;
            }

            ocl::ops::from_bytes(capture->bitmap.data, *capture->target);
            capture->result->error = false;
            capture->result->ready = true;
        }

        void start_worker() {
            if (worker != nullptr)
                return;
            worker = lw::create_worker(100, 1000000);
        }

        void stop_wait(CameraNode *that) {
            const lw::status_t status = lw::status(that->internal->channel);
            if ((status & lw::status_t::finished) == 0) {
                lw::stop(that->internal->channel);
                while ((lw::status(that->internal->channel) & lw::status_t::finished) == 0) {
                    ::usleep(100);
                }
            }
        }

        void open_camera(CameraNode *that, const int api, const int index, const std::vector<int> &params) {
            that->result->type  = action_open;
            that->result->error = false;
            that->result->ready = false;

            camera_t *payload = new camera_t {
                params,
                &that->capture->camera,
                that->result,
                static_cast<unsigned>(index),
                static_cast<unsigned>(api)
            };

            lw::enqueue(that->internal->channel, &open_routine, payload);
        }

        void enqueue(CameraNode *that) {
            that->result->type  = action_read;
            that->result->error = false;
            that->result->ready = false;

            that->capture->result = that->result;
            that->capture->target = &(that->internal->promises[(that->internal->b_index + 1) % 2]);
            lw::enqueue(that->internal->channel, &worker_routine, that->capture);
        }

    }

    const ex::data::array<v_device_t> &list_camera_devices(const bool refresh) {
        static ex::data::array<v_device_t> list = ex::platform::dev::list_devices();
        if (refresh) {
            list.clean();
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
        internal->enabled = false;
        ::memset(&timings, 0, sizeof(timings_t));
        stop_wait(this);

        if (!capture)
            return;

        if (capture->camera.isOpened())
            capture->camera.release();

        capture->bitmap.release();

        result->type  = action_none;
        result->error = false;
        result->ready = false;
    }

    void CameraNode::reset() {
        list_camera_devices(true);

        capabilities.clean();
        capabilities = ex::platform::dev::list_capabilities(state.device.device);

        controls.clean();
        controls = ex::platform::dev::list_controls(state.device.device);
        open();
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

        const int index = platform::dev::get_device_index(state.device.device);
        const int api   = platform::dev::get_capture_api();
        open_camera(this, api, index, params);
    }

    void CameraNode::set_image_id(const id_type_16 id) {
        internal->promises[0].image.id = static_cast<id_type_16>(id);
        internal->promises[1].image.id = static_cast<id_type_16>(id);
        state.image_id                 = static_cast<id_type_16>(id);
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

        static char buff_form[5] {};
        ::memcpy(buff_form, cap.format_4cc, 4);

        state.mode.mode     = cap;
        state.mode.selected = true;
        snprintf(state.mode.description, 95,
            "%s (%s) - %d fps [%d x %d]", cap.description, buff_form,
                 cap.refresh_rate, cap.width, cap.height);
        state.mode.description[95] = '\0';

        controls = ex::platform::dev::list_controls(state.device.device);

        const ex::State &state_ = ex::logic::globals::state();
        if (internal->queue_p == nullptr) {
            internal->queue_p = xm::ocl::create_queue_device(state_.cl.context, state_.cl.device, true, false);
        }

        internal->b_index = 0;
        internal->promises[0].image.release();
        internal->promises[1].image.release();
        internal->promises[0].queue    = internal->queue_p;
        internal->promises[1].queue    = internal->queue_p;
        internal->promises[0].image    = ocl::Image2D::allocate(cap.width, cap.height, 3, 1, state_.cl.context);
        internal->promises[1].image    = ocl::Image2D::allocate(cap.width, cap.height, 3, 1, state_.cl.context);
        internal->promises[0].image.id = state.image_id;
        internal->promises[1].image.id = state.image_id;
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
        unsigned char *bytes = static_cast<unsigned char *>(::malloc(sizeof(camera_state)));
        ::memcpy(bytes, &state, sizeof(camera_state));
        return io::byte_array::from(bytes, sizeof(camera_state), &::free);
    }

    void CameraNode::deserialize(const io::byte_array &array) {
        if (array.empty())
            return;
        list_camera_devices(true);

        camera_state *parcel = reinterpret_cast<camera_state *>(array.data);
        set_image_id(parcel->image_id);
        set_device(parcel->device.device);
        set_device_buff(parcel->buffering);
        set_device_mode(parcel->mode.mode);

        state = *parcel;
        open();
    }

    void CameraNode::on_input(const int in_idx, const int out_idx, const int clock_i, const void *data) {
        // TODO process synchronizer
    }

    void CameraNode::on_update(const int clock_i) {

        const lw::status_t status = lw::status(internal->channel);
        if ((status & lw::status_t::success) != 0) {

            const auto t1 = std::chrono::high_resolution_clock::now();
            if (timings.t0 != 0) {
                const unsigned long dt = t1.time_since_epoch().count() - timings.t0;
                timings.frame_rate = static_cast<float>(1000000000. / dt);
            }

            if (result->type == action_read) {
                if (result->ready && !result->error) {
                    internal->b_index = (internal->b_index + 1) % 2; // swap buffers
                    internal->updates.refresh();
                }
            }

            else if (result->type == action_sync) {
                // TODO
            }

            else if (result->type == action_open) {
                if (result->ready && !result->error) {
                    internal->enabled = true;
                    list_camera_devices(true);
                    capabilities = ex::platform::dev::list_capabilities(state.device.device);
                    controls     = ex::platform::dev::list_controls(state.device.device);
                }
                log->info("Camera device: [{}] open: {}", state.device.device.uuid, internal->enabled);
            }

            timings.t0 = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            enqueue(this);
        }

        internal->promises[internal->b_index].image.header.fresh = internal->updates.update();

        if ((status & lw::status_t::failure) != 0)
            abort_("capture worker error");
    }

    void *CameraNode::on_output(const int in_idx, const int out_idx, const int clock_i) {
        if (!internal->enabled)
            return nullptr;
        return &internal->promises[internal->b_index];
    }

    CameraNode::~CameraNode() {
        internal->enabled = false;
        capture->camera.release();

        internal->promises[0].image.release();
        internal->promises[1].image.release();

        if (internal->queue_p != nullptr) {
            clReleaseCommandQueue(internal->queue_p);
            internal->queue_p = nullptr;
        }

        if (internal->channel != nullptr) {
            lw::close_channel(internal->channel);
            internal->channel = nullptr;
        }

        delete internal;
        delete capture;
        delete result;

        internal = nullptr;
        capture  = nullptr;
        result   = nullptr;
    }

    CameraNode::CameraNode() :
        internal(new internal_t()), // new bruh
        capture(new capture_t()),   // new bruh
        result(new result_t()),     // new bruh
        state({}),
        timings({0, 0})
    {
        start_worker();
        init_state(&state);
        init_internal(internal);
        init_capture(capture);
        init_result(result);
    }

}
