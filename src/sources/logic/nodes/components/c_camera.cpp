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

    void CameraNode::on_input(int in_idx, int out_idx, int clock_i, const void *data) {
        // TODO
    }

    void *CameraNode::on_output(int in_idx, int out_idx, int clock_i) {
        // TODO
        return nullptr;
    }

    void CameraNode::on_update(int clock_i) {
        // TODO
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

    limbo::graph::BaseNode *instance(TreeNode *from) {
        const State            &state = ex::logic::globals::state();
        limbo::graph::BaseNode *node = new cmp::CameraNode(state.cl.context, state.cl.device);
        node->set_id(from->id_node);
        return node;
    }

    limbo::graph::BaseNode *relocate(TreeNode *, limbo::graph::BaseNode *self) {
        return self;
    }

}
