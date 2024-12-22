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
        deserialize_(state);
        restore_(state);
        log->info("main logic initialization complete");
    }

    void Logic::run(State &state) {
        if (state.work_tree == nullptr) {
            restore_(state);
            return;
        }
        synchronize_(state);
        process_(state);
    }

    void Logic::deserialize_(State &state) {
        // TODO
    }

    void Logic::process_(State &_) {
        clock_.next();
    }

    void Logic::synchronize_(State &state) {
        const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        if (const auto elapsed = now - timestamp; elapsed < update_threshold)
            return;

        timestamp = now;

        bool rewire = false;
        for (std::size_t i = 0; i < state.work_tree->children_n; i++) {
            traverse_(state, &state.work_tree->children[i], &rewire);
        }

        if (rewire) {
            rewire_(state);
        }
    }

    void Logic::traverse_(State &state, TreeNode *node, bool *rewire) {
        if (node == nullptr)
            return;

        if (node->deleted) {
            *rewire = true;
            return;
        }

        for (std::size_t i = 0; i < node->children_n; i++) {
            traverse_(state, &node->children[i], rewire);
        }

        if (node->created) {
            if (!node->is_group) {
                nodes::BaseNode *new_node = nullptr;

                new_node = nodes_.contains(node->id_local)
                                   ? mappers_.at(node->id_node).re_instance(state, node, nodes_.at(node->id_local))
                                   : mappers_.at(node->id_node).instance(state);

                mappers_.at(node->id_node).from_tree(state, node, new_node);
                nodes_.emplace(node->id_local, new_node);

                if (counter_.contains(node->id_local))
                    counter_.at(node->id_local)++;
                else
                    counter_.emplace(node->id_local, 1);
            }
            node->created = false;
            *rewire = true;
        }

        if (node->is_group || !nodes_.contains(node->id_local))
            return;

        nodes::BaseNode *node_ptr = nodes_.at(node->id_local);

        if (node->updated) {
            mappers_.at(node->id_node).from_tree(state, node, node_ptr);
            node->updated = false;
        }

        mappers_.at(node->id_node).from_base(state, node_ptr, node);
    }

    void Logic::rewire_(State &state) {
        clock_.detach_all();
        for (auto &[local_id, node]: nodes_) {
            node->detach_all_any();
        }

        simple_list current = { state.work_tree };
        build_tree_(state, state.work_tree, nullptr, &current);
    }

    void connect_nodes__(TreeNode *from, TreeNode *to) {
        if (from == nullptr || to == nullptr)
            return;
        // TODO
    }

    simple_list *get_head_(simple_list *node) {
        if (node == nullptr)
            return nullptr;
        simple_list *head = node;
        while (true) {
            if (head->next == nullptr)
                return head;
            head = head->next;
        }
    }

    void Logic::build_tree_(State &state, TreeNode *node, TreeNode *previous, simple_list *group_list) {
        if (node == nullptr || group_list == nullptr)
            return;

        if (node->is_group) {
            TreeNode *prev_node = previous;
            TreeNode *curr_node = nullptr;

            for (std::size_t i = 0; i < node->children_n; i++) {
                curr_node = &node->children[i];

                build_tree_(state, curr_node, prev_node, group_list);

                if (!curr_node->is_group) {
                    prev_node = curr_node;
                }
            }

            if (curr_node != nullptr && !curr_node->is_group) {
                // add last non-group element within group to list
                get_head_(group_list)->next = new simple_list(curr_node);
            }
            return;
        }

        connect_nodes__(previous, node);
        for (std::size_t i = 0; i < node->children_n; i++) {
            TreeNode *curr_node = &node->children[i];

            // non-group element can contain only groups as children
            if (!curr_node->is_group)
                continue;

            simple_list list = {};
            build_tree_(state, curr_node, nullptr, &list);

            const simple_list *head = list.next;
            while (head != nullptr) {
                // collect results of the group traversal
                connect_nodes__(head->node, node);
                head = head->next;
            }
        }
    }

    void Logic::restore_(State &state) {
        // TODO
    }

} // namespace ex::main
