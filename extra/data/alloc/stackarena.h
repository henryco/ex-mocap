//
// Created by xd on 3/16/25.
//

#ifndef EX_LIMBO_DATA_STACK_ARENA_H
#define EX_LIMBO_DATA_STACK_ARENA_H

#include "datalloc.h"

namespace ex::data::stackarena {

    template <std::size_t block_s, std::size_t arena_s, std::size_t stack_s = arena_s / block_s * sizeof(void *)>
    struct static_allocator {

        static constexpr std::size_t data_s = arena_s / block_s * block_s;
        static constexpr std::size_t free_s = stack_s / sizeof(void *);

        unsigned char arena_[data_s];
        void         *stack_[free_s];

        std::size_t offset;
        std::size_t free_n;

        // ReSharper disable once CppPossiblyUninitializedMember
        static_allocator() : offset(0), free_n(0) { // NOLINT(*-pro-type-member-init)
        }

        static_allocator(const static_allocator &other) = delete;
        static_allocator(static_allocator &&other)      = delete;

        static_allocator &operator = (const static_allocator &other) = delete;
        static_allocator &operator = (static_allocator &&other)      = delete;

        /**
         * allocates only one block at a time
         */
        void *malloc(std::size_t, std::size_t) {
            if (free_n > 0)
                return stack_[--free_n];
            if (offset >= data_s)
                abort_("out of memory: " << offset << "b");
            void *ptr = static_cast<void *>(arena_ + offset);
            offset += block_s;
            return ptr;
        }

        void *realloc(void *, const std::size_t) {
            abort_("Realloc not supported");
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
