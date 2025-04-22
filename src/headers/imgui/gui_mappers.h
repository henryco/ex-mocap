//
// Created by henryco on 25/02/25.
//

#ifndef GUI_MAPPERS_H
#define GUI_MAPPERS_H

#include "nodes/gui_nodes.h"
#include "../../extra/data/struct/array_map.h"

namespace ex::gui {
    using id_type  = unsigned long int;

    /**
    * Sum of node_outputs and node_components
    */
    const data::array_map<id_type, GuiNodeMapper> &node_mappers();

    namespace mappers {
        const data::array_map<id_type, GuiNodeMapper> &node_outputs();

        const data::array_map<id_type, GuiNodeMapper> &node_components();
    }

}

#endif //GUI_MAPPERS_H
