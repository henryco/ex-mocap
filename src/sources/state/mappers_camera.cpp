//
// Created by xd on 19/12/24.
//

#include "../../headers/components/c_camera.h"
#include "../../headers/nodes/nodes_component.h"

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

} // namespace ex::map::camera
