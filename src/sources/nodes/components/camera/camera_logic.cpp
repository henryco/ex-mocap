//
// Created by xd on 4/30/25.
//

#include <pthread.h>
#include <opencv2/videoio.hpp>

#include "../../../../headers/nodes/components/camera/camera_logic.h"

#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/logic/logic_nodes.h"
#include "../../../../headers/cl/cl_kernel.h"

#include "../../../../../extra/worker/worker.h"

namespace {
    // MAYBE MOVE WORKER OUT INTO GLOBAL REGISTRY ?
    namespace lw = limbo::worker;
    namespace lg = limbo::graph;

    lw::worker_p worker = nullptr;

    void idle_routine(lw::worker_p) {
        ::usleep(100);
    }

    void start_worker() {
        if (worker != nullptr)
            return;
        worker = lw::create_worker((1UL << 26), &idle_routine);
    }
}

namespace ex::logic::node::sync {

    using cam_ref_t = camera::CameraNode *;
    using sync_id_t = limbo::uid64_t;

    struct sync_t {
        sync_id_t node_id;
        cam_ref_t camera[254];
        unsigned  clock_i;
        unsigned  size;
    };

    sync_t *create_sync() {
        sync_t *instance = static_cast<sync_t *>(::malloc(sizeof(sync_t)));
        ::memset(instance, 0, sizeof(sync_t));
        return instance;
    }

    void free_sync(sync_t *ptr) {
        if (ptr == nullptr)
            return;
        ::free(ptr);
    }

}

namespace ex::logic::node::camera {

    namespace result {
        static constexpr unsigned OPEN_SUCCESS = 1U << 0;
        static constexpr unsigned OPEN_FAILURE = 1U << 1;
        static constexpr unsigned OPEN_BITMASK = OPEN_SUCCESS | OPEN_FAILURE;

        static constexpr unsigned READ_SUCCESS = 1U << 2;
        static constexpr unsigned READ_PENDING = 1U << 3;
        static constexpr unsigned READ_FAILURE = 1U << 4;
        static constexpr unsigned READ_BITMASK = READ_SUCCESS | READ_PENDING | READ_FAILURE;

        static constexpr unsigned SYNC_SUCCESS = 1U << 5;
        static constexpr unsigned SYNC_PENDING = 1U << 6;
        static constexpr unsigned SYNC_FAILURE = 1U << 7;
        static constexpr unsigned SYNC_BITMASK = SYNC_SUCCESS | SYNC_PENDING | SYNC_FAILURE;

        unsigned decode(const void *code_ptr) {
            return code_ptr == nullptr ? 0 : *static_cast<const unsigned *>(code_ptr);
        }
    }

    // 64 bytes touched once, dont care
    struct alignas(64) camera_t {
        std::vector<int>  params;
        cv::VideoCapture *camera;
        const unsigned    index;
        const unsigned    api;
    };

    // 192 bytes regularly touched by worker
    struct alignas(64) capture_t {
        cv::Mat            bitmap;
        cv::VideoCapture   camera;
        ocl::ImagePromise *target;
    };

    struct internal_t {
        ocl::img_promise promises[2];
        cl_command_queue queue_p;
        lw::channel_p    channel;
        unsigned int     b_index;
        lg::UpdateState  updates;
        bool             enabled;
    };

    struct serial_parcel_t {
        camera_state  state;
        unsigned long size;
    };

    namespace {

        void *open_routine(void *arg) {
            camera_t *camera = static_cast<camera_t *>(arg);

            bool success = false;
            if (camera->params.size() > 0)
                success = camera->camera->open(camera->index, camera->api, camera->params);
            else
                success = camera->camera->open(camera->index, camera->api);

            delete camera;

            return const_cast<unsigned *>(success ? &result::OPEN_SUCCESS : &result::OPEN_FAILURE);
        }

        void *sync_routine(void *arg) {
            // TODO
            return nullptr;
        }

        void *read_routine(void *arg) {
            capture_t *capture = static_cast<capture_t *>(arg);

            if (!capture->camera.isOpened())
                return const_cast<unsigned *>(&result::READ_FAILURE);
            if (!capture->camera.grab())
                return const_cast<unsigned *>(&result::READ_PENDING);
            if (!capture->camera.retrieve(capture->bitmap))
                return const_cast<unsigned *>(&result::READ_PENDING);

            ocl::ops::from_bytes(capture->bitmap.data, *capture->target);
            return const_cast<unsigned *>(&result::READ_SUCCESS);
        }

        void open_camera(CameraNode *that, const int api, const int index, const std::vector<int> &params) {
            camera_t *payload = new camera_t {
                params,
                &that->capture->camera,
                static_cast<unsigned>(index),
                static_cast<unsigned>(api)
            };

            lw::enqueue(that->internal->channel, &open_routine, payload);
        }

        void enqueue(CameraNode *that) {
            that->capture->target = &(that->internal->promises[(that->internal->b_index + 1) % 2]);
            lw::enqueue(that->internal->channel, &read_routine, that->capture);
        }

    }

    namespace {
        const auto log = logs::get_logger("node_camera");

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

        void process_open(CameraNode *that, const unsigned status) {
            if (status & result::OPEN_FAILURE) {
                log->info("Cannot open camera device: {}", that->state.device.device.uuid);
                return;
            }

            CameraNode::list_camera_devices(true);
            that->capabilities      = ex::platform::dev::list_capabilities(that->state.device.device);
            that->controls          = ex::platform::dev::list_controls(that->state.device.device);
            that->internal->enabled = true;
            log->info("Camera device: [{}]", that->state.device.device.uuid);

            enqueue(that);
        }

        void process_sync(CameraNode *that, const unsigned status) {
            // TODO result process
        }

        void process_read(CameraNode *that, const unsigned status) {
            if (status & result::READ_FAILURE)
                return;
            if (status & result::READ_PENDING) {
                enqueue(that);
                return;
            }
            that->internal->b_index = (that->internal->b_index + 1) % 2; // swap buffers
            that->internal->updates.refresh();
            enqueue(that);
        }
    }

    const ex::data::array<v_device_t> &CameraNode::list_camera_devices(const bool refresh) {
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
            if (internal->enabled)
                controls = ex::platform::dev::list_controls(state.device.device);
        }
        return controls;
    }

    const ex::data::array<v_devcap_t> &CameraNode::list_capabilities() const {
        return capabilities;
    }

    limbo::graph::BaseNode *CameraNode::instance(TreeNode *from) {
        CameraNode *ptr = new CameraNode();
        ptr->set_node_id(from->id_node);
        ptr->set_image_id(from->id_node);
        return ptr;
    }

    limbo::graph::BaseNode *CameraNode::relocate(TreeNode *, limbo::graph::BaseNode *node) {
        return node;
    }

    void CameraNode::destroy(TreeNode *, limbo::graph::BaseNode *node) {
        if (CameraNode *base = dynamic_cast<CameraNode *>(node))
            base->close();
        node->detach_all();
        delete node;
    }

    void CameraNode::close() {
        internal->enabled = false;
        ::memset(&timings, 0, sizeof(timings_t));

        {
            const lw::status_t status = lw::status(internal->channel);
            if ((status & lw::status_t::finished) == 0) {
                lw::stop(internal->channel);
                while ((lw::status(internal->channel) & lw::status_t::finished) == 0) {
                    ::usleep(100);
                }
            }
        }

        if (!capture)
            return;

        if (capture->camera.isOpened())
            capture->camera.release();

        capture->bitmap.release();
    }

    void CameraNode::reset() {
        list_camera_devices(true);

        capabilities.clean();
        capabilities = ex::platform::dev::list_capabilities(state.device.device);

        if (internal->enabled) {
            controls.clean();
            controls = ex::platform::dev::list_controls(state.device.device);
            open();
            return;
        }

        for (int i = 0; i < controls.size; ++i) {
            const v_control_t &item = controls[i];
            if (item.step == 0 || item.min == item.max)
                continue;
            set_control(item.id, item.value);
        }

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

        log->debug("Enqueued camera open");
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
        if (!state.device.selected)
            return io::byte_array::from(nullptr, 0);
        if (!state.mode.selected)
            return io::byte_array::from(nullptr, 0);

        const unsigned long controls_size = controls.size * sizeof(v_control_t);
        const unsigned long total_size    = sizeof(serial_parcel_t) + controls_size;

        serial_parcel_t parcel;
        parcel.state = state;
        parcel.size  = static_cast<unsigned long>(controls.size);

        unsigned char *bytes = static_cast<unsigned char *>(::malloc(total_size));
        ::memcpy(bytes, &parcel, sizeof(serial_parcel_t));
        ::memcpy(bytes + sizeof(serial_parcel_t), controls.data, controls_size);

        return io::byte_array::from(bytes, controls_size, &::free);
    }

    void CameraNode::deserialize(const io::byte_array &array) {
        if (array.empty())
            return;
        list_camera_devices(true);

        serial_parcel_t *serial = reinterpret_cast<serial_parcel_t *>(array.data);
        camera_state    *parcel = &serial->state;

        controls.clean();

        set_image_id(parcel->image_id);
        set_device(parcel->device.device);
        set_device_buff(parcel->buffering);
        set_device_mode(parcel->mode.mode);

        // sanity check
        if (serial->size < 1024) {
            for (int i = 0; i < serial->size; ++i) {
                const v_control_t *s_controls = reinterpret_cast<v_control_t *>(array.data + sizeof(serial_parcel_t));
                const v_control_t &item       = s_controls[i];
                if (item.step == 0 || item.min == item.max)
                    continue;
                set_control(item.id, item.value);
                controls.push(item);
            }
        }

        state = *parcel;
        open();
    }

    void CameraNode::on_input(const int in_idx, const int out_idx, const int clock_i, const void *data) {
        // TODO process synchronizer
    }

    void CameraNode::on_update(const int clock_i) {
        void              *result = nullptr;
        const lw::status_t status = lw::result(internal->channel, &result);

        if (status & lw::status_t::finished)
            lw::consume(internal->channel);

        if (status & lw::status_t::failure)
            abort_("camera worker error");

        if (status & lw::status_t::success) {
            const unsigned code = result::decode(result);

            const auto t1 = std::chrono::high_resolution_clock::now();
            if (timings.t0 != 0 && internal->enabled) {
                const unsigned long dt = t1.time_since_epoch().count() - timings.t0;
                timings.frame_rate = static_cast<float>(1000000000. / dt);
            }

            if (code & result::READ_BITMASK) {
                process_read(this, code);
            }

            else if (code & result::SYNC_BITMASK) {
                process_sync(this, code);
            }

            else if (code & result::OPEN_BITMASK) {
                process_open(this, code);
            }

            timings.t0 = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        }

        internal->promises[internal->b_index].image.header.fresh = internal->updates.update();
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

        internal = nullptr;
        capture  = nullptr;
    }

    CameraNode::CameraNode() :
        internal(new internal_t()), // new bruh
        capture(new capture_t()),   // new bruh
        state({}),
        timings({0, 0})
    {
        start_worker();
        init_state(&state);
        init_internal(internal);
        init_capture(capture);
    }

}


// SYNC NODE
namespace ex::logic::node::sync {

    struct sync_serial_t {
        sync_id_t node_id;
    };

    limbo::graph::BaseNode *SyncNode::instance(TreeNode *from) {
        SyncNode *node = new SyncNode();
        node->set_node_id(from->id_node);
        return node;
    }

    limbo::graph::BaseNode *SyncNode::relocate(TreeNode *, limbo::graph::BaseNode *node) {
        return node;
    }

    void SyncNode::destroy(TreeNode *, limbo::graph::BaseNode *node) {
        node->detach_all();
        delete node;
    }

    io::byte_array SyncNode::serialize() const {
        constexpr long total_size = sizeof(sync_serial_t);
        sync_serial_t parcel;
        parcel.node_id = synchronizer->node_id;

        unsigned char *bytes = static_cast<unsigned char *>(::malloc(total_size));
        ::memcpy(bytes, &parcel, sizeof(sync_serial_t));

        return io::byte_array::from(bytes, total_size, &::free);
    }

    void SyncNode::deserialize(const io::byte_array &array) {
        if (array.empty())
            return;

        const sync_serial_t *parcel = reinterpret_cast<sync_serial_t *>(array.data);
        synchronizer->node_id = parcel->node_id;
    }

    void SyncNode::on_input(int, int, int, const void *) {
    }

    void SyncNode::on_update(const int clock_i) {
        synchronizer->node_id = get_node_id();
        synchronizer->clock_i = clock_i;
    }

    void *SyncNode::on_output(int, int, int) {
        return &synchronizer;
    }

    SyncNode::~SyncNode() {
        free_sync(synchronizer);
        synchronizer = nullptr;
    }

    SyncNode::SyncNode():
        synchronizer(create_sync()) {
    }
}