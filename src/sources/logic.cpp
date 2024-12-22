//
// Created by xd on 17/12/24.
//

#include <map>
#include "../headers/logic.h"
#include "../headers/components/c_camera.h"

namespace ex::main {

    #define update_threshold 1000

    void connect_nodes__(TreeNode *from, TreeNode *to) {
        if (from == nullptr || to == nullptr)
            return;
        // TODO connect IO nodes
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

    void Logic::serialize(State &state) {
        // TODO write to json from clock
    }

    void Logic::deserialize_(State &state) {
        // TODO read from json to clock
    }

    void Logic::restore_(State &state) {
        if (state.work_tree != nullptr)
            delete state.work_tree;

        state.work_tree = new TreeNode();
        state.work_tree->children_n = 0;
        state.work_tree->is_group   = true;

        for (auto &[local_id, data]: data_) {
            mappers_.at(types_.at(local_id)).cleanup(data);
        }

        // I see the potential memory leak here for "nodes_"
        counter_.clear();
        nodes_.clear();
        types_.clear();
        data_.clear();

        // TODO build from IO nodes (clock)
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

                // create or duplicate node
                new_node = nodes_.contains(node->id_local)
                                   ? mappers_.at(node->id_node).re_instance(state, node, nodes_.at(node->id_local))
                                   : mappers_.at(node->id_node).instance(state, node);
                nodes_.emplace(node->id_local, new_node);


                // update types storage
                if (types_.contains(node->id_local)) {
                    types_.erase(node->id_local);
                    types_.emplace(node->id_local, node->id_node);
                } else
                    types_.emplace(node->id_local, node->id_node);


                // update data storage
                if (data_.contains(node->id_local)) {
                    if (void *data = data_.at(node->id_local); data != node->data) {
                        data_.erase(node->id_local);
                        data_.emplace(node->id_local, node->data);
                        mappers_.at(node->id_node).cleanup(data);
                    }
                } else
                    data_.emplace(node->id_local, node->data);


                // update node ptr counter
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

        cleanup_(state);

        const simple_list *head = get_head_(&current);
        if (head == nullptr) {
            return;
        }

        const TreeNode *node = head->node;
        if (node == nullptr) {
            return;
        }

        if (!nodes_.contains(node->id_local)) {
            log->error("Node does not exist: {}", node->id_local);
            return;
        }

        nodes::BaseNode *base_node = nodes_.at(node->id_local);
        if (base_node == nullptr) {
            log->error("Node is nullptr: {}", node->id_local);
            return;
        }

        // TODO connect to clock
    }

    void Logic::cleanup_(State &_) {
        for (auto it = counter_.begin(); it != counter_.end(); /*nop*/) {
            const auto id_local = it->first;
            if (it->second <= 0) {
                mappers_.at(types_.at(id_local)).cleanup(data_.at(id_local));
                types_.erase(id_local);
                data_.erase(id_local);

                delete nodes_.at(id_local);
                nodes_.erase(id_local);

                it = counter_.erase(it);
            } else
                ++it;
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

} // namespace ex::main
