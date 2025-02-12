//
// Created by xd on 12/02/25.
//

#include "../../headers/logic/logic_globals.h"
#include "../../headers/imgui/cmp/test_cmp.h"

namespace ex::logic::globals {

    State &get_state() {
        static State instance;
        return instance;
    }

    const std::map<id_type, ex::LogicNodeComponent> &node_components() {
        // TODO REPLACE WITH REAL COMPONENTS
        static const std::map<id_type, ex::LogicNodeComponent> instance = {
            // { cmp::CameraNode::get_node_id(), map::camera::mappers },
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
        return instance;
    }

}