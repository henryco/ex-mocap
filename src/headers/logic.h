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

    using local_id_ = std::size_t;
    using type_id_  = std::size_t;

    class Logic {
    private:
        static inline const std::map<type_id_, DataMappers> mappers_ = {
            { cmp::CameraNode::get_node_id(), map::camera::mappers },
        };

        std::map<local_id_, nodes::BaseNode *> nodes_ = {};

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

        void rewire_(State &state);

        void restore_(State &state);
    };

    Logic create_logic(State& state);

}

#endif //LOGIC_H
