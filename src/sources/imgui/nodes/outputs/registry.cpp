//
// Created by xd on 2/24/25.
//

#include "../../../../headers/imgui/nodes/outputs/display.h"
#include "../../../../headers/imgui/gui_globals.h"
#include "../../../../headers/imgui/gui_mappers.h"

namespace ex::gui::mappers {

    namespace internal_ {
        inline const data::map<id_type, GuiNodeMapper> &instance_out_map_() {
            static data::map<id_type, GuiNodeMapper> instance;

            namespace display_n = ex::gui::node::out::display;
            instance.put(display_n::get_node_type_id(), display_n::gui_mappers);

            return instance;
        }
    }

    const data::map<id_type, GuiNodeMapper> &node_outputs() {
        static const data::map<id_type, GuiNodeMapper> &instance = internal_::instance_out_map_();
        return instance;
    }
}