//
// Created by xd on 17/12/24.
//

#include "../../extra/data/struct/cache.h"
#include "../../headers/logic/logic.h"
#include "../../headers/io/byte_serializer.h"
#include "../../headers/logic/logic_globals.h"
#include "../../headers/logic/logic_mappers.h"
#include "../../headers/logic/logic_state.h"
#include "../../headers/utils/file_utils.h"
#include "../../headers/utils/time_utils.h"
#include "../../headers/utils/log_utils.h"

#include <iostream>

namespace ex::main {
    const auto log = ex::logs::get_logger("logic");
}

namespace {

    struct simple_list {
        ex::TreeNode *node = nullptr;
        simple_list  *next = nullptr;

        ~simple_list() {
            // hopefully it will not end up with stack overflow :/
            if (next != nullptr) {
                delete next;
                next = nullptr;
            }
        }

        static simple_list *get_head_(simple_list *node) {
            if (node == nullptr)
                return nullptr;
            simple_list *head = node;
            while (true) {
                if (head->next == nullptr)
                    return head;
                head = head->next;
            }
        }
    };

    void abort__(const char *msg) {
        ex::main::log->error(msg);
        std::cerr << msg << std::endl;
        ::abort();
    }

    ex::data::array_map<std::uint32_t, int> ptr_counter_map(10);

    void cleanup_nodes() {
        ex::main::log->debug("start node gc");
        for (int i = 0; i < ptr_counter_map.size; /*nop*/) {
            const uint32_t key         = ptr_counter_map.key_entries[i];
            int           &ptr_counter = ptr_counter_map.val_entries[i];
            if (ptr_counter <= 0) {

                if (const auto base = ex::logic::state::get_node(key)) {
                    if (const auto d_func = ex::logic::node_mappers()[base->get_type_id()].destroy)
                        d_func(nullptr, base);
                }

                ex::logic::state::remove_node(key);
                ptr_counter_map.remove_at_pos(i);
            } else {
                ptr_counter = 0;
                ++i;
            }
        }
        ex::main::log->debug("complete node gc");
    }
}

namespace ex::main {

    Logic create_logic() {
        Logic logic;
        logic.initialize();
        return logic;
    }

    // == INTERNAL METHODS ==

    void rewire_(Logic &logic);

    void traverse_(TreeNode *node);

    void build_tree_(Logic &logic, TreeNode *node, TreeNode *previous, simple_list *group_list);

    bool connect_nodes_(TreeNode *from, TreeNode *to);

    void process_();

    void synchronize_(Logic &logic);

    void reset_internal_state_();

    bool attach_to_clock_(TreeNode *node);

    // == INTERNAL METHODS ==


    void Logic::initialize() {
        log->info("main logic initialization started");

        reset_internal_state_();

        log->info("main logic initialization synchronization");
        synchronize_(*this);
        log->info("main logic initialization complete");

        ex::entity::set_dirty(false);
    }

    void Logic::run() {
        static State &state = ex::logic::globals::state();

        if (state.logic.root_node != 0)
            abort__("Logic::run: Work tree root node id != 0");

        if (state.project.brand_new) {
            state.project.brand_new = false;
        }

        if (state.project.should_create) {
            state.project.should_create = false;
            create_new_();
            state.project.has_changes = true;
            return;
        }

        if (state.project.should_open) {
            state.project.should_open = false;
            deserialize_();
            process_();
            state.project.has_changes = true;
            return;
        }

        if (ex::entity::is_dirty()) {
            ex::entity::set_dirty(false);
            synchronize_(*this);
            state.project.has_changes = true;
        }

        process_();

        if (state.project.should_save) {
            state.project.should_save = false;
            serialize_();
            state.project.has_changes = true;
        }
    }

    void Logic::create_new_() {
        static State &state = ex::logic::globals::state();

        log->info("create new project: {}", state.project.project_path);
        reset_internal_state_();
        synchronize_(*this);
        serialize_();

        const auto str_pre        = std::string("ExMoCap - ");
        const auto str_post       = std::string(state.project.project_path);
        state.project.title       = (new std::string(str_pre + str_post))->c_str();
        state.project.brand_new   = true;
        state.project.has_changes = false;

        log->info("new project created: {}", state.project.project_path);
    }

    void Logic::serialize_() {
        static State &state = ex::logic::globals::state();

        log->info("serialization start: {}", state.project.project_path);
        auto bytes = io::bytes::ByteSerializer().serialize(&state);
        ex::files::write_binary_file(state.project.project_path, bytes.data, bytes.size);
        io::byte_array::free(bytes);

        const auto str_pre        = std::string("ExMoCap - ");
        const auto str_post       = std::string(state.project.project_path);
        state.project.title       = (new std::string(str_pre + str_post))->c_str();
        state.project.brand_new   = false;
        state.project.has_changes = false;

        log->info("serialization complete: {}", state.project.project_path);
    }

    void Logic::deserialize_() {
        reset_internal_state_();

        static State &state = ex::logic::globals::state();

        log->info("deserialization start: {}", state.project.project_path);

        std::size_t size;
        const auto data = ex::files::read_binary_file(state.project.project_path, &size);

        const auto new_state =
                static_cast<State *>(io::bytes::ByteSerializer().deserialize(io::byte_array::from(data, size)));

        if (new_state->error.code != 0) {
            state.error.code = new_state->error.code;
            state.error.msg  = new_state->error.msg;
            delete new_state;
            delete[] data;

            log->error("deserialization failure: {}", state.project.project_path);
            return;
        }

        delete new_state;
        delete[] data;

        synchronize_(*this);

        const auto str_pre        = std::string("ExMoCap - ");
        const auto str_post       = std::string(state.project.project_path);
        state.project.title       = (new std::string(str_pre + str_post))->c_str();
        state.project.brand_new   = true;
        state.project.has_changes = false;

        log->info("deserialization complete: {}", state.project.project_path);
    }

    void rewire_(Logic &logic) {
        static State &state = ex::logic::globals::state();
        static limbo::graph::Clock &clock = ex::logic::state::get_clock();

        log->debug("rewire");

        clock.reset();
        for (const auto &node : ex::logic::state::node_storage())
            node.val->detach_all();

        TreeNode *root_node = ex::entity::find(state.logic.root_node);
        simple_list current = { root_node };
        build_tree_(logic, root_node, nullptr, &current);

        if (const simple_list *head = simple_list::get_head_(&current)) {
            if (root_node != head->node)
                attach_to_clock_(head->node);
        }
    }

    void reset_internal_state_() {
        static State        &state = ex::logic::globals::state();
        limbo::graph::Clock &clock = ex::logic::state::get_clock();
        clock.reset();

        for (const auto &node : ex::logic::state::node_storage())
            node.val->detach_all();
        cleanup_nodes();

        ex::entity::reset_entities();
        TreeNode *root = ex::entity::create(0);
        root->state    = node_state::group;
        root->name     = "root";
        root->id_type  = 0;

        state.logic.root_node   = root->id_node;
        state.logic.active_node = ID32_UNSET;
    }

    void process_() {
        static State &state = ex::logic::globals::state();
        static limbo::graph::Clock &clock = ex::logic::state::get_clock();

        static ex::time::measure_avg timing = { 10000, 100 };
        if (state.metrics.timing_enabled)
            timing.start();

        clock.pull();

        if (state.metrics.timing_enabled && timing.stop())
            state.metrics.logic_duration_ns = timing.result();
    }

    void synchronize_(Logic &logic) {
        static State &state = ex::logic::globals::state();
        TreeNode *root_node = ex::entity::find(state.logic.root_node);

        traverse_(root_node);
        rewire_(logic);
        cleanup_nodes();
    }

    bool connect_nodes_(TreeNode *from, TreeNode *to) {
        if (from == nullptr || to == nullptr || (to->state >>= node_state::link))
            return false;

        to->state -= node_state::connected;

        const std::size_t from_id_local = (from->state >>= node_state::link) ? from->id_link : from->id_node;
        const auto node_src = ex::logic::state::get_node(from_id_local);

        if (from->state >>= node_state::link) {
            if (node_src == nullptr) {
                log->warn("Cannot connect shared node, reference deleted: {}", from_id_local);
                from->state -= node_state::connected;
                return false;
            }

            TreeNode *real_node = ex::entity::find(from->id_link);
            if (real_node == nullptr ||
                (real_node->state >>= node_state::disabled) ||
                (real_node->state >>= node_state::deleted)) {
                log->warn("Cannot connect shared node, reference disabled or deleted: {}", from_id_local);
                from->state -= node_state::connected;
                return false;
            }
        }

        const auto node_dst = ex::logic::state::get_node(to->id_node);
        if (node_dst == nullptr || node_src == nullptr) {
            log->error("One of nodes does not exist: {}, {}", to->id_node, from_id_local);
            return false;
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
            const bool attached = node_dst->attach(node_src, idx_in, idx_out);

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

        to->state -= node_state::connected;
        to->state |= (node_state::connected && (connects_num > 0));
        log->debug("CONNECTION STATUS: {} -> {} [{}]", from_id_local, to->id_node, to->state >>= node_state::connected);

        return to->state >>= node_state::connected;
    }

    void traverse_(TreeNode *node) {
        if (node == nullptr)
            return;

        if (node->state >>= node_state::deleted)
            return;

        bool should_skip = false;

        if (node->state >>= node_state::link) {
            should_skip = true;
        }

        if (node->state >>= node_state::group) {
            should_skip = true;
        }

        if (!should_skip) {
            const logic::NodeMapper &mappers = ex::logic::node_mappers()[node->id_type];
            if (const auto node_ptr = ex::logic::state::get_node(node->id_node)) {
                if (node_ptr->get_node_id() != node->id_node) {
                    mappers.destroy(node, node_ptr);
                    ex::logic::state::replace_node(node->id_node, mappers.instance(node));
                }
            } else {
                ex::logic::state::store_node(node->id_node, mappers.instance(node));
            }

            ptr_counter_map[node->id_node]++;
        }

        const auto children = ex::entity::children(node->id_node);
        for (int i = 0; i < children.size; ++i) {
            traverse_(ex::entity::find(children.data[i]));
        }
    }

    bool attach_to_clock_(TreeNode *node) {
        static limbo::graph::Clock &clock = ex::logic::state::get_clock();

        if (node == nullptr) {
            return false;
        }

        const auto base_node = ex::logic::state::get_node(node->id_node);
        if (base_node == nullptr) {
            log->error("Node is nullptr: {}", node->id_node);
            return false;
        }

        int outputs_num = 0;
        const auto outputs = base_node->describe_outputs(&outputs_num);

        for (std::size_t i = 0; i < outputs_num; i++) {
            const auto &output = outputs[i];
            if (output.aux)
                continue;

            if (clock.attach(base_node)) {
                log->debug("Node {} [{}][{}] (main) attached to clock",
                    base_node->get_type_name(), node->id_node, output.index);
                return true;
            }
        }

        for (std::size_t i = 0; i < outputs_num; i++) {
            const auto &output = outputs[i];
            if (!output.aux)
                continue;

            if (clock.attach(base_node)) {
                log->debug("Node {} [{}][{}] (aux) attached to clock",
                    base_node->get_type_name(), node->id_node, output.index);
                return true;
            }
        }

        log->error("Cannot connect node to clock");
        return false;
    }

    void build_tree_(Logic &logic, TreeNode *node, TreeNode *previous, simple_list *group_list) {
        static limbo::graph::Clock &clock = ex::logic::state::get_clock();

        if (node == nullptr || group_list == nullptr)
            return;

        if (node->state >>= node_state::deleted)
            return;

        if (node->state >>= node_state::disabled) {
            return;
        }

        if (!(node->state >>= node_state::link) && !(node->state >>= node_state::group)) {
            node->state -= node_state::connected;
        }

        if (node->state >>= node_state::group) {
            TreeNode *prev_node = nullptr;
            TreeNode *curr_node = nullptr;
            TreeNode *last_node = nullptr;

            if (!(node->state >>= node_state::section)) {
                prev_node = previous; // sections are isolated
            }

            const auto children = ex::entity::children(node->id_node);
            for (std::size_t i = 0; i < children.size; i++) {
                const auto child_node = ex::entity::find(children.data[i]);

                if (child_node->state >>= node_state::disabled)
                    continue;

                curr_node = child_node;

                build_tree_(logic, curr_node, prev_node, group_list);

                if (!(curr_node->state >>= node_state::group) &&
                    !(curr_node->state >>= node_state::output))
                {
                    prev_node = curr_node;
                }

                if (!(curr_node->state >>= node_state::output)) {
                    last_node = curr_node;
                }
            }

            if (node->state >>= node_state::section) {
                return; // sections are isolated
            }

            if (last_node != nullptr &&
                !(last_node->state >>= node_state::group) &&
                !(last_node->state >>= node_state::group))
            {
                // add last non-group element within group to list
                simple_list::get_head_(group_list)->next = new simple_list(last_node);
            }
            return;
        }

        connect_nodes_(previous, node);

        if (node->state >>= node_state::output) {

            if (const auto base_node = ex::logic::state::get_node(node->id_node); base_node == nullptr) {
                log->error("Node is nullptr: {}", node->id_node);
            } else {
                if (clock.attach(base_node)) {
                    log->debug("Node {} [{}] attached to clock",
                        base_node->get_type_name(), node->id_node);
                }
            }
        }

        const auto children = ex::entity::children(node->id_node);
        for (std::size_t i = 0; i < children.size; i++) {
            TreeNode *curr_node = ex::entity::find(children.data[i]);

            // non-group element can contain only groups as children
            if (!(curr_node->state >>= node_state::group) || (curr_node->state >>= node_state::disabled))
                continue;

            simple_list list = {};
            build_tree_(logic, curr_node, nullptr, &list);

            const simple_list *head = list.next;
            while (head != nullptr) {
                // collect results of the group traversal
                connect_nodes_(head->node, node);
                head = head->next;
            }
        }
    }

}
