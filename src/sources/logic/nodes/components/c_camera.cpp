//
// Created by xd on 15/12/24.
//

#include "../../../../headers/logic/nodes/components/c_camera.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/logic/nodes/logic_nodes.h"

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

    proto::image CameraNode::on_output(proto::image *_, const int __, const int clock_i, limbo::nodes::Input<proto::image> *io) {
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

    limbo::nodes::BaseNode *instance(TreeNode *from) {
        const State            &state = ex::logic::globals::state();
        limbo::nodes::BaseNode *node = new cmp::CameraNode(state.cl.context, state.cl.device);
        node->set_id(from->id_local);
        return node;
    }

    limbo::nodes::BaseNode *relocate(TreeNode *tree, limbo::nodes::BaseNode *self) {
        return self;
    }

}
