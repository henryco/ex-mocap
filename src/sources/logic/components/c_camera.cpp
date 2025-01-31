//
// Created by xd on 15/12/24.
//

#include "../../../headers/logic/components/c_camera.h"
#include "../../../headers/nodes/nodes_component.h"

namespace ex::cmp {

    CameraNode::CameraNode(cl_context context, cl_device_id device)
    : camera_(context, device) {}

    CameraNode::CameraNode(const camera::Camera &camera)
    : camera_(camera) {}

    CameraNode::~CameraNode() {
        camera_.close();
    }

    camera::Camera *CameraNode::get() {
        return &camera_;
    }

    proto::image CameraNode::on_output(proto::image *_, const int __, const int clock_i, nodes::Input<proto::image> *io) {
        capture_frame(clock_i);
        return last_frame_;
    }

    void CameraNode::capture_frame(const int clock_i) {
        if (clock_i == last_clock_)
            return;
        last_clock_ = clock_i;
        last_frame_ = {
            .image_promise = camera_.capture(true),
            .sync_         = &CameraNode::sync_grab,
            .ctx___        = &this->camera_,
        };
    }

    bool CameraNode::sync_grab(void *ctx) {
        return static_cast<camera::Camera*>(ctx)->grab();
    }

}

namespace ex::map::camera {

    void from_base(State &state, nodes::BaseNode *from, TreeNode *to) {
        if (to->data == nullptr) {
            // TODO
        }

        const auto cam = static_cast<ex::camera::Camera *>(from->get_generic_data_ptr());
        // TODO
    }

    void from_tree(State &state, TreeNode *from, nodes::BaseNode *to) {
        if (from == nullptr || to == nullptr)
            return;

        const auto camera = static_cast<ex::camera::Camera *>(to->get_generic_data_ptr());
        if (camera == nullptr)
            return;
        // TODO
    }

    nodes::BaseNode *instance(const State &state, TreeNode *from) {
        if (from->data != nullptr)
            delete static_cast<state::camera *>(from->data);

        nodes::BaseNode *node = new cmp::CameraNode(state.ocl_context, state.ocl_device);
        node->set_id(from->id_local);
        return node;
    }

    nodes::BaseNode *relocate(const State &state, TreeNode *tree, nodes::BaseNode *self) {
        return self;
    }

}
