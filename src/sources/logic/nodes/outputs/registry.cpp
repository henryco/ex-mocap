//
// Created by xd on 2/24/25.
//

#include "../../../../headers/logic/logic_mappers.h"
#include "../../../../headers/logic/nodes/outputs/display.h"

namespace ex::logic::mappers {

    namespace internal_ {

        inline NodeMapper verify_(const NodeMapper &mapper) {
            if (mapper.instance == nullptr)
                abort_("mapper.instance == nullptr");
            if (mapper.relocate == nullptr)
                abort_("mapper.relocate == nullptr");
            if (mapper.destroy == nullptr)
                abort_("mapper.destroy == nullptr");
            return mapper;
        }

        const data::array_map<id_type, NodeMapper> &instance_out_map_() {
            static data::array_map<id_type, NodeMapper> instance;

            using display_t = ex::logic::node::out::DisplayOutputNode;
            instance.put(display_t::get_node_id(), verify_({ display_t::instance, display_t::relocate, display_t::destroy }));

            return instance;
        }
    }

    const data::array_map<id_type, NodeMapper> &node_outputs() {
        static const data::array_map<id_type, NodeMapper> &instance = internal_::instance_out_map_();
        return instance;
    }

}