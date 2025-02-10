//
// Created by xd on 31/01/25.
//

#ifndef LOGIC_GLOBALS_H
#define LOGIC_GLOBALS_H

#include <map>
#include "logic_component.h"

namespace ex::logic::globals {

    const std::map<std::size_t, ex::LogicNodeComponent> &node_components();

}

#endif //LOGIC_GLOBALS_H
