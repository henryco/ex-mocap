//
// Created by xd on 31/01/25.
//

#ifndef LOGIC_GLOBALS_H
#define LOGIC_GLOBALS_H

#include <map>
#include "logic_component.h"

namespace ex::logic::globals {

    using id_type = std::uint64_t;
    using id_node = std::uint32_t;

    const std::map<id_type, ex::LogicNodeComponent> &node_components();
    std::map<id_node, limbo::nodes::BaseNode *>     &node_store();

}

#endif //LOGIC_GLOBALS_H
