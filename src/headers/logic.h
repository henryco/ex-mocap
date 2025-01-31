//
// Created by xd on 17/12/24.
//

#ifndef LOGIC_H
#define LOGIC_H

#include <map>

#include "components/c_camera.h"
#include "imgui/cmp/test_cmp.h"
#include "nodes/nodes.h"
#include "nodes/nodes_component.h"
#include "state.h"
#include "utils/log_utils.h"

namespace ex::main {

    struct simple_list {
        TreeNode    *node = nullptr;
        simple_list *next = nullptr;

        ~simple_list() {
            // hopefully it will not end up with stack overflow :/
            if (next != nullptr) {
                delete next;
                next = nullptr;
            }
        }
    };

    using local_id_ = std::size_t;
    using type_id_  = std::size_t;

    class Logic {
    private:
        static inline const std::map<type_id_, BaseNodeComponent> mappers_ = {
//            { cmp::CameraNode::get_node_id(), map::camera::mappers },

            // TODO REMOVE
            { ex::gui::test::NodeOne::get_node_id(), ex::gui::test::one::data_mappers },
            { ex::gui::test::NodeTwo::get_node_id(), ex::gui::test::two::data_mappers },
            { ex::gui::test::NodeTri::get_node_id(), ex::gui::test::tri::data_mappers },
            { 0, ex::gui::test::one::data_mappers },
            { 1, ex::gui::test::tri::data_mappers },
            { 2, ex::gui::test::two::data_mappers },
            { 3, ex::gui::test::one::data_mappers },
            { 4, ex::gui::test::tri::data_mappers },
            { 5, ex::gui::test::two::data_mappers },
            { 6, ex::gui::test::one::data_mappers },
        };

        std::map<local_id_, nodes::BaseNode *> nodes_       = {};
        std::map<local_id_, int>               ptr_counter_ = {};

        long timestamp = 0;

        nodes::clock clock_;

    public:
        static inline const auto log = logs::get_logger("logic");

        Logic() = default;

        ~Logic() = default;

        void initialize(State& state);

        void run(State &state);

        void serialize(State &state);

    protected:
        void traverse_(State &state, TreeNode *node, bool *rewire, bool fresh);

        void synchronize_(State &state);

        void process_(State &state);

        void rewire_(State &state);

        void restore_(State &state);

        void build_tree_(State &state, TreeNode *node, TreeNode *previous, simple_list *group_list);

        void deserialize_(State &state);

        void gc_(State &state);

        void mark_node_deleted_(State &state, TreeNode *node, bool moved);

        bool connect_nodes_(TreeNode *from, TreeNode *to);
    };

    Logic create_logic(State& state);

}

#endif //LOGIC_H
