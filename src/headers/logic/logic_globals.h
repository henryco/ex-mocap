//
// Created by xd on 12/02/25.
//

#ifndef LOGIC_GLOBALS_H
#define LOGIC_GLOBALS_H

#include "../utils/log_utils.h"
#include "../state.h"

namespace ex::logic::globals {

    using id_type = unsigned long int;

    State &state();

    logs::logger &logger();

}

#endif //LOGIC_GLOBALS_H
