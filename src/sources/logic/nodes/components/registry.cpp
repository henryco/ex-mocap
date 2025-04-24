//
// Created by xd on 2/19/25.
//

#include "../../../../headers/logic/logic_mappers.h"

// TODO REPLACE WITH REAL COMPONENTS
#include "../../../../headers/imgui/nodes/components/test_cmp.h"
// TODO REPLACE WITH REAL COMPONENTS

#include "../../../../headers/logic/nodes/components/image.h"

namespace ex::logic::mappers {

    namespace internal_ {

        template<typename T>
        NodeMapper from_struct_() {
            return {
                .instance = &T::instance,
                .destroy  = &T::destroy
            };
        }

        inline NodeMapper verify_(const NodeMapper &mapper) {
            if (mapper.instance == nullptr)
                abort_("mapper.instance == nullptr");
            if (mapper.destroy == nullptr)
                abort_("mapper.destroy == nullptr");
            return mapper;
        }

        const data::array_map<id_type, NodeMapper> &instance_cmp_map_() {
            static data::array_map<id_type, NodeMapper> instance;

            using image_node_t = ex::logic::node::cmp::image::ImageNode;
            instance.put(image_node_t::get_node_id(), from_struct_<image_node_t>());

            // TODO REPLACE WITH REAL COMPONENTS
            instance.put(ex::gui::test::NodeOne::get_node_id(), verify_(ex::gui::test::one::data_mappers));
            instance.put(ex::gui::test::NodeTwo::get_node_id(), verify_(ex::gui::test::two::data_mappers));
            instance.put(ex::gui::test::NodeTri::get_node_id(), verify_(ex::gui::test::tri::data_mappers));
            instance.put(0, verify_(ex::gui::test::one::data_mappers));
            instance.put(1, verify_(ex::gui::test::tri::data_mappers));
            instance.put(2, verify_(ex::gui::test::two::data_mappers));
            instance.put(3, verify_(ex::gui::test::one::data_mappers));
            instance.put(4, verify_(ex::gui::test::tri::data_mappers));
            instance.put(5, verify_(ex::gui::test::two::data_mappers));
            instance.put(6, verify_(ex::gui::test::one::data_mappers));
            // TODO REPLACE WITH REAL COMPONENTS
            return instance;
        }
    }

    const data::array_map<id_type, NodeMapper> &node_components() {
        static const data::array_map<id_type, NodeMapper> &instance = internal_::instance_cmp_map_();
        return instance;
    }
}