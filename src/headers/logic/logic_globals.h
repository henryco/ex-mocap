//
// Created by xd on 12/02/25.
//

#ifndef LOGIC_GLOBALS_H
#define LOGIC_GLOBALS_H

#include <map>
#include "logic_component.h"
#include "../state.h"

namespace ex::logic::globals {

    using id_type  = std::uint64_t;
    using id_local = std::uint32_t;

    State &get_state();

    const std::map<id_type, ex::LogicNodeComponent> &node_components();

}

#endif //LOGIC_GLOBALS_H
