//
// Created by xd on 2/24/25.
//

#include "../../../../headers/logic/logic_mappers.h"
#include "../../../../headers/logic/nodes/outputs/display.h"

namespace ex::logic::mappers {

    namespace internal_ {

        template<typename T>
        NodeMapper from_struct_() {
            return {
                .instance = &T::instance,
                .relocate = &T::relocate,
                .destroy  = &T::destroy
            };
        }

        const data::array_map<id_type, NodeMapper> &instance_out_map_() {
            static data::array_map<id_type, NodeMapper> instance;

            using display_t = ex::logic::node::out::DisplayOutputNode;
            instance.put(display_t::get_node_id(), from_struct_<display_t>());

            return instance;
        }
    }

    const data::array_map<id_type, NodeMapper> &node_outputs() {
        static const data::array_map<id_type, NodeMapper> &instance = internal_::instance_out_map_();
        return instance;
    }

}