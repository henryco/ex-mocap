//
// Created by xd on 4/30/25.
//

#include "../../../../headers/nodes/components/camera/camera_logic.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/logic/logic_nodes.h"
#include "../../../../headers/cl/cl_kernel.h"

namespace ex::logic::node::camera {

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

    void CameraNode::set_image_id(const id_type_16 id) {
        state.image_id    = static_cast<id_type_16>(id);
        promise_.image.id = static_cast<id_type_16>(id);
        // TODO
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
        // TODO
        return &promise_;
    }

    CameraNode::~CameraNode() {
        // TODO
        if (queue_ != nullptr) {
            clReleaseCommandQueue(queue_);
            queue_ = nullptr;
        }
    }

    CameraNode::CameraNode() :
        state({ .image_id = 0, .selected = false }),
        queue_(nullptr) {
        // TODO
    }

} // namespace ex::logic::node::camera
