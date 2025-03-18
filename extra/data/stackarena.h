//
// Created by xd on 3/16/25.
//

#ifndef EX_LIMBO_DATA_STACK_ARENA_H
#define EX_LIMBO_DATA_STACK_ARENA_H

#include <cstdlib>
#include <iostream>

namespace ex::data::stackarena {

#ifndef UNIT_TEST
    #define abort_(msg) do { std::cerr << msg << std::endl; ::abort(); } while(false)
#else
    #define abort_(msg) throw std::runtime_error("Generic abort exception")
#endif

    template <std::size_t block_s, std::size_t arena_s, std::size_t stack_s = arena_s / block_s * sizeof(void *)>
    struct static_allocator {

        static constexpr std::size_t data_s = arena_s / block_s * block_s;
        static constexpr std::size_t free_s = stack_s / sizeof(void *);

        unsigned char arena_[data_s];
        void         *stack_[free_s];

        std::size_t offset = 0;
        std::size_t free_n = 0;

        /**
         * allocates only one block at a time
         */
        void *allocate() {
            if (free_n > 0)
                return stack_[--free_n];
            if (offset >= data_s)
                abort_("out of memory: " << offset << "b");
            void *ptr = static_cast<void *>(arena_ + offset);
            offset += block_s;
            return ptr;
        }

        /**
         * releases block
         */
        void free(void *ptr) {
            const unsigned char *have = static_cast<unsigned char *>(ptr) + block_s;
            const unsigned char *last = arena_ + offset;

            if (have == last) {
                offset -= block_s;
                return;
            }

            if (free_n < free_s)
                stack_[free_n++] = ptr;
        }

        constexpr long size_total() const {
            return static_cast<long>(((arena_s / block_s) * block_s) + stack_s);
        }

        long size_used() const {
            return static_cast<long>(offset + (free_n * sizeof(void *)));
        }

        void clean() {
            offset = 0;
            free_n = 0;
        }
    };

    // TODO: add dynamic allocation

}

#endif //EX_LIMBO_DATA_STACK_ARENA_H
