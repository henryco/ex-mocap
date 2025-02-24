//
// Created by henryco on 24/02/25.
//

#include "../../headers/logic/logic_mappers.h"

namespace ex::logic {

    namespace internal__ {
        inline const data::map<id_type, ex::LogicNodeComponent> &instance_mappers_map_() {
            static data::map<id_type, ex::LogicNodeComponent> instance;

            static const auto &out_map = ex::logic::mappers::node_outputs();
            for (int i = 0; i < out_map.size; i++)
                instance.put(out_map.key_entries[i], out_map.val_entries[i]);

            static const auto &cmp_map = ex::logic::mappers::node_components();
            for (int i = 0; i < cmp_map.size; i++)
                instance.put(cmp_map.key_entries[i], cmp_map.val_entries[i]);

            return instance;
        }
    }

    const data::map<id_type, ex::LogicNodeComponent> &node_mappers() {
        static const data::map<id_type, ex::LogicNodeComponent> &instance = internal__::instance_mappers_map_();
        return instance;
    }

}