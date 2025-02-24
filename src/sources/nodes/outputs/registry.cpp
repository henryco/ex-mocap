//
// Created by xd on 2/24/25.
//

#include "../../../headers/logic/logic_mappers.h"

namespace ex::logic::mappers {

    namespace internal_ {
        inline const data::map<id_type, ex::NodeMapper> &instance_out_map_() {
            static data::map<id_type, ex::NodeMapper> instance;
            // TODO
            // instance.put(0, ex::gui::test::one::data_mappers);
            // TODO
            return instance;
        }
    }

    const data::map<id_type, ex::NodeMapper> &node_outputs() {
        static const data::map<id_type, ex::NodeMapper> &instance = internal_::instance_out_map_();
        return instance;
    }

}