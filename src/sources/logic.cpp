//
// Created by xd on 17/12/24.
//

#include <map>
#include "../headers/logic.h"
#include "../headers/components/c_camera.h"

namespace ex::main {

    #define update_threshold 1000

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
        if (state.work_tree == nullptr) {
            // TODO RESET
            return;
        }
        process_(state);
        synchronize_(state);
    }

    void Logic::process_(State &state) {
        // TODO
    }

    void Logic::synchronize_(State &state) {
        const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        if (const auto elapsed = now - timestamp; elapsed < update_threshold)
            return;

        timestamp = now;

        for (std::size_t i = 0; i < state.work_tree->children_n; i++) {
            traverse_(state, &state.work_tree->children[i]);
        }
    }

    void Logic::traverse_(State &state, TreeNode *node) {
        if (node == nullptr)
            return;

        if (node->deleted) {
            // TODO rebuild tree
            return;
        }

        for (std::size_t i = 0; i < node->children_n; i++) {
            traverse_(state, &node->children[i]);
        }

        if (node->created) {
            nodes::BaseNode *new_node = mappers_.at(node->id_node).instance(state);
            mappers_.at(node->id_node).from_tree(node, new_node);
            nodes_.emplace(node->id_local, new_node);
            node->created = false;
        }

        if (node->updated) {
            node->updated = false;
            mappers_.at(node->id_node).from_tree(node, nodes_.at(node->id_local));
        }

        mappers_.at(node->id_node).from_base(nodes_.at(node->id_local), node);
    }

}
