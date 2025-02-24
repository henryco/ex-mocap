//
// Created by henryco on 24/02/25.
//

#ifndef EX_MOTION_LOGIC_MAPPERS_H
#define EX_MOTION_LOGIC_MAPPERS_H

#include "nodes.h"
#include "../utils/data_utils.h"

namespace ex::logic {
    using id_type  = std::uint64_t;

    /**
     * Sum of node_outputs and node_components
     */
    const data::map<id_type, ex::NodeMapper> &node_mappers();

    namespace mappers {
        const data::map<id_type, ex::NodeMapper> &node_outputs();

        const data::map<id_type, ex::NodeMapper> &node_components();
    }

}

#endif //EX_MOTION_LOGIC_MAPPERS_H
