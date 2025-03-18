//
// Created by xd on 3/17/25.
//

#ifndef EX_LIMBO_DATA_RBTALLOC_H
#define EX_LIMBO_DATA_RBTALLOC_H

#include "datalloc.h"
#include "stackarena.h"
#include "../struct/array_map.h"
#include "../struct/rmq_map.h"

namespace ex::data::rbtalloc {

// -----------------------------
#define ALLOCATOR_SIZE_B (1 MB)
#define STACK_ARENA_SIZE (10 MB)
// -----------------------------

#ifndef UNIT_TEST
    #define abort_(msg) do { std::cerr << msg << std::endl; ::abort(); } while(false)
#else
    #define abort_(msg) throw std::runtime_error("Generic allocator abort exception")
#endif

    struct memory_block {
        u_int32_t size_b;
        u_int16_t alignment_b;
        u_int8_t  is_free;
    };

    using block_addr_t = void *;
    using block_free_t = u_int32_t;
    using block_map_t  = rmq_map<block_addr_t, memory_block, block_free_t>;
    using watch_map_t  = array_map<watch_id_t, watch_fn_t>;

    namespace providers_ {
        // TODO REPLACE WITH SYS-CALLS
        struct mem_provider {
            void       *data     = nullptr;
            std::size_t offset   = 0;
            std::size_t capacity = 0;

            mem_provider() = default;

            ~mem_provider() {
                if (data == nullptr)
                    return;
                ::free(data);
            }

            void clean() {
                offset   = 0;
                capacity = 0;
                if (data == nullptr)
                    return;
                ::free(data);
            }

            long size() const {
                return offset;
            }

            void *data_ptr() const {
                return data;
            }

            void *request(const std::size_t size, std::size_t *size_allocated = nullptr, int64_t *addr_diff = nullptr) {
                const std::size_t curr_offset = offset;
                const std::size_t real_size   = capacity + size;

                const void *old_data_ptr = data;
                {
                    capacity += real_size;
                    offset   += real_size;
                    if (data == nullptr) data = ::malloc(capacity);
                    else                 data = ::realloc(data, capacity);
                }
                const void *new_data_ptr = data;

                if (addr_diff != nullptr && old_data_ptr != nullptr) {
                    if (old_data_ptr == new_data_ptr)
                        *addr_diff = 0;
                    else
                        *addr_diff = reinterpret_cast<unsigned long int>(new_data_ptr) -
                                     reinterpret_cast<unsigned long int>(old_data_ptr);
                }

                if (size_allocated != nullptr) {
                    *size_allocated = real_size;
                }

                return static_cast<unsigned char *>(data) + curr_offset;
            }
        };
    }

    struct allocator {
#define STACK_BLOCK_SIZE (sizeof(block_map_t::rmq_node))
        // todo replace with dynamic
        static inline stackarena::static_allocator<STACK_BLOCK_SIZE, STACK_ARENA_SIZE> arena_{};

        static int32_t ptr_compare(const block_addr_t *a, const block_addr_t *b) {
            const u_int64_t aa = reinterpret_cast<u_int64_t>(*a);
            const u_int64_t bb = reinterpret_cast<u_int64_t>(*b);
            return (aa > bb) - (aa < bb);
        }

        block_map_t              blocks_ = { arena_.compat_allocator(), &ptr_compare };
        watch_map_t              watche_;
        providers_::mem_provider memory_;

        std::size_t used_mem_ = 0;

        allocator(const std::size_t pre_allocated = ALLOCATOR_SIZE_B) {
            if (pre_allocated <= 0)
                abort_("Allocation size must be greater than 0");

            std::size_t allocated_size = 0;
            void *const allocated_ptr  = memory_.request(pre_allocated, &allocated_size);

            const auto size_b = static_cast<block_free_t>(allocated_size);
            blocks_.put(allocated_ptr, { size_b, 1, true }, size_b);
        }

        void cleanup() {
            watche_.clean();
            blocks_.clean();
            memory_.clean();
        }

        void add_watcher(const watch_id_t id, const watch_fn_t fn) {
            watche_.put(id, fn);
        }

        void del_watcher(const watch_id_t id, watch_fn_t) {
            watche_.remove(id);
        }

        void *allocate(const std::size_t size_b, const std::size_t alignment_b) {
            if (size_b <= 0) abort_("Empty memory block allocation");
            // TODO
            abort_("Out of free space");
        }

        void *reallocate(void *ptr, const std::size_t size_new_b) {
            if (size_new_b <= 0 || ptr == nullptr) {
                free(ptr);
                return nullptr;
            }
            // TODO
            abort_("Out of free space");
        }

        void free(void *ptr) {
            if (ptr == nullptr)
                return;
            const memory_block block = blocks_.at(ptr);
            if (block.is_free)
                abort_("Memory block double free");

            used_mem_ -= block.size_b;

            memory_block free_block = { block.size_b, 1, 1 };
            void        *free_ptr   = ptr;

            const block_map_t::rmq_node *next = blocks_.next(ptr);
            const block_map_t::rmq_node *prev = blocks_.prev(ptr);

            if (next != nullptr && next->val.is_free == true) {
                // merging next block
                free_block.size_b += next->val.size_b;
                blocks_.remove(next->key);
            }

            if (prev != nullptr && prev->val.is_free == true) {
                // merging prev block
                free_block.size_b += prev->val.size_b;
                free_ptr = prev->key;
                blocks_.remove(free_ptr);
            }

            blocks_.remove(ptr);
            blocks_.put(free_ptr, free_block, free_block.size_b);
        }

        long memory_used_b() const {
            return used_mem_ + arena_.size_used();
        }

        long memory_total_b() const {
            return memory_.size() + arena_.size_total();
        }
    };

}

#endif //EX_LIMBO_DATA_RBTALLOC_H
