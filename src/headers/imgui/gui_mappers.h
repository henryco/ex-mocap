//
// Created by henryco on 25/02/25.
//

#ifndef GUI_MAPPERS_H
#define GUI_MAPPERS_H

#include "nodes/gui_nodes.h"
#include "../../extra/data/data_utils.h"

namespace ex::gui {
    using id_type  = std::uint64_t;

    /**
    * Sum of node_outputs and node_components
    */
    const data::map<id_type, GuiNodeMapper> &node_mappers();

    namespace mappers {
        const data::map<id_type, GuiNodeMapper> &node_outputs();

        const data::map<id_type, GuiNodeMapper> &node_components();
    }

}

#endif //GUI_MAPPERS_H
