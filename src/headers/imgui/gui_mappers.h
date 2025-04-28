//
// Created by henryco on 25/02/25.
//

#ifndef EX_MOTION_GUI_MAPPERS_H
#define EX_MOTION_GUI_MAPPERS_H

#include "../../extra/data/struct/array_map.h"
#include "gui_nodes.h"

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

#endif //EX_MOTION_GUI_MAPPERS_H
