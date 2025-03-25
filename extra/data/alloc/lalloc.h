//
// Created by henryco on 24/03/25.
//

#ifndef EX_LIMBO_DATA_LALLOC_H
#define EX_LIMBO_DATA_LALLOC_H

#include "../struct/list_map.h"
#include "stackarena.h"

namespace ex::data::lalloc {

// -----------------------------
#define ALLOC_DEF_SIZE_B (1 MB)
#define STACK_ARENA_SIZE (1 MB)
// -----------------------------

    struct memory_block {
        u_int32_t size_b;
        u_int16_t alignment_b;
        u_int8_t  is_free;
    };

    using block_addr_t = void *;
    using block_size_t = u_int32_t;
    using block_ref_t  = memory_block *;
    using block_map_t  = list_map<block_addr_t, memory_block>;
    using block_free_t = list_map<block_size_t, block_ref_t>;

#define BLOCK_MAX_SIZE_B (4294967295U)
#define STACK_BLOCK_SIZE (sizeof(block_map_t::list_node))

    struct allocator {

        static int8_t ptr_compare(const block_addr_t *a, const block_addr_t *b) {
            const u_int64_t aa = reinterpret_cast<u_int64_t>(*a);
            const u_int64_t bb = reinterpret_cast<u_int64_t>(*b);
            return (aa > bb) - (aa < bb);
        }

        ex::data::stackarena::allocator arena_ = {
            STACK_BLOCK_SIZE,
            STACK_ARENA_SIZE
        };

        block_map_t  blocks_ = { arena_.compat(), &ptr_compare };
        block_free_t free_   = { arena_.compat() };

        mem_provider memory_;

        std::size_t used_mem_ = 0;
        std::size_t def_size_ = 0;
        std::size_t grow_mem_ = 0;

        block_size_t min_;
        block_size_t max_;

        ex::data::allocator compat() {
            return alloc_compat<allocator>::allocator(this);
        }

        allocator(const std::size_t pre_allocated = ALLOC_DEF_SIZE_B) {
            preallocate(pre_allocated);
        }

        allocator(const allocator &other) = delete;
        allocator(allocator &&other)      = delete;

        allocator &operator = (const allocator &other) = delete;
        allocator &operator = (allocator &&other)      = delete;

        ~allocator() {
            blocks_.clean();
            free_.clean();
            arena_.clean();
            memory_.clean();
        }

        void clean() {
            blocks_.clean();
            free_.clean();
            arena_.clean();
            memory_.clean();
            used_mem_ = 0;
        }

        void preallocate(const std::size_t pre_allocated = ALLOC_DEF_SIZE_B) {
            if (pre_allocated <= 0)
                abort_("Allocation size must be greater than 0");

            blocks_.clean();
            free_.clean();
            used_mem_ = 0;

            std::size_t        allocated_size = 0;
            const block_addr_t allocated_ptr  = memory_.request(pre_allocated, &allocated_size);

            const block_size_t      size_b = static_cast<block_size_t>(allocated_size);
            block_map_t::list_node *block  = blocks_.put(allocated_ptr, { size_b, 1, true });
            free_.put(size_b, &(block->val));

            def_size_ = pre_allocated;
            grow_mem_ = pre_allocated;
        }

        block_addr_t malloc(const std::size_t size_b, const std::size_t alignment_b) {
            if (size_b == 0)
                abort_("Empty memory block allocation");
            // TODO
            return nullptr;
        }

        block_addr_t realloc(void *ptr, const std::size_t size_new_b) {
            if (size_new_b == 0 || ptr == nullptr) {
                free(ptr);
                return nullptr;
            }
            // TODO
            return nullptr;
        }

        void free(const block_addr_t ptr) {
            if (ptr == nullptr)
                return;
            // TODO
        }

        long size_used() const {
            return used_mem_;
        }

        long size_total() const {
            return memory_.size();
        }

        // =========================================== INTERNAL UTILS ==================================================
        // TODO
    };

}

#endif //EX_LIMBO_DATA_LALLOC_H
