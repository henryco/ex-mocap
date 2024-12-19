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

    namespace mappers {
        struct DataMappers {
            void (*from)(nodes::BaseNode *node, TreeNode *state) = nullptr;
            void (*to)(TreeNode *state, nodes::BaseNode *node)   = nullptr;
        };
    }

    class Logic {
    private:
        static inline const auto log =
            logs::get_logger("logic");

        static inline std::map<std::size_t, mappers::DataMappers> mappers_ = {
            {cmp::CameraNode::get_node_id(), {}}
        };

    public:
        Logic() = default;

        ~Logic() = default;

        void initialize(State& state);

        void run(State &state);
    };

    Logic create_logic(State& state);

}

#endif //LOGIC_H
