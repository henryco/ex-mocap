//
// Created by xd on 04/06/25.
//
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/nodes/components/calibration/calibration_logic.h"

#include "../../../../../extra/worker/worker.h"

namespace ex::logic::node::calibration {

    static const auto log = logs::get_logger("node_calibration");

    // 1. Need worker thread
    // 2. Need double buffering for draws (call draw_bytes_bgra_8)
    // 3. Unload -> gray -> points -> refresh_t -> accumulate -> draw -> calibrate

    struct updates_t {
        id_type_16                prev_h;
        limbo::graph::UpdateState input;
        limbo::graph::UpdateState state;
    };

    struct buffers_t {
        ocl::img_promise input; // managed outside
    };

    struct internal_t {
        cv::socalib_t calibration;
        buffers_t     buffers;
        config_t      state;
        updates_t     updates;
        bool          enabled;
    };

    struct serial_t {
        cv::socalib_t calibration;
        config_t      config;
        id_type_16    id;
    };

    static void init_config(config_t *c) {
        // TODO
    }

    static internal_t *create_internal() {
        log->debug("create calibration node");

        internal_t *instance = static_cast<internal_t *>(::malloc(sizeof(internal_t)));
        if (instance == nullptr)
            abort_("cannot create calibration node instance, nullptr");

        ::memset(instance, 0, sizeof(internal_t));
        init_config(&instance->state);

        log->debug("calibration node created");
        return instance;
    }

    static void destroy_internal(internal_t *ptr) {
        if (!ptr)
            return;
        log->debug("destroying calibration node");
        ::free(ptr); // TODO ?
        log->debug("calibration node destroyed");
    }

    static void reset(internal_t *internal) {
        // TODO ?
    }

    limbo::graph::BaseNode *CalibNode::instance(TreeNode *from) {
        auto *instance = new CalibNode();
        instance->set_node_id(from->id_node);
        instance->enable();
        return instance;
    }

    limbo::graph::BaseNode *CalibNode::relocate(TreeNode *, limbo::graph::BaseNode *node) {
        if (auto *t = dynamic_cast<CalibNode *>(node))
            reset(t->internal);
        return node;
    }

    void CalibNode::destroy(TreeNode *, limbo::graph::BaseNode *node) {
        node->detach_all();
        delete node;
    }

    void CalibNode::set_config(const config_t &conf) {
        config_t copy;
        ::memcpy(&copy, &conf, sizeof(config_t));

        // TODO?

        if (0 == ::memcmp(&copy, &internal->state, sizeof(config_t)))
            return;

        ::memcpy(&internal->state, &copy, sizeof(config_t));
        ::memcpy(&config, &copy, sizeof(config_t));
        reset(internal);
    }

    id_type_16 CalibNode::get_input_id() const {
        return internal->buffers.input.image.id;
    }

    unsigned CalibNode::get_input_w() const {
        return internal->buffers.input.image.cols;
    }

    unsigned CalibNode::get_input_h() const {
        return internal->buffers.input.image.rows;
    }

    void CalibNode::enable() {
        reset(internal);
        internal->enabled = true;
    }

    void CalibNode::disable() {
        internal->enabled = false;
        reset(internal);
    }

    io::byte_array CalibNode::serialize() const {
        constexpr int total_size = sizeof(serial_t);

        serial_t parcel;
        parcel.id          = id_;
        parcel.config      = internal->state;
        parcel.calibration = internal->calibration;

        unsigned char *bytes = static_cast<unsigned char *>(::malloc(total_size));
        ::memcpy(bytes, &parcel, total_size);

        return io::byte_array::from(bytes, total_size, &::free);
    }

    void CalibNode::deserialize(const io::byte_array &array) {
        if (array.empty())
            return;

        const serial_t *serial = reinterpret_cast<serial_t *>(array.data);
        ::memcpy(&id_, &serial->id, sizeof(id_type_16));
        ::memcpy(&internal->state, &serial->config, sizeof(config_t));
        ::memcpy(&internal->calibration, &serial->calibration, sizeof(cv::socalib_t));
        ::memcpy(&config, &serial->config, sizeof(config_t));
    }

    void CalibNode::on_input(const int in_idx, const int out_idx, const int clock_i, void *data) {
        if (!internal->enabled)
            return;

        ocl::img_promise *promise = static_cast<ocl::img_promise *>(data);
        if (promise == nullptr) {
            reset(internal);
            return;
        }

        if (promise->empty()) {
            reset(internal);
            return;
        }

        if (internal->buffers.input.image.cols != promise->image.cols ||
            internal->buffers.input.image.rows != promise->image.rows ||
            internal->updates.prev_h != promise->image.id) {

            internal->updates.prev_h = promise->image.id;
            internal->buffers.input  = *promise;

            if (!promise->empty()) {
                internal->updates.input.refresh();
                internal->updates.state.refresh();

                // TODO ALLOCATE ?

                ::memcpy(&config, &internal->state, sizeof(config_t));
            }
            return;
        }

        internal->updates.input.refresh();
        internal->buffers.input = *promise;
    }

    void CalibNode::on_update(int clock_i) {
        if (!internal->enabled)
            return;
        // TODO
        return;
    }

    void *CalibNode::on_output(int in_idx, int out_idx, int clock_i) {
        if (!internal->enabled)
            return nullptr;
        // TODO
        return &internal->calibration;
    }

    CalibNode::~CalibNode() {
        destroy_internal(internal);
        internal = nullptr;
    }

    CalibNode::CalibNode() : internal(create_internal()) {
        ::memcpy(&config, &internal->state, sizeof(config_t));
        reset(internal);
    }

} // namespace ex::logic::node::calibration
