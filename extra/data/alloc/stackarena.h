//
// Created by xd on 3/16/25.
//

#ifndef EX_LIMBO_DATA_STACK_ARENA_H
#define EX_LIMBO_DATA_STACK_ARENA_H

#include <cstdlib>
#include <iostream>
#include "datalloc.h"

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

        // There is some compatibility layer type/mem-layout tricknology

        struct tricklocator {
            malloc_fn_t       malloc;
            realloc_fn_t      realloc;
            free_fn_t         free;
            reg_fn_t          add_watcher;
            reg_fn_t          del_watcher;
            size_fn_t         size_total;
            size_fn_t         size_used;
            static_allocator *instance;
        };

        const ex::data::allocator tricklocator_ = { .malloc      = &static_allocator::malloc_,
                                                    .realloc     = &static_allocator::realloc_,
                                                    .free        = &static_allocator::free_,
                                                    .add_watcher = &static_allocator::nop_,
                                                    .del_watcher = &static_allocator::nop_,
                                                    .size_total  = &static_allocator::size_total_,
                                                    .size_used   = &static_allocator::size_used_,
                                                    .instance    = this };

        const ex::data::allocator &compat_allocator() const {
            return tricklocator_;
        }

        static void *malloc_(void *this_, std::size_t, std::size_t) {
            return static_cast<const tricklocator *>(this_)->instance->allocate();
        }

        static void *realloc_(void *, void *, const std::size_t) {
            abort_("Realloc not supported");
        }

        static void free_(void *this_, void *ptr) {
            static_cast<const tricklocator *>(this_)->instance->free(ptr);
        }

        static void nop_(void *, void *const, const ex::data::watch_fn_t) {
        }

        static long size_total_(void *this_) {
            return static_cast<const tricklocator *>(this_)->instance->data_s;
        }

        static long size_used_(void *this_) {
            return static_cast<const tricklocator *>(this_)->instance->offset;
        }
    };

    // TODO: add dynamic allocation

}

#endif //EX_LIMBO_DATA_STACK_ARENA_H
