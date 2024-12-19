//
// Created by xd on 17/12/24.
//

#include "../headers/logic.h"

#include <map>

#include "../headers/components/c_camera.h"

namespace ex::main {

    struct DataMappers {
        void (*from)(nodes::BaseNode *node, TreeNode *state) = nullptr;
        void (*to)(TreeNode *state, nodes::BaseNode *node)   = nullptr;
    };

    static std::map <std::size_t, DataMappers> mappers_ = {
        {cmp::CameraNode::get_node_id(), {}}
    };

    Logic create_logic(State &state) {
        Logic logic;
        logic.initialize(state);
        return logic;
    }

    void Logic::initialize(State &state) {
        log->info("main logic initialization started");
        // TODO
        log->info("main logic initialization complete");
    }

    void Logic::run(State &state) {
        // TODO
    }


} // namespace ex::logic
