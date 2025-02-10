//
// Created by xd on 31/01/25.
//

#ifndef LOGIC_GLOBALS_H
#define LOGIC_GLOBALS_H

#include <map>
#include "logic_component.h"

namespace ex::logic::globals {

    const std::map<std::uint64_t, ex::LogicNodeComponent> &node_components();

    std::map<std::uint32_t, limbo::nodes::BaseNode *> &node_store();

}

#endif //LOGIC_GLOBALS_H
