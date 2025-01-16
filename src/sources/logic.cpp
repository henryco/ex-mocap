//
// Created by xd on 17/12/24.
//

#include <map>
#include "../headers/logic.h"
#include "../headers/components/c_camera.h"

namespace ex::main {

    #define update_threshold 1000

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

        // TODO UNCOMMENT
        // deserialize_(state);
        // restore_(state);
        // TODO UNCOMMENT


        // TODO REMOVE
        if (state.work_tree != nullptr)
            delete state.work_tree;

        state.work_tree             = new TreeNode();
        state.work_tree->name       = "root";
        state.work_tree->is_group   = true;
        state.work_tree->children_n = 0;

        std::size_t local_id_ct = 1;
        for (int i = 0; i < 5; i++) {
            const auto node  = new TreeNode();
            node->created    = true;
            node->children_n = 0;
            node->name       = (new std::string("Node " + std::to_string(local_id_ct)))->c_str();
            node->is_group   = i > 2;
            node->id_local   = local_id_ct++;
            node->id_node    = 0;

            if (i > 2) { // inside a group
                for (int k = 0; k < 3; k++) {
                    const auto sub  = new TreeNode();
                    sub->created    = true;
                    sub->children_n = 0;
                    sub->name       = "Sub one";
                    sub->is_group   = k == 1;
                    sub->id_local   = local_id_ct++;
                    sub->id_node    = 1;

                    if (k == 1) { // inside a group
                        for (int z = 0; z < 5; z++) {
                            const auto sub_2  = new TreeNode();
                            sub_2->created    = true;
                            sub_2->children_n = 0;
                            sub_2->name       = "Sub zero";
                            sub_2->is_group   = false;
                            sub_2->id_local   = local_id_ct++;
                            sub_2->id_node    = 2;

                            sub->add_node(sub_2);
                        }
                    } else {
                        const auto inputs  = new TreeNode();
                        inputs->created    = true;
                        inputs->children_n = 0;
                        inputs->name       = "inputs";
                        inputs->is_group   = true;
                        inputs->id_local   = local_id_ct++;

                        for (int z = 0; z < 3; z++) {
                            const auto in  = new TreeNode();
                            in->created    = true;
                            in->children_n = 0;
                            in->name       = "Input!";
                            in->is_group   = false;
                            in->id_local   = local_id_ct++;
                            in->id_node    = 3;

                            inputs->add_node(in);
                        }

                        sub->add_node(inputs);
                    }

                    node->add_node(sub);
                }
            }

            else { // inside a regular element
                const auto inputs  = new TreeNode();
                inputs->created    = true;
                inputs->children_n = 0;
                inputs->name       = (new std::string("Inputs. " + std::to_string(local_id_ct)))->c_str();
                inputs->is_group   = true;
                inputs->id_local   = local_id_ct++;

                for (int k = 0; k < 4; k++) { // inside inputs group
                    const auto sub  = new TreeNode();
                    sub->created    = true;
                    sub->children_n = 0;
                    sub->name       = (new std::string("Sub in " + std::to_string(local_id_ct)))->c_str();
                    sub->is_group   = k == 1 || k == 2;
                    sub->id_local   = local_id_ct++;
                    sub->id_node    = 4;

                    if (k == 1 || k == 2) { // inside a group
                        for (int z = 0; z < 6; z++) {
                            const auto in  = new TreeNode();
                            in->created    = true;
                            in->children_n = 0;
                            in->name       = (new std::string("in in " + std::to_string(local_id_ct)))->c_str();
                            in->is_group   = false;
                            in->id_local   = local_id_ct++;
                            in->id_node    = 5;

                            sub->add_node(in);
                        }
                    } else {
                        const auto another_in = new TreeNode();
                        another_in->created    = true;
                        another_in->children_n = 0;
                        another_in->name       = (new std::string("another_in " + std::to_string(local_id_ct)))->c_str();
                        another_in->is_group   = true;
                        another_in->id_local   = local_id_ct++;

                        for (int p = 0; p < 2; p++) {
                            const auto sub_3  = new TreeNode();
                            sub_3->created    = true;
                            sub_3->children_n = 0;
                            sub_3->name       = (new std::string("another in " + std::to_string(local_id_ct)))->c_str();
                            sub_3->is_group   = false;
                            sub_3->id_local   = local_id_ct++;
                            sub_3->id_node    = 6;

                            another_in->add_node(sub_3);
                        }

                        sub->add_node(another_in);
                    }

                    inputs->add_node(sub);
                }

                node->add_node(inputs);
            }

            state.work_tree->add_node(node);
        }
        // TODO REMOVE

        log->info("main logic initialization synchronization");
        synchronize_(state);
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

        for (auto &[local_id, data]: view_data_) {
            mappers_.at(nodes_.at(local_id)->get_type_id()).cleanup(data);
        }

        for (auto &[_, data]: nodes_) {
            delete data;
        }

        ptr_counter_.clear();
        view_data_.clear();
        nodes_.clear();
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
            traverse_(state, state.work_tree->children[i], &rewire);
        }

        if (rewire) {
            rewire_(state);
        }
    }

    void Logic::mark_deleted_(TreeNode *node) {
        node->deleted = true;
        if (ptr_counter_.contains(node->id_local))
            ptr_counter_.at(node->id_local)--;
        for (std::size_t i = 0; i < node->children_n; i++) {
            mark_deleted_(node->children[i]);
        }
    }

    void Logic::traverse_(State &state, TreeNode *node, bool *rewire) {
        if (node == nullptr)
            return;

        if (node->deleted) {
            mark_deleted_(node);
            *rewire = true;
            return;
        }

        for (std::size_t i = 0; i < node->children_n; i++) {
            const auto child = node->children[i];

            if (child->deleted) {
                mark_deleted_(child);
                *rewire = true;
            }

            else {
                traverse_(state, child, rewire);
                if (child->is_group && (child->children_n <= 0 || child->children == nullptr)) {
                    mark_deleted_(child);
                    *rewire = true;
                }
            }
        }

        int o = 0;
        while (true) {
            if (o >= node->children_n)
                break;
            if (!node->children[o]->deleted) {
                o += 1;
                continue;
            }
            node->remove_node(node->children[o]);
            o = 0;
        }

        if (node->created) {
            if (!node->is_group) {

                if (node->id_local < 0) {
                    static std::size_t c_ = 1;
                    node->id_local        = c_++;
                }

                // create or duplicate node
                const DataMappers &mappers = mappers_.at(node->id_node);
                if (nodes_.contains(node->id_local)) {
                    // basically we create reference
                    nodes::BaseNode *old_one = nodes_.at(node->id_local);
                    if (nodes::BaseNode *new_one = mappers.re_instance(state, node, old_one); old_one != new_one) {
                        nodes_.erase(node->id_local);
                        nodes_.emplace(node->id_local, new_one);
                    }
                } else
                    nodes_.emplace(node->id_local, mappers.instance(state, node));


                // update data storage
                if (view_data_.contains(node->id_local)) {
                    if (void *data = view_data_.at(node->id_local); data != node->data) {
                        view_data_.erase(node->id_local);
                        view_data_.emplace(node->id_local, node->data);
                        mappers.cleanup(data);
                    }
                } else
                    view_data_.emplace(node->id_local, node->data);


                // update node ptr counter
                if (ptr_counter_.contains(node->id_local))
                    ptr_counter_.at(node->id_local)++;
                else
                    ptr_counter_.emplace(node->id_local, 1);
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
        log->debug("rewire");

        clock_.detach_all();
        for (auto &[_, node]: nodes_) {
            node->detach_all_any();
        }

        simple_list current = { state.work_tree };
        build_tree_(state, state.work_tree, nullptr, &current);

        gc_(state);

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

        int outputs_num = 0;
        const auto outputs = base_node->describe_outputs(&outputs_num);

        for (std::size_t i = 0; i < outputs_num; i++) {
            const auto &output = outputs[i];
            if (output.aux)
                continue;

            if (clock_.attach(base_node, output.index)) {
                log->debug("Node {} [{}][{}] (main) attached to clock",
                    base_node->get_node_name(), node->id_local, output.index);
                return;
            }
        }

        for (std::size_t i = 0; i < outputs_num; i++) {
            const auto &output = outputs[i];
            if (!output.aux)
                continue;

            if (clock_.attach(base_node, output.index)) {
                log->debug("Node {} [{}][{}] (aux) attached to clock",
                    base_node->get_node_name(), node->id_local, output.index);
                return;
            }
        }

        log->error("Cannot connect node to clock");
    }

    void Logic::gc_(State &state) {
        log->debug("start node gc");

        for (auto it = ptr_counter_.begin(); it != ptr_counter_.end(); /*nop*/) {
            const auto id_local = it->first;
            if (it->second <= 0) {
                mappers_.at(nodes_.at(id_local)->get_type_id()).cleanup(view_data_.at(id_local));
                view_data_.erase(id_local);

                delete nodes_.at(id_local);
                nodes_.erase(id_local);

                it = ptr_counter_.erase(it);

                log->debug("node gc remove: {}", id_local);

            } else
                ++it;
        }

        log->debug("complete node gc");
    }

    void Logic::build_tree_(State &state, TreeNode *node, TreeNode *previous, simple_list *group_list) {
        if (node == nullptr || group_list == nullptr || node->deleted)
            return;

        if (node->is_group) {
            TreeNode *prev_node = previous;
            TreeNode *curr_node = nullptr;

            for (std::size_t i = 0; i < node->children_n; i++) {
                curr_node = node->children[i];

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

        connect_nodes_(previous, node);
        for (std::size_t i = 0; i < node->children_n; i++) {
            TreeNode *curr_node = node->children[i];

            // non-group element can contain only groups as children
            if (!curr_node->is_group)
                continue;

            simple_list list = {};
            build_tree_(state, curr_node, nullptr, &list);

            const simple_list *head = list.next;
            while (head != nullptr) {
                // collect results of the group traversal
                connect_nodes_(head->node, node);
                head = head->next;
            }
        }
    }

    void Logic::connect_nodes_(TreeNode *from, TreeNode *to) {
        if (from == nullptr || to == nullptr)
            return;

        nodes::BaseNode *node_dst = nodes_.at(to->id_local);
        nodes::BaseNode *node_src = nodes_.at(from->id_local);

        if (node_dst == nullptr || node_src == nullptr) {
            log->error("One of nodes does not exist: {}, {}", to->id_local, from->id_local);
            return;
        }

        int        inputs_num  = 0;
        int        outputs_num = 0;
        const auto inputs      = node_dst->describe_inputs(&inputs_num);
        const auto outputs     = node_src->describe_outputs(&outputs_num);

        // [a2    c4 c5 c6 d7 e8    g9   ] <- in
        // [   b1 c2 c3    d4    f5 g6 g7] <- out

        // [a2 c4 c5 c6 d7 e8 g9] <-  in
        // [b1 c2 c3 d4 f5 g6 g7] <- out

        // heavily relying on fact that descriptions arrays sorted by [type, index] asc
        int connects_num = 0;
        for (int i = 0, k = 0; i < inputs_num && k < outputs_num; /*nop*/) {
            const std::size_t type__in = inputs[i].type_id;
            const std::size_t type_out = outputs[k].type_id;

            if (inputs[i].aux || type__in < type_out) {
                i++;
                continue;
            }

            if (outputs[k].aux || type_out < type__in) {
                k++;
                continue;
            }

            const int  idx_in   = inputs[i].index;
            const int  idx_out  = outputs[k].index;
            const bool attached = node_dst->attach_any(node_src, idx_out, idx_in);

            if (!attached) {
                log->warn("Cannot connect nodes: [{}]: {} -> [{}]: {}",
                    outputs[k].name, outputs[k].index, inputs[i].name, inputs[i].index);
            } else {
                log->debug("Connected nodes: [{}]: {} -> [{}]: {}",
                    outputs[k].name, outputs[k].index, inputs[i].name, inputs[i].index);
            }

            connects_num += attached;

            i++;
            k++;
        }

        to->connected = (connects_num > 0);
        log->debug("CONNECTION STATUS: {} -> {} [{}]", from->id_local, to->id_local, to->connected);
    }

} // namespace ex::main
