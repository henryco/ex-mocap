//
// Created by henryco on 24/02/25.
//

#ifndef EX_MOTION_LOGIC_MAPPERS_H
#define EX_MOTION_LOGIC_MAPPERS_H

#include "../../extra/data/data_utils.h"
#include "nodes/logic_nodes.h"

namespace ex::logic {
    using id_type  = std::uint64_t;

    /**
     * Sum of node_outputs and node_components
     */
    const data::map<id_type, NodeMapper> &node_mappers();

    namespace mappers {
        const data::map<id_type, NodeMapper> &node_outputs();

        const data::map<id_type, NodeMapper> &node_components();
    }

}

#endif //EX_MOTION_LOGIC_MAPPERS_H
