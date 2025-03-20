//
// Created by xd on 3/17/25.
//

#ifndef EX_LIMBO_DATA_RBTALLOC_H
#define EX_LIMBO_DATA_RBTALLOC_H

#include "datalloc.h"
#include "provider.h"

#include "../struct/array_map.h"
#include "../struct/rmq_map.h"

namespace ex::data::rbtalloc {

// -----------------------------
#define ALLOC_DEF_SIZE_B (1  MB)
#define STACK_ARENA_SIZE (10 MB)
// -----------------------------

    struct memory_block {
        u_int32_t size_b;
        u_int16_t alignment_b;
        u_int8_t  is_free;
    };

    using block_addr_t = void *;
    using block_free_t = u_int32_t;
    using block_map_t  = rmq_map<block_addr_t, memory_block, block_free_t>;
    using watch_map_t  = array_map<watch_id_t, watch_fn_t>;

#define BLOCK_MAX_SIZE_B (4294967295U)
#define STACK_BLOCK_SIZE (sizeof(block_map_t::rmq_node))

    struct allocator {

        static int32_t ptr_compare(const block_addr_t *a, const block_addr_t *b) {
            const u_int64_t aa = reinterpret_cast<u_int64_t>(*a);
            const u_int64_t bb = reinterpret_cast<u_int64_t>(*b);
            return (aa > bb) - (aa < bb);
        }

        block_map_t  blocks_ = &ptr_compare;
        watch_map_t  watche_;
        mem_provider memory_;

        std::size_t used_mem_ = 0;
        std::size_t def_size_ = 0;

        allocator(const std::size_t pre_allocated = ALLOC_DEF_SIZE_B) {
            preallocate(pre_allocated);
        }

        ~allocator() {
            watche_.clean();
            blocks_.clean();
            memory_.clean();
        }

        void clean() {
            watche_.clean();
            blocks_.clean();
            memory_.clean();
            used_mem_ = 0;
        }

        void preallocate(const std::size_t pre_allocated = ALLOC_DEF_SIZE_B) {
            if (pre_allocated <= 0)
                abort_("Allocation size must be greater than 0");

            std::size_t        allocated_size = 0;
            const block_addr_t allocated_ptr  = memory_.request(pre_allocated, &allocated_size);

            const auto size_b = static_cast<block_free_t>(allocated_size);
            blocks_.put(allocated_ptr, { size_b, 1, true }, size_b);

            def_size_ = pre_allocated;
        }

        void add_watcher(const watch_id_t id, const watch_fn_t fn) {
            watche_.put(id, fn);
        }

        void del_watcher(const watch_id_t id, watch_fn_t) {
            watche_.remove(id);
        }

        block_addr_t malloc(const std::size_t size_b, const std::size_t alignment_b) {
            if (size_b == 0)
                abort_("Empty memory block allocation");
            if (const block_addr_t allocated = block_acquire_(size_b, alignment_b))
                return allocated;
            block_request_(size_b + alignment_b);
            if (const block_addr_t allocated = block_acquire_(size_b, alignment_b))
                return allocated;
            abort_("Out of free space");
        }

        block_addr_t realloc(void *ptr, const std::size_t size_new_b) {
            if (size_new_b == 0 || ptr == nullptr) {
                free(ptr);
                return nullptr;
            }

            const auto start_before = memory_.data_ptr();
            const auto start_ptr    = reinterpret_cast<u_int64_t>(start_before);
            const auto data_ptr     = reinterpret_cast<u_int64_t>(ptr);
            const auto offset       = data_ptr - start_ptr;

            const memory_block block = blocks_.at(ptr);
            block_free_(ptr, block); // this marks block as free but preserves data

            const block_addr_t allocated = malloc(size_new_b, block.alignment_b);
            if (allocated != ptr) {
                const void *old_src_ptr = static_cast<unsigned char *>(memory_.data_ptr()) + offset;
                ::memmove(allocated, old_src_ptr, size_new_b < block.size_b ? size_new_b : block.size_b);
            }

            return allocated;
        }

        void free(const block_addr_t ptr) {
            if (ptr == nullptr)
                return;
            const memory_block block = blocks_.at(ptr);
            block_free_(ptr, block);
        }

        long size_used() const {
            return used_mem_;
        }

        long size_total() const {
            return memory_.size();
        }

        void *memory_start() const {
            return memory_.data_ptr();
        }

        // =========================================== INTERNAL UTILS ==================================================

        /**
         * Allocate at least additional <b>min_size_b</b> bytes of memory
         */
        void block_request_(const std::size_t min_size_b) {

            if (min_size_b > BLOCK_MAX_SIZE_B)
                abort_("Requested allocation size overflow");

            block_map_t::rmq_node *last = blocks_.last();
            if (last == nullptr) {
                preallocate(def_size_);
                last = blocks_.last();
                if (last == nullptr)
                    abort_("Allocator not initialized");
            }

            const block_addr_t last_ptr   = last->key;
            const memory_block last_block = last->val;

            int64_t     address_shift  = 0;
            std::size_t allocated_size = 0;

            void *allocated_ptr = memory_.request(min_size_b, &allocated_size, &address_shift);

            // validate data continuity
            if ((static_cast<unsigned char *>(last_ptr) + address_shift + last_block.size_b) != allocated_ptr) {
                abort_("Data fragmentation error: requested block alignment error");
            }

            // update pointers in case of data relocation
            if (address_shift != 0) {
                std::cout << "shift: " << address_shift << std::endl;
                for (auto &node : blocks_) { // not very safe btw
                    node.key = static_cast<unsigned char *>(node.key) + address_shift;
                }

                for (int i = 0; i < watche_.size; ++i) {
                    if (const auto fn = watche_.val_entries[i])
                        fn(watche_.key_entries[i], address_shift);
                }
            }

            // extend last free block
            if (last_block.is_free == true) {
                last->val.size_b += allocated_size;
                last->rmq = last->val.size_b;
                blocks_.update_ranges_(last);
                return;
            }

            // append new free block
            const auto size_b = static_cast<block_free_t>(allocated_size);
            blocks_.put(allocated_ptr, { size_b, 1, true }, size_b);
        }

        struct test_this {
            u_int64_t size_needed;
            u_int64_t alignment_b;
            u_int64_t data_offset;
        };

        static bool test_padding_(const block_map_t::rmq_node *node, void *this_) {
            const test_this *that_   = static_cast<test_this *>(this_);
            const u_int64_t  padding = block_padding_(that_->data_offset, node->key, that_->alignment_b);
            return node->val.size_b >= (that_->size_needed + padding);
        }

        static u_int32_t block_padding_(const u_int64_t data, const void *ptr, const u_int64_t alignment_b) {
            const u_int64_t  offset  = reinterpret_cast<u_int64_t>(ptr) - data;
            return (offset % alignment_b != 0) ? alignment_b - (offset % alignment_b) : 0;
        }

        block_addr_t block_acquire_(const std::size_t size_b, const std::size_t alignment_b) {
            if (size_b == 0)
                abort_("Requested blocks size must be non-zero");
            if ((size_b + alignment_b) > BLOCK_MAX_SIZE_B)
                abort_("Requested allocation size overflow");
            if (alignment_b == 0 || (alignment_b & (alignment_b - 1)) != 0)
                abort_("Alignment must be non-zero and a power of two");

            const u_int64_t data_start = reinterpret_cast<u_int64_t>(memory_.data_ptr());
            test_this       test_that  = { size_b, alignment_b, data_start };

            const block_map_t::rmq_node *fit = blocks_.range_fit(size_b, BLOCK_MAX_SIZE_B, &test_padding_, &test_that);
            if (fit == nullptr)
                return nullptr;

            const memory_block fit_block = fit->val;
            void              *fit_key   = fit->key;
            const u_int32_t    padding   = block_padding_(data_start, fit_key, alignment_b);

            const u_int32_t size_ls = static_cast<u_int32_t>(size_b);
            const u_int32_t size_rs = fit_block.size_b - size_ls - padding;

            const block_addr_t l_ptr   = static_cast<unsigned char *>(fit_key) + padding;
            const memory_block l_block = { size_ls, static_cast<u_int16_t>(alignment_b), false };
            blocks_.put(l_ptr, l_block, 0);

            if (padding > 0) {
                const memory_block f_block = { padding, 1, true };
                blocks_.put(fit_key, f_block, padding);
            }

            if (size_rs > 0) {
                const block_addr_t r_ptr   = static_cast<unsigned char *>(l_ptr) + size_ls;
                const memory_block r_block = { size_rs, 1, true };
                blocks_.put(r_ptr, r_block, size_rs);
            }

            used_mem_ += size_ls;

            return l_ptr;
        }

        void block_free_(const block_addr_t ptr, const memory_block &block) {
            if (block.is_free == true)
                abort_("Memory block double free");

            used_mem_ -= block.size_b;

            memory_block free_block = { block.size_b, 1, 1 };
            block_addr_t free_ptr   = ptr;

            const block_map_t::rmq_node *next = blocks_.next(ptr);
            const block_map_t::rmq_node *prev = blocks_.prev(ptr);

            block_addr_t prev_key = nullptr;
            block_addr_t next_key = nullptr;

            if (next != nullptr && next->val.is_free == true) {
                // merging next block
                free_block.size_b += next->val.size_b;
                next_key = next->key;
            }

            if (prev != nullptr && prev->val.is_free == true) {
                // merging prev block
                free_block.size_b += prev->val.size_b;
                prev_key = prev->key;
                free_ptr = prev->key;
            }

            if (next_key != nullptr) {
                blocks_.remove(next_key);
            }

            if (prev_key != nullptr) {
                blocks_.remove(prev_key);
            }

            blocks_.remove(ptr);
            blocks_.put(free_ptr, free_block, free_block.size_b);
        }
    };

}

#endif //EX_LIMBO_DATA_RBTALLOC_H
