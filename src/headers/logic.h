//
// Created by xd on 17/12/24.
//

#ifndef LOGIC_H
#define LOGIC_H

#include <map>


#include "components/c_camera.h"
#include "nodes/nodes.h"
#include "state.h"
#include "utils/log_utils.h"

namespace ex::main {

    struct DataMappers {
        void (*from)(nodes::BaseNode *node, TreeNode *state) = nullptr;
        void (*to)(TreeNode *state, nodes::BaseNode *node)   = nullptr;
    };

    namespace map::camera {
        void from(nodes::BaseNode *node, TreeNode *state);
        void to(TreeNode *state, nodes::BaseNode *node);
    }

    class Logic {
    private:
        static inline const std::map<std::size_t, DataMappers> mappers_ = {
            { cmp::CameraNode::get_node_id(), { .from = map::camera::from, .to = map::camera::to } },
        };

    public:
        static inline const auto log = logs::get_logger("logic");

        Logic() = default;

        ~Logic() = default;

        void initialize(State& state);

        void run(State &state);
    };

    Logic create_logic(State& state);

}

#endif //LOGIC_H
