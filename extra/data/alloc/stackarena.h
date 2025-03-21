//
// Created by xd on 3/16/25.
//

#ifndef EX_LIMBO_DATA_STACK_ARENA_H
#define EX_LIMBO_DATA_STACK_ARENA_H

#include "datalloc.h"
#include "provider.h"

namespace ex::data::stackarena {

    template <std::size_t block_s, std::size_t arena_s, std::size_t stack_s = arena_s / block_s * sizeof(void *)>
    struct static_allocator {

        static constexpr std::size_t data_s = arena_s / block_s * block_s;
        static constexpr std::size_t free_s = stack_s / sizeof(void *);

        unsigned char arena_[data_s];
        void         *stack_[free_s];

        std::size_t offset;
        std::size_t free_n;

        ex::data::allocator compat() {
            return alloc_compat<static_allocator>::allocator(this);
        }

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

    struct allocator {

        static constexpr int chunk_size = 64;

        const std::size_t block_s;

        unsigned char * *arena_;
        void         ** *stack_;

        std::size_t data_s = 0;
        std::size_t free_s = 0;

        std::size_t offset = 0;
        std::size_t free_n = 0;

        mem_provider memory_;

        ex::data::allocator compat() {
            return alloc_compat<allocator>::allocator(this);
        }

        allocator(const std::size_t block_s) :
            block_s(block_s),
            arena_(nullptr),
            stack_(nullptr),
            data_s(block_s),
            free_s(1) {
            mem_request_();
        }

        allocator(const std::size_t block_s,
                  const std::size_t arena_s) :
            block_s(block_s),
            arena_(nullptr),
            stack_(nullptr) {
            data_s = arena_s / block_s * block_s;
            free_s = arena_s / block_s;
            mem_request_();
        }

        allocator(const std::size_t block_s,
                  const std::size_t arena_s,
                  const std::size_t stack_s) :
            block_s(block_s),
            arena_(nullptr),
            stack_(nullptr) {
            data_s = arena_s / block_s * block_s;
            free_s = stack_s / sizeof(void *);
            mem_request_();
        }

        allocator(const allocator &other) = delete;
        allocator(allocator &&other)      = delete;

        allocator &operator = (const allocator &other) = delete;
        allocator &operator = (allocator &&other)      = delete;

        ~allocator() {

        }

        /**
         * allocates only one block at a time
         */
        void *malloc(std::size_t, std::size_t) {

            return nullptr;
        }

        void *realloc(void *, const std::size_t) {
            abort_("Realloc not supported");
        }

        /**
         * releases block
         */
        void free(void *ptr) {
            if (free_n >= free_s)
                stack_request_();
            const int idx  = to_idx_(free_n);
            const int n    = to_n_(free_n);
            stack_[idx][n] = ptr;
            free_n++;
        }

        constexpr long size_total() const {
            return 0;
        }

        long size_used() const {
            return 0;
        }

        void mem_request_() {
            std::size_t allocated_size = 0;
            void       *allocated_ptr  = memory_.request(block_s * chunk_size, &allocated_size);

        }

        void stack_request_() {
            std::size_t allocated_size = 0;
            void       *allocated_ptr  = memory_.request(sizeof(void *) * chunk_size, &allocated_size);

        }

        static int to_idx_(const int n) {
            return n / chunk_size;
        }

        static int to_n_(const int n) {
            return n % chunk_size;
        }
    };


}

#endif //EX_LIMBO_DATA_STACK_ARENA_H
