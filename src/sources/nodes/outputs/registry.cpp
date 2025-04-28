//
// Created by xd on 4/28/25.
//

#include "../../../headers/nodes/node_registry.h"

#include "../../../headers/nodes/outputs/display/display_gui.h"
#include "../../../headers/nodes/outputs/display/display_logic.h"

namespace ex::nodes::mappers {

    const data::array_map<id_type, Mapper> &out_registry_() {
        static data::array_map<id_type, Mapper> registry;

        { // Display output
            namespace display_n = ex::gui::node::out::display;
            using display_t     = ex::logic::node::out::DisplayOutputNode;
            registry.put(id_from_struct_<display_t>(), {
                .gui = display_n::gui_mappers,
                .logic = logic_from_struct_<display_t>()
            });
        }

        return registry;
    }

    const data::array_map<id_type, Mapper> &node_outputs() {
        static const data::array_map<id_type, Mapper> &instance = out_registry_();
        return instance;
    }
}