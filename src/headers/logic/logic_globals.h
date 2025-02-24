//
// Created by xd on 12/02/25.
//

#ifndef LOGIC_GLOBALS_H
#define LOGIC_GLOBALS_H

#include "logic_component.h"
#include "../utils/data_utils.h"
#include "../utils/log_utils.h"
#include "../state.h"

namespace ex::logic::globals {

    using id_type  = std::uint64_t;

    State &state();

    logs::logger &logger();

    const data::map<id_type, ex::LogicNodeComponent> &node_outputs();

    /**
     * Offset by N node_outputs
     */
    const data::map<id_type, ex::LogicNodeComponent> &node_components();

}

#endif //LOGIC_GLOBALS_H
