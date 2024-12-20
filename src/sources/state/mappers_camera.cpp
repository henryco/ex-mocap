//
// Created by xd on 19/12/24.
//

#include "../../headers/state/mappers.h"
#include "../../headers/components/c_camera.h"

namespace ex::map::camera {

    void from_base(State &state, nodes::BaseNode *from, TreeNode *to) {
        auto *node = dynamic_cast<cmp::CameraNode *>(from);
        if (node == nullptr) {
            to->disabled = true;
            to->deleted  = true;
            return;
        }

        if (to->data == nullptr) {
            to->data = new state::camera{};
        }

        auto data = static_cast<state::camera *>(to->data);
        auto camera = node->get();

        // TODO
    }

    void from_tree(State &state, TreeNode *from, nodes::BaseNode *to) {
        auto *node = dynamic_cast<cmp::CameraNode *>(to);
        if (node == nullptr)
            return;

        auto camera = node->get();
        // TODO
    }

    nodes::BaseNode *re_instance(State &state, TreeNode *from, nodes::BaseNode *src) {
        return src;
    }

    nodes::BaseNode *instance(const State &state) {
        return new cmp::CameraNode(state.ocl_context, state.ocl_device);
    }

}