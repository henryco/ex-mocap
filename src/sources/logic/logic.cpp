//
// Created by xd on 17/12/24.
//

#include "../../headers/utils/time_utils.h"
#include "../../headers/utils/data_utils.h"
#include "../../headers/utils/file_utils.h"
#include "../../headers/logic/io/byte_serializer.h"
#include "../../headers/logic/logic_globals.h"
#include "../../headers/logic/logic_state.h"
#include "../../headers/logic/logic.h"

namespace internal_ {

    static ex::data::map<std::uint32_t, int> ptr_counter_map(10);

    void cleanup_nodes() {
        for (int i = 0; i < ptr_counter_map.size; /*nop*/) {
            const uint32_t key         = ptr_counter_map.key_entries[i];
            int           &ptr_counter = ptr_counter_map.val_entries[i];
            if (ptr_counter <= 0) {
                ex::logic::state::remove_node(key);
                ptr_counter_map.remove_at_pos(i);
            } else {
                ptr_counter = 0;
                ++i;
            }
        }
    }

    inline int *fetch_(const std::uint32_t &id) {
        return &ptr_counter_map[id];
    }

    static ex::data::cache_line cache = fetch_;
}

namespace ex::main {

    void abort_(const char *msg) {
        std::cerr << msg << std::endl;
        std::abort();
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

    void Logic::reset_internal_state_(State &state) {
        clock_.detach_all();

        if (state.logic.work_tree != nullptr)
            delete state.logic.work_tree;

        state::uuid::reset_last_id();

        state.logic.work_tree             = TreeNode::create();
        state.logic.work_tree->state      = node_state::is_group;
        state.logic.work_tree->name       = "root";
        state.logic.work_tree->children_n = 0;
    }

    Logic create_logic(State &state) {
        Logic logic;
        logic.initialize(state);
        return logic;
    }

    void Logic::initialize(State &state) {
        log->info("main logic initialization started");

        reset_internal_state_(state);

        log->info("main logic initialization synchronization");
        synchronize_(state);
        log->info("main logic initialization complete");
    }

    void Logic::run(State &state) {
        if (state.logic.work_tree == nullptr)
            abort_("Logic::run: work tree is null");

        if (state.project.brand_new) {
            state.project.brand_new = false;
        }

        if (state.project.should_create) {
            state.project.should_create = false;
            create_new_(state);
            return;
        }

        if (state.project.should_open) {
            state.project.should_open = false;
            deserialize_(state);
            process_(state);
            return;
        }

        synchronize_(state);
        process_(state);

        if (state.project.should_save) {
            state.project.should_save = false;
            serialize_(state);
        }
    }

    void Logic::create_new_(State &state) {
        log->info("create new project: {}", state.project.project_path);
        reset_internal_state_(state);
        synchronize_(state);
        serialize_(state);

        const auto str_pre        = std::string("ExMoCap - ");
        const auto str_post       = std::string(state.project.project_path);
        state.project.title       = (new std::string(str_pre + str_post))->c_str();
        state.project.brand_new   = true;
        state.project.has_changes = false;

        log->info("new project created: {}", state.project.project_path);
    }

    void Logic::serialize_(State &state) {
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

    void Logic::deserialize_(State &state) {
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

        if (state.logic.work_tree != nullptr)
            delete state.logic.work_tree;

        state.logic.work_tree     = new_state->logic.work_tree;
        state.logic.rewire        = true;

        delete new_state;
        delete[] data;

        synchronize_(state);

        const auto str_pre        = std::string("ExMoCap - ");
        const auto str_post       = std::string(state.project.project_path);
        state.project.title       = (new std::string(str_pre + str_post))->c_str();
        state.project.brand_new   = true;
        state.project.has_changes = false;

        log->info("deserialization complete: {}", state.project.project_path);
    }

    void Logic::process_(State &state) {
        static ex::time::measure_avg timing = 1000;
        if (state.metrics.timing_enabled)
            timing.start();

        clock_.next();

        if (state.metrics.timing_enabled && timing.stop())
            state.metrics.logic_duration_ns = timing.result();
    }

    void Logic::synchronize_(State &state) {

        static auto prev_target_fps = state.gl.target_fps;
        static auto fps_guard = ex::time::fps_guard::from_fps(state.gl.target_fps);
        if (prev_target_fps != state.gl.target_fps) {
            prev_target_fps = state.gl.target_fps;
            fps_guard.set_fps(state.gl.target_fps);
        }
        if (!fps_guard.step())
            return;

        bool rewire = state.logic.rewire;

        internal_::cache.begin();
        traverse_(state, state.logic.work_tree, &rewire, false);
        internal_::cache.end();

        static bool gc_run = false;

        if (gc_run && !rewire) {
            gc_(state);
            gc_run = false;
        }

        if (rewire) {
            rewire_(state);
            gc_run = true;

            state.project.has_changes = true;
            state.logic.dirty         = true;
        }

        state.logic.rewire = false;
    }

    void Logic::mark_node_deleted_(State &state, TreeNode *node, const bool moved) {
        if (node == nullptr)
            return;

        node->state -= node_state::deleted;
        node->state -= node_state::moved;

        node->state |= (node_state::deleted && !moved);
        node->state |= (node_state::moved && moved);

        if (!moved &&
            !(node->state >>= node_state::is_link) &&
            !(node->state >>= node_state::is_group)) {
            ex::logic::state::remove_node(node->id_local);
            internal_::ptr_counter_map.remove(node->id_local);
        }

        if (moved && !(node->state >>= node_state::is_link) && !(node->state >>= node_state::is_group)) {
            const auto base = ex::logic::state::get_node(node->id_local);
            const auto r_func = ex::logic::globals::node_components()[node->id_node].relocate;
            if (r_func != nullptr) {
                const auto new_one = r_func(node, base);
                if (base != new_one) {
                    ex::logic::state::replace_node(node->id_local, new_one);
                }
            }
        }

        if (node->children_n <= 0 || node->children == nullptr) {
            return;
        }

        for (int i = 0; i < node->children_n; i++) {
            mark_node_deleted_(state, node->children[i], moved);
        }
    }

    void Logic::traverse_(State &state, TreeNode *node, bool *rewire, const bool fresh) {
        if (node == nullptr)
            return;


        if ((node->state >>= node_state::deleted) || (node->state >>= node_state::moved)) {
            *rewire = true;
            return;
        }

        for (std::size_t i = 0; i < node->children_n; i++) {
            const auto child = node->children[i];

            if ((child->state >>= node_state::deleted) || (child->state >>= node_state::moved)) {
                *rewire = true;
            }

            else {
                traverse_(state, child, rewire, fresh || (node->state >>= node_state::created));
                if ((child->state >>= node_state::is_group) && (child->children_n <= 0 || child->children == nullptr)) {
                    child->state |= node_state::deleted;
                    *rewire = true;
                }
            }
        }

        int o = 0;
        while (true) {
            if (o >= node->children_n)
                break;
            const auto child = node->children[o];
            if (!(child->state >>= node_state::deleted) && !(child->state >>= node_state::moved)) {
                o += 1;
                continue;
            }
            mark_node_deleted_(state, child, child->state >>= node_state::moved);
            node->remove_node(child);
            delete child;
            o = 0;
        }

        if ((node->state >>= node_state::created) || fresh) {

            if (!(node->state >>= node_state::is_group) && !(node->state >>= node_state::is_link)) {

                const LogicNodeComponent &mappers = ex::logic::globals::node_components()[node->id_node];
                if (const auto node_ptr = ex::logic::state::get_node(node->id_local); node_ptr != nullptr) {
                    if (node_ptr->get_id() != node->id_local)
                        ex::logic::state::replace_node(node->id_local, mappers.instance(node));
                } else
                    ex::logic::state::store_node(node->id_local, mappers.instance(node));

                internal_::ptr_counter_map[node->id_local]++;
            }

            node->state -= node_state::created;
            *rewire = true;
        }

        if ((node->state >>= node_state::is_group) || (node->state >>= node_state::is_link))
            return;

//        int &v_ref = internal_::ptr_counter_map[node->id_local];    // 1
//        v_ref      = std::min(v_ref + 1, 10000000);

        int *v_ref = internal_::cache(node->id_local);              // 2
        *v_ref     = std::min(*v_ref + 1, 10000000);
    }

    void Logic::rewire_(State &state) {
        log->debug("rewire");

        clock_.detach_all();
        for (const auto &[_, node] : ex::logic::state::node_storage())
            node->detach_all_any();

        simple_list current = { state.logic.work_tree };
        build_tree_(state, state.logic.work_tree, nullptr, &current);

        gc_(state);

        const simple_list *head = get_head_(&current);
        if (head == nullptr) {
            return;
        }

        const TreeNode *node = head->node;
        if (node == nullptr) {
            return;
        }

        const auto base_node = ex::logic::state::get_node(node->id_local);
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

    void Logic::gc_(State &_) {
        log->debug("start node gc");
        internal_::cleanup_nodes();
        log->debug("complete node gc");
    }

    void Logic::build_tree_(State &state, TreeNode *node, TreeNode *previous, simple_list *group_list) {
        if (node == nullptr || group_list == nullptr || (node->state >>= node_state::deleted))
            return;

        if (!(node->state >>= node_state::is_link) && !(node->state >>= node_state::is_group)) {
            node->state -= node_state::connected;
        }

        if (node->state >>= node_state::is_group) {
            TreeNode *prev_node = previous;
            TreeNode *curr_node = nullptr;

            for (std::size_t i = 0; i < node->children_n; i++) {
                const auto child_node = node->children[i];

                if (child_node->state >>= node_state::disabled)
                    continue;

                curr_node = child_node;

                build_tree_(state, curr_node, prev_node, group_list);

                if (!(curr_node->state >>= node_state::is_group)) {
                    prev_node = curr_node;
                }
            }

            if (curr_node != nullptr && !(curr_node->state >>= node_state::is_group)) {
                // add last non-group element within group to list
                get_head_(group_list)->next = new simple_list(curr_node);
            }
            return;
        }

        connect_nodes_(previous, node);
        for (std::size_t i = 0; i < node->children_n; i++) {
            TreeNode *curr_node = node->children[i];

            // non-group element can contain only groups as children
            if (!(curr_node->state >>= node_state::is_group) || (curr_node->state >>= node_state::disabled))
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

    bool Logic::connect_nodes_(TreeNode *from, TreeNode *to) {
        if (from == nullptr || to == nullptr || (to->state >>= node_state::is_link))
            return false;

        to->state -= node_state::connected;

        const std::size_t from_id_local = (from->state >>= node_state::is_link) ? from->id_link : from->id_local;
        const auto node_src = ex::logic::state::get_node(from_id_local);

        if ((from->state >>= node_state::is_link) && node_src == nullptr) {
            log->warn("Cannot connect shared node, reference deleted: {}", from_id_local);
            from->state -= node_state::connected;
            return false;
        }

        const auto node_dst = ex::logic::state::get_node(to->id_local);
        if (node_dst == nullptr || node_src == nullptr) {
            log->error("One of nodes does not exist: {}, {}", to->id_local, from_id_local);
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

        to->state -= node_state::connected;
        to->state |= (node_state::connected && (connects_num > 0));
        log->debug("CONNECTION STATUS: {} -> {} [{}]", from_id_local, to->id_local, to->state >>= node_state::connected);

        return to->state >>= node_state::connected;
    }

}
