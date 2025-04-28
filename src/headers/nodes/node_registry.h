//
// Created by xd on 4/28/25.
//

#ifndef EX_MOTION_NODE_REGISTRY_H
#define EX_MOTION_NODE_REGISTRY_H

#include "../../extra/data/struct/array_map.h"

#include "../logic/nodes/logic_nodes.h"
#include "../imgui/nodes/gui_nodes.h"

namespace ex::nodes {

    using id_type  = unsigned long int;

    struct Mapper {
        ex::gui::GuiNodeMapper gui;
        ex::logic::NodeMapper  logic;
    };

    namespace mappers {
        const data::array_map<id_type, Mapper> &node_outputs();

        const data::array_map<id_type, Mapper> &node_components();
    }

    template<typename T>
    id_type id_from_struct_() {
        return T::get_node_id();
    }

    template<typename T>
    ex::logic::NodeMapper logic_from_struct_() {
        return {
            .instance = &T::instance,
            .destroy  = &T::destroy
        };
    }
}

#endif //EX_MOTION_NODE_REGISTRY_H
