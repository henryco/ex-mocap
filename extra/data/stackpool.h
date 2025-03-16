//
// Created by xd on 3/16/25.
//

#ifndef EX_LIMBO_DATA_STACKPOOL_H
#define EX_LIMBO_DATA_STACKPOOL_H

#include <cstdlib>
#include <cstring>

namespace ex::data::stackpool {

    using watch_id_t = void *;
    using block_id_t = void *;

    template <std::size_t block_s, std::size_t pool_s, std::size_t stack_s>
    struct allocator {
        static constexpr std::size_t data_s = block_s * pool_s;
        static constexpr std::size_t meta_s = stack_s / sizeof(void *);

        unsigned char pool_[data_s];


    };

}

#endif //EX_LIMBO_DATA_STACKPOOL_H
