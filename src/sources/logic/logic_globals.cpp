//
// Created by xd on 12/02/25.
//

#include "../../headers/logic/logic_globals.h"

namespace ex::logic::globals {

    State &state() {
        static State instance;
        return instance;
    }

    logs::logger &logger() {
        static ex::logs::logger log = ex::logs::get_logger("logic_global");
        return log;
    }

}