//
// Created by xd on 3/16/25.
//

#ifndef EX_LIMBO_DATA_STACKPOOL_H
#define EX_LIMBO_DATA_STACKPOOL_H

#include <iostream>
#include <cstdlib>

namespace ex::data::stackpool {

#ifndef UNIT_TEST
    #define abort_(msg) do { std::cerr << msg << std::endl; ::abort(); } while(false)
#else
    #define abort_(msg) throw std::runtime_error("Generic abort exception")
#endif

    template <std::size_t block_s, std::size_t pool_s, std::size_t stack_s>
    struct allocator {

        static constexpr std::size_t data_s = (pool_s / block_s) * block_s;
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
            if (offset >= data_s)
                abort_("out of memory: " << offset << "b");
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

        std::size_t size_total() const {
            static std::size_t total_= data_s + stack_s;
            return total_;
        }

        std::size_t size_used() const {
            return offset + (freed_ * sizeof(void *));
        }

        void clean() {
            offset = 0;
            free_n = 0;
        }
    };

}

#endif //EX_LIMBO_DATA_STACKPOOL_H
