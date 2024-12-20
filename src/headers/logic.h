//
// Created by xd on 17/12/24.
//

#ifndef LOGIC_H
#define LOGIC_H

#include <map>

#include "components/c_camera.h"
#include "nodes/nodes.h"
#include "state.h"
#include "state/mappers.h"
#include "utils/log_utils.h"

namespace ex::main {

    class Logic {
    private:
        static inline const std::map<std::size_t, DataMappers> mappers_ = {
            { cmp::CameraNode::get_node_id(), map::camera::mappers },
        };

        std::map<std::size_t, nodes::BaseNode *> nodes_ = {};

        long timestamp = 0;

    public:
        static inline const auto log = logs::get_logger("logic");

        Logic() = default;

        ~Logic() = default;

        void initialize(State& state);

        void run(State &state);

    protected:
        void traverse_(State &state, TreeNode *node, bool *rewire);

        void synchronize_(State &state);

        void process_(State &state);
    };

    Logic create_logic(State& state);

}

#endif //LOGIC_H
