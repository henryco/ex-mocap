//
// Created by xd on 17/12/24.
//

#ifndef EX_MO_LOGIC_H
#define EX_MO_LOGIC_H

namespace ex::main {

    struct Logic {

        Logic() = default;

        ~Logic();

        void initialize();

        void run();

        void deserialize_();

        void serialize_();

        void create_new_();
    };

    Logic create_logic();

}

#endif //EX_MO_LOGIC_H
