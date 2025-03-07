//
// Created by xd on 17/12/24.
//

#ifndef LOGIC_H
#define LOGIC_H

#include "../../../extra/graph/graph.h"
#include "../state.h"

namespace ex::main {

    struct Logic {
        limbo::graph::Clock clock_;

        Logic() = default;

        ~Logic() = default;

        void initialize(State& state);

        void run(State &state);

        void reset_internal_state_(State &state);

        void synchronize_(State &state);

        void process_(State &state);

        void rewire_(State &state);

        void deserialize_(State &state);

        void serialize_(State &state);

        void create_new_(State &state);

    };

    Logic create_logic(State& state);

}

#endif //LOGIC_H
