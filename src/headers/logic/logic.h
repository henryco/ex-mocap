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

        void initialize();

        void run();

        void deserialize_();

        void serialize_();

        void create_new_();
    };

    Logic create_logic();

}

#endif //LOGIC_H
