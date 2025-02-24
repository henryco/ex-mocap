//
// Created by xd on 2/24/25.
//

#include "../../../../headers/imgui/gui_globals.h"
#include "../../../../headers/imgui/gui_mappers.h"

namespace ex::gui::mappers {

    namespace internal_ {
        inline const data::map<id_type, GuiNodeMapper> &instance_out_map_() {
            static data::map<id_type, GuiNodeMapper> instance;
            // TODO
            // instance.put(0, ex::gui::test::one::gui_mappers);
            // TODO
            return instance;
        }
    }

    const data::map<id_type, GuiNodeMapper> &node_outputs() {
        static const data::map<id_type, GuiNodeMapper> &instance = internal_::instance_out_map_();
        return instance;
    }
}