//
// Created by xd on 17/12/24.
//

#ifndef LOGIC_H
#define LOGIC_H

#include "state.h"
#include "utils/log_utils.h"

namespace ex::main {

    class Logic {
    private:
        static inline const auto log =
            logs::get_logger("logic");

    public:
        Logic() = default;

        ~Logic() = default;

        void initialize(State& state);

        void run(State &state);
    };

    Logic create_logic(State& state);

}

#endif //LOGIC_H
