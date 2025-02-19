//
// Created by xd on 2/19/25.
//

#include "../../../headers/logic/logic_globals.h"


// TODO REPLACE WITH REAL COMPONENTS
#include "../../../headers/imgui/components/test_cmp.h"
// TODO REPLACE WITH REAL COMPONENTS


namespace ex::logic::globals {

    const data::map<id_type, ex::LogicNodeComponent> &node_components() {
        static bool fist_time = true;
        static data::map<id_type, ex::LogicNodeComponent> instance;

        if (fist_time) {
            // TODO REPLACE WITH REAL COMPONENTS
            instance.put(ex::gui::test::NodeOne::get_node_id(), ex::gui::test::one::data_mappers);
            instance.put(ex::gui::test::NodeTwo::get_node_id(), ex::gui::test::two::data_mappers);
            instance.put(ex::gui::test::NodeTri::get_node_id(), ex::gui::test::tri::data_mappers);
            instance.put(0, ex::gui::test::one::data_mappers);
            instance.put(1, ex::gui::test::tri::data_mappers);
            instance.put(2, ex::gui::test::two::data_mappers);
            instance.put(3, ex::gui::test::one::data_mappers);
            instance.put(4, ex::gui::test::tri::data_mappers);
            instance.put(5, ex::gui::test::two::data_mappers);
            instance.put(6, ex::gui::test::one::data_mappers);
            // TODO REPLACE WITH REAL COMPONENTS
            fist_time = false;
        }

        return instance;
    }
}