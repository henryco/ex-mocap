//
// Created by henryco on 25/02/25.
//

#include "../../headers/imgui/gui_mappers.h"

namespace ex::gui {

    namespace internal__ {
        inline const data::map<id_type, GuiNodeMapper> &instance_mappers_map_() {
            static data::map<id_type, GuiNodeMapper> instance;

            static const auto &out_map = ex::gui::mappers::node_outputs();
            for (int i = 0; i < out_map.size; i++)
                instance.put(out_map.key_entries[i], out_map.val_entries[i]);

            static const auto &cmp_map = ex::gui::mappers::node_components();
            for (int i = 0; i < cmp_map.size; i++)
                instance.put(cmp_map.key_entries[i], cmp_map.val_entries[i]);

            return instance;
        }
    }

    const data::map<id_type, GuiNodeMapper> &node_mappers() {
        static const data::map<id_type, GuiNodeMapper> &instance = internal__::instance_mappers_map_();
        return instance;
    }

}
