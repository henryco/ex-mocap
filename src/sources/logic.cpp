//
// Created by xd on 17/12/24.
//

#include "../headers/logic.h"

#include <map>

#include "../headers/components/c_camera.h"

namespace ex::main {

    namespace map::camera {
        void from_base(nodes::BaseNode *from, TreeNode *to) {
            Logic::log->info("FROM");
        }

        void from_tree(TreeNode *from, nodes::BaseNode *to) {
            Logic::log->info("TO");
        }
    }

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
