//
// Created by xd on 31/01/25.
//

#ifndef LOGIC_GLOBALS_H
#define LOGIC_GLOBALS_H

#include "../nodes/nodes_component.h"
#include <map>

namespace ex::logic::globals {

    const std::map<std::size_t, ex::BaseNodeComponent> &node_components();

}

#endif //LOGIC_GLOBALS_H
