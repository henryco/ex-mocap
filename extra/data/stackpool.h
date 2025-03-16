//
// Created by xd on 3/16/25.
//

#ifndef EX_LIMBO_DATA_STACKPOOL_H
#define EX_LIMBO_DATA_STACKPOOL_H

#include <cstdlib>

namespace ex::data::stackpool {

    template <std::size_t block_s, std::size_t pool_s, std::size_t stack_s>
    struct allocator {

        static constexpr std::size_t data_s = pool_s / block_s;
        static constexpr std::size_t free_s = stack_s / sizeof(void *);

        unsigned char stack_[data_s];
        void         *freed_[free_s];

        std::size_t offset = 0;
        std::size_t free_n = 0;

        /**
         * allocates only one block at a time
         */
        void *allocate() {
            if (free_n > 0)
                return freed_[--free_n];
            if (offset >= pool_s)
                ::abort();
            void *ptr = static_cast<void *>(stack_ + offset);
            offset += block_s;
            return ptr;
        }

        /**
         * releases block
         */
        void free(void *ptr) {
            if (free_n < free_s)
                freed_[free_n++] = ptr;
        }
    };

}

#endif //EX_LIMBO_DATA_STACKPOOL_H
