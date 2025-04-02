//
// Created by henryco on 24/03/25.
//

#ifndef EX_LIMBO_DATA_LALLOC_H
#define EX_LIMBO_DATA_LALLOC_H

#include "../struct/lifo_queue.h"
#include "../struct/list_map.h"
#include "stackarena.h"

#ifdef _MSC_VER
    #include <intrin.h>
#endif

namespace ex::data::lalloc {

// -----------------------------
#define ALLOC_DEF_SIZE_B (1 MB)
#define STACK_ALLOC_SIZE (1 MB)
// -----------------------------

    struct memory_node;

    struct alignas(8) memory_block {
        memory_node *node_ptr;
    };

    struct memory_node {
        memory_node  *prev_ptr;
        memory_block *block_ptr;
        void         *bucket_ptr;
        u_int32_t     size_b;
        u_int32_t     alignment_b;
    };

    using block_addr_t   = void *;
    using block_size_t   = u_int32_t;
    using block_bucket_t = ex::data::lifo_queue<memory_node *>;

#define BLOCK_MAX_SIZE_B (4294967295U)
#define ARENA_BLOCK_SIZE (sizeof(memory_node))
#define STACK_BLOCK_SIZE (sizeof(block_bucket_t::lifo_node))
#define LIFO_BLOCK_SIZE  (16)
#define BUCKETS_NUM      (32)

#ifndef UNIT_TEST
    #define validate_header_(ptr) validate_header__(ptr)
    #define block_init_(ptr) block_eager__(ptr)
#else
    #define validate_header_(ptr) ;
    #define block_init_(ptr) ;
#endif

    static u_int32_t power_range_r_s(const u_int32_t size_b) {
        if (size_b == 0)
            return 0;
#ifdef __clang__
        return (sizeof(size_b) * 8 - __builtin_clz(size_b) - 1);
#elif defined(__GNUC__)
        return return (sizeof(size_b) * 8 - __builtin_clz(size_b) - 1);
#elif defined(_MSC_VER)
        u_int64_t index;
        _BitScanReverse(&index, size_b);
        return static_cast<u_int32_t>(index);
#else
    #error "Compiller not supported"
        abort_("Compiller not supported");
#endif
    }

    struct allocator {

        ex::data::stackarena::allocator arena_ = {
            ARENA_BLOCK_SIZE,
            STACK_ALLOC_SIZE
        };

        ex::data::stackarena::allocator stack_ = {
            STACK_BLOCK_SIZE,
            STACK_ALLOC_SIZE
        };

        block_bucket_t lifo_ = {
            stack_.compat()
        };

        block_bucket_t buckets_[BUCKETS_NUM] = {
            stack_.compat(), // 2^0
            stack_.compat(), // 2^1
            stack_.compat(), // 2^2
            stack_.compat(), // 2^3
            stack_.compat(), // 2^4
            stack_.compat(), // 2^5
            stack_.compat(), // 2^6
            stack_.compat(), // 2^7
            stack_.compat(), // 2^9
            stack_.compat(), // 2^10
            stack_.compat(), // 2^11
            stack_.compat(), // 2^12
            stack_.compat(), // 2^13
            stack_.compat(), // 2^14
            stack_.compat(), // 2^15
            stack_.compat(), // 2^16
            stack_.compat(), // 2^17
            stack_.compat(), // 2^18
            stack_.compat(), // 2^19
            stack_.compat(), // 2^20
            stack_.compat(), // 2^21
            stack_.compat(), // 2^22
            stack_.compat(), // 2^23
            stack_.compat(), // 2^24
            stack_.compat(), // 2^25
            stack_.compat(), // 2^26
            stack_.compat(), // 2^27
            stack_.compat(), // 2^28
            stack_.compat(), // 2^29
            stack_.compat(), // 2^30
            stack_.compat(), // 2^31
            stack_.compat(), // 2^32
        };

        mem_provider memory_;

        std::size_t def_size_     = 0;
        std::size_t grow_mem_     = 0;
        std::size_t used_mem_     = 0;
        std::size_t blocks_total_ = 0;
        u_int32_t   min_bucket_   = BUCKETS_NUM;

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
            lifo_.clear();
            for (int i = 0; i < BUCKETS_NUM; i++)
                buckets_[i].clear();
            arena_.clean();
            stack_.clean();
            memory_.clean();
        }

        void clean() {
            lifo_.clear();
            for (int i = 0; i < BUCKETS_NUM; i++)
                buckets_[i].clear();
            arena_.clean();
            stack_.clean();
            memory_.clean();
            used_mem_ = 0;
        }

        void preallocate(const std::size_t pre_allocated = ALLOC_DEF_SIZE_B) {
            if (pre_allocated <= 0)
                abort_("Allocation size must be greater than 0");

            used_mem_ = 0;

            std::size_t        allocated_size = 0;
            const block_addr_t allocated_ptr  = memory_.request(pre_allocated, &allocated_size);

            const uint32_t     padding = block_padding_(allocated_ptr, sizeof(memory_block));
            const block_size_t size_b  = static_cast<block_size_t>(allocated_size - padding) - sizeof(memory_block);

            unsigned char *block_ptr = static_cast<unsigned char *>(allocated_ptr) + padding;
            memory_block  *block     = block_create_(block_ptr, size_b - sizeof(memory_block));

            validate_header_(block_ptr + sizeof(memory_block));
            block_init_(block);

            unsigned char *null_space = static_cast<unsigned char *>(allocated_ptr) + allocated_size - sizeof(memory_block);
            ::memset(null_space, 0, sizeof(memory_block));

            bucket_put_(block);

            def_size_ = pre_allocated;
            grow_mem_ = pre_allocated;
        }

        block_addr_t malloc(const std::size_t size_b, const std::size_t alignment_b) {
            if (size_b == 0)
                abort_("Empty memory block allocation");

            if (const block_addr_t allocated = block_acquire_(size_b, alignment_b)) {
                validate_header_(allocated);
                return allocated;
            }

            block_request_(size_b + alignment_b);

            if (const block_addr_t allocated = block_acquire_(size_b, alignment_b)) {
                validate_header_(allocated);
                return allocated;
            }

            abort_("Out of free space");
        }

        block_addr_t realloc(void *ptr, const std::size_t size_new_b) {

            if (size_new_b == 0 || ptr == nullptr) {
                free(ptr);
                return nullptr;
            }

            unsigned char *h_ptr = static_cast<unsigned char *>(header_ptr_(ptr));
            memory_block  *block = reinterpret_cast<memory_block *>(h_ptr);

            if (block->node_ptr->block_ptr != block)
                abort_("Invalid block address, block bucket address invalid: " << block << " | " << block->node_ptr->block_ptr);

            if (block->node_ptr->bucket_ptr != nullptr)
                abort_("Reallocation of free block");

            if (size_new_b == static_cast<std::size_t>(block->node_ptr->size_b))
                return ptr;

            if (size_new_b < static_cast<std::size_t>(block->node_ptr->size_b)) {
                // TODO: SHRINK
                goto fallback_go;
            }

            if (memory_block *next_block = next_block_(block)) {
                memory_node *next_node = next_block->node_ptr;

                if (next_node->bucket_ptr == nullptr)
                    goto fallback_go;

                if (next_node->prev_ptr != block->node_ptr)
                    abort_("Memory layout fragmentation error");
                if (!continuous_(block, next_block))
                    abort_("Memory layout fragmentation error");

                const u_int64_t available = next_node->size_b + sizeof(memory_block);
                const u_int64_t has       = block->node_ptr->size_b;
                const u_int64_t need      = size_new_b - has;

                if (available < need)
                    goto fallback_go;

                const u_int32_t power = power_range_r_s(next_node->size_b);
                buckets_[power].remove(static_cast<block_bucket_t::lifo_node *>(next_node->bucket_ptr));

                int32_t p3, // empty "dead" space or padding before FREE block (if p4 exists)
                        p4; // size of free block (after)
                calculate_post_(next_block, available, need, &p3, &p4);

                block->node_ptr->size_b += need;

                if (p3 >= 0)
                    block->node_ptr->size_b += p3;

                if (p4 >= 0) {
                    unsigned char *start_ptr = reinterpret_cast<unsigned char *>(next_block);
                    memory_block *end_block = block_create_(start_ptr + need + p3, p4);
                    end_block->node_ptr->prev_ptr = block->node_ptr;

                    if (memory_block *next = next_block_(end_block); next != nullptr)
                        next->node_ptr->prev_ptr = end_block->node_ptr;

                    lifo_put_(end_block);
                }

                return ptr;
            }

        fallback_go:
            const block_addr_t allocated = malloc(size_new_b, block->node_ptr->alignment_b);

            if (allocated != ptr) {
                validate_header_(allocated);
                const int32_t padding = static_cast<unsigned char *>(ptr) - (h_ptr + sizeof(memory_block));
                const int32_t block_s = block->node_ptr->size_b - padding;
                ::memmove(allocated, ptr, size_new_b < block_s ? size_new_b : block_s);
                free(ptr);
                return allocated;
            }

            validate_header_(allocated);
            return allocated;
        }

        void free(const block_addr_t ptr) {
            if (ptr == nullptr)
                return;

            unsigned char *h_ptr = static_cast<unsigned char *>(header_ptr_(ptr));
            memory_block  *block = reinterpret_cast<memory_block *>(h_ptr);

            if (block->node_ptr->block_ptr != block)
                abort_("Invalid block address, block bucket address invalid: " << block << " | " << block->node_ptr->block_ptr);

            if (block->node_ptr->bucket_ptr != nullptr)
                abort_("Memory block double free");

            used_mem_ -= (block->node_ptr->size_b + sizeof(memory_block));

            block->node_ptr->alignment_b = 1;
            lifo_put_(block);
        }

        long size_used() const {
            return used_mem_;
        }

        long size_total() const {
            return memory_.size();
        }

        long blocks_total() const {
            return blocks_total_;
        }

        long blocks_used() const {
            u_int32_t c = 0;
            for (int i = 0; i < BUCKETS_NUM; ++i)
                c += buckets_[i].size();
            c += lifo_.size();
            return c;
        }

        // =========================================== INTERNAL UTILS ==================================================

        /**
         * Allocate at least additional <b>min_size_b</b> bytes of memory
         */
        void block_request_(const std::size_t min_size_b) {
            if (min_size_b > BLOCK_MAX_SIZE_B)
                abort_("Requested allocation size overflow");

            grow_mem_ = grow_mem_ + grow_mem_ + min_size_b + sizeof(memory_block);

            std::size_t allocated_size = 0;
            void       *allocated_ptr  = memory_.request(grow_mem_, &allocated_size);

            // append new free block
            const uint32_t     padding = block_padding_(allocated_ptr, sizeof(memory_block));
            const block_size_t size_b  = static_cast<block_size_t>(allocated_size - padding) - sizeof(memory_block);

            unsigned char *block_ptr = static_cast<unsigned char *>(allocated_ptr) + padding;
            memory_block  *block     = block_create_(block_ptr, size_b - sizeof(memory_block));

            validate_header_(block_ptr + sizeof(memory_block));
            block_init_(block);

            unsigned char *null_space = static_cast<unsigned char *>(allocated_ptr) + allocated_size - sizeof(memory_block);
            ::memset(null_space, 0, sizeof(memory_block));

            bucket_put_(block);
        }

        block_addr_t block_acquire_(const std::size_t size_b, const std::size_t alignment_b) {
            if (size_b == 0)
                abort_("Requested blocks size must be non-zero");
            if ((size_b + alignment_b) > BLOCK_MAX_SIZE_B)
                abort_("Requested allocation size overflow");
            if (alignment_b == 0 || (alignment_b & (alignment_b - 1)) != 0)
                abort_("Alignment must be non-zero and a power of two");

            if (size_b < LIFO_BLOCK_SIZE && !lifo_.empty())
                if (void *ptr = bucket_scan_(lifo_, size_b, alignment_b))
                    return ptr;

            const u_int32_t power = power_range_r_s(size_b);
            for (u_int32_t i = power > min_bucket_ ? power : min_bucket_; i < BUCKETS_NUM; i++) {
                if (buckets_[i].empty())
                    continue;
                if (void *ptr = bucket_scan_(buckets_[i], size_b, alignment_b))
                    return ptr;
            }

            return nullptr;
        }

        void lifo_put_(memory_block *block) {
            if (block->node_ptr->bucket_ptr != nullptr)
                abort_("block should be free");
//            block_merge_(block);
            if (block->node_ptr->size_b == 0 || block->node_ptr->size_b >= LIFO_BLOCK_SIZE) {
                block_merge_(block);
                if (!lifo_.empty())
                    block_merge_(lifo_.root_remove()->block_ptr);
                return;
            }
            lifo_.push(block->node_ptr);
            if (lifo_.size() >= 16)
                block_merge_(lifo_.root_remove()->block_ptr);
        }

        void bucket_put_(memory_block *block) {
            const u_int32_t power = power_range_r_s(block->node_ptr->size_b);
            if (power >= BUCKETS_NUM)
                abort_("Block size too big");

            min_bucket_ = power < min_bucket_ ? power : min_bucket_;

            void *ptr = buckets_[power].push(block->node_ptr);
            block->node_ptr->bucket_ptr = ptr;
        }

        void block_merge_(memory_block *block) {
            memory_block *target_block = block;
            memory_node  *node         = block->node_ptr;

//            std::cout << "merge: " << block->node_ptr->block_ptr << ", " << node->size_b << std::endl;

            if (node->bucket_ptr != nullptr) {
                const u_int32_t p = power_range_r_s(node->size_b);
                buckets_[p].remove(static_cast<block_bucket_t::lifo_node *>(node->bucket_ptr));
            }

            if (memory_block *next_block = next_block_(block)) {
                if (memory_node *next_node = next_block->node_ptr;
                    next_node->bucket_ptr != nullptr) {

//                    std::cout << "+ next: " << next_block << ", " << next_node->size_b << std::endl;

                    if (!continuous_(block, next_block, node->size_b + sizeof(memory_block)))
                        abort_("block layout discontinuity");

                    const u_int32_t p = power_range_r_s(next_node->size_b);
                    buckets_[p].remove(static_cast<block_bucket_t::lifo_node *>(next_node->bucket_ptr));

                    node->size_b += next_node->size_b + sizeof(memory_block);
                    release_node_(next_node);

                    blocks_total_--;
                }
            }

            if (memory_node *prev_node = node->prev_ptr) {
                if (memory_block *prev_block = prev_node->block_ptr;
                    prev_node->bucket_ptr != nullptr) {

//                    std::cout << "+ prev: " << prev_block << ", " << prev_node->size_b << std::endl;

                    const u_int32_t p = power_range_r_s(prev_node->size_b);
                    buckets_[p].remove(static_cast<block_bucket_t::lifo_node *>(prev_node->bucket_ptr));

                    prev_node->size_b += (node->size_b + sizeof(memory_block));
                    release_node_(node);

                    blocks_total_--;

                    target_block = prev_block;
                    node         = prev_node;
                }
            }

            if (memory_block *next_block = next_block_(target_block)) {
                if (!continuous_(target_block, next_block, node->size_b + sizeof(memory_block)))
                    abort_("block layout discontinuity");
                next_block->node_ptr->prev_ptr = node;
            }

//            std::cout << "put: " << target_block->node_ptr->block_ptr << ", " << target_block->node_ptr->size_b << '\n' << std::endl;
            bucket_put_(target_block);
        }

        memory_block *block_create_(void *mem, const u_int32_t size) {
            memory_block *new_block = reinterpret_cast<memory_block *>(mem);
            memory_node  *new_node  = create_node_();

            new_node->block_ptr   = new_block;
            new_node->bucket_ptr  = nullptr;
            new_node->prev_ptr    = nullptr;
            new_node->size_b      = size;
            new_node->alignment_b = 1;

            blocks_total_ += 1;

            new_block->node_ptr = new_node;
            return new_block;
        }

        void release_node_(memory_node *node) {
            arena_.free(node);
        }

        memory_node *create_node_() {
            return static_cast<memory_node *>(arena_.malloc(sizeof(memory_node), sizeof(memory_node)));
        }

        void *bucket_scan_(block_bucket_t &bucket, const std::size_t size_b, const std::size_t alignment_b) {

            for (block_bucket_t::lifo_node *head = bucket.peek(); head != nullptr; head = head->prev) {
                memory_node  *node  = head->val;
                memory_block *block = node->block_ptr;

                if (node->block_ptr != block)
                    abort_("Invalid block address, block bucket address invalid: " << block << " | " << node->block_ptr);

                if (static_cast<std::size_t>(node->size_b) < size_b)
                    continue;

                unsigned char *byte_ptr = reinterpret_cast<unsigned char *>(block) + sizeof(memory_block);

                int32_t p1, // natural padding or size of the block (if p2 exists)
                        p2; // natural padding before data block
                if (!calculate_pre_(byte_ptr, alignment_b, &p1, &p2))
                    continue;

                int32_t offset = 0;
                if (p1 >= 0)
                    offset += p1;

                if (p2 >= 0)
                    offset += p2 + sizeof(memory_block);

                const int32_t size_have = static_cast<int32_t>(node->size_b) - offset;
                if (size_have < size_b)
                    continue;

                unsigned char *start_ptr = byte_ptr + offset;

                int32_t p3, // empty "dead" space or padding before FREE block (if p4 exists)
                        p4; // size of free block (after)
                calculate_post_(start_ptr, size_have, size_b, &p3, &p4);

                memory_block *work_block = block;
                memory_block *lifo_block = nullptr;
                bool          remove_og  = false;
                bool          reinsert_  = false;

                if (p1 < 0 && p2 < 0) {
                    node->size_b = static_cast<u_int32_t>(size_b);
                    remove_og = true;
                }

                else if (p1 >= 0 && p2 < 0) {
                    node->size_b = static_cast<u_int32_t>(size_b + p1);
                    remove_og     = true;
                }

                else if (p1 >= 0 && p2 >= 0) {
                    // free og block
                    node->alignment_b = static_cast<u_int16_t>(1);
                    node->size_b      = static_cast<u_int32_t>(p1);

                    remove_og = true;
                    reinsert_ = true;

                    // new padded block
                    work_block = block_create_(byte_ptr + p1, size_b + p2);
                    work_block->node_ptr->prev_ptr = node;
                }

                if (p3 >= 0) {
                    work_block->node_ptr->size_b += p3;
                }

                if (p4 >= 0) {
                    memory_block *end_block = block_create_(start_ptr + size_b + p3, p4);
                    end_block->node_ptr->prev_ptr = work_block->node_ptr;

                    if (memory_block *next = next_block_(end_block); next != nullptr)
                        next->node_ptr->prev_ptr = end_block->node_ptr;

                    lifo_block = end_block;
                }

                if (p4 < 0 && p1 >= 0 && p2 >= 0) {
                    if (memory_block *next = next_block_(work_block); next != nullptr)
                        next->node_ptr->prev_ptr = work_block->node_ptr;
                }

                work_block->node_ptr->alignment_b = static_cast<u_int16_t>(alignment_b);

                used_mem_ += (work_block->node_ptr->size_b + sizeof(memory_block));

                validate_header_(start_ptr);
                block_init_(work_block);

                if (remove_og) {
                    head->val->bucket_ptr = nullptr;
                    bucket.remove(head);
                }

                if (reinsert_)
                    lifo_put_(block);

                if (lifo_block != nullptr)
                    lifo_put_(lifo_block);

                return start_ptr;
            }

            return nullptr;
        }

        static void calculate_post_(void *ptr, const int32_t size_have, const int32_t size_need, int32_t *p3, int32_t *p4) {
            const int32_t gap = size_have - size_need;

            if (gap == 0) {
                *p3 = -1;
                *p4 = -1;
                return;
            }

            if (static_cast<size_t>(gap) <= sizeof(memory_block)) {
                *p3 = gap;
                *p4 = -1;
                return;
            }

            unsigned char *gap_ptr = reinterpret_cast<unsigned char *>(ptr) + size_need;
            const int32_t  padding = static_cast<int32_t>(block_padding_(gap_ptr, sizeof(memory_block)));
            const int32_t  free_b  = gap - padding - static_cast<int32_t>(sizeof(memory_block));

            if (free_b <= 0) {
                *p3 = gap;
                *p4 = -1;
                return;
            }

            *p3 = padding;
            *p4 = free_b;
        }

        static bool calculate_pre_(void *ptr, const std::size_t alignment_b, int32_t *p1, int32_t *p2) {
            const u_int32_t padding_1 = block_padding_(ptr, alignment_b);

            // no padding, best case scenario
            if (padding_1 == 0) {
                *p1 = -1;
                *p2 = -1;
                return true;
            }

            // natural padding for header-data alignment
            if (static_cast<size_t>(padding_1) < sizeof(memory_block)) {
                *p1 = padding_1;
                *p2 = -1;
                return true;
            }

            unsigned char *zero_ptr = reinterpret_cast<unsigned char *>(ptr);
            unsigned char *data_ptr = zero_ptr + padding_1;
            unsigned char *head_ptr = reinterpret_cast<unsigned char *>(header_ptr_(data_ptr));

            const int32_t space_1 = static_cast<int32_t>(head_ptr - zero_ptr);
            if (space_1 < 0) // should be impossible in practice, I guess?
                return false; // replace with abort_ ?

            const int32_t space_2 = static_cast<int32_t>(data_ptr - head_ptr - sizeof(memory_block));
            if (space_2 < 0 || space_2 >= static_cast<int32_t>(sizeof(memory_block))) // WTF? HOW?
                return false; // replace with abort_ ?

            *p1 = space_1;
            *p2 = space_2;
            return true;
        }

        static memory_block *next_block_(memory_block *block) {
            memory_block *next = reinterpret_cast<memory_block *>(reinterpret_cast<unsigned char *>(block) +
                                                                  sizeof(memory_block) + block->node_ptr->size_b);
            if (next->node_ptr == nullptr)
                return nullptr;
            return next;
        }

        static bool continuous_(void *prev, void *next, const u_int64_t prev_size) {
            if (next == nullptr || prev == nullptr)
                return false;
            return (static_cast<unsigned char *>(prev) + static_cast<long>(prev_size)) == next;
        }

        static bool continuous_(memory_block *prev, memory_block *next) {
            if (next == nullptr || prev == nullptr)
                return false;
            void *a = reinterpret_cast<unsigned char *>(prev) + static_cast<long>(prev->node_ptr->size_b) + static_cast<long>(sizeof(memory_block));
            return a == next;
        }

        static u_int32_t block_padding_(const void *ptr, const u_int64_t alignment_b) {
            const u_int64_t address = reinterpret_cast<u_int64_t>(ptr);
            return (alignment_b - (address % alignment_b)) % alignment_b;
        }

        static void *header_ptr_(void *ptr) {
            unsigned char  *data_ptr = reinterpret_cast<unsigned char *>(ptr) - static_cast<long>(sizeof(memory_block));
            const u_int64_t address  = reinterpret_cast<u_int64_t>(data_ptr);
            const u_int64_t offset   = address % sizeof(memory_block);
            return data_ptr - static_cast<long>(offset);
        }

        static void validate_header__(void *ptr) {
            const memory_block *block = reinterpret_cast<memory_block *>(header_ptr_(ptr));
            if (block->node_ptr->block_ptr != block)
                abort_("block header corruption");
        }

        static void block_eager__(memory_block *block) {
            ::memset(reinterpret_cast<unsigned char *>(block) + sizeof(memory_block), 0, block->node_ptr->size_b);
        }
    };

}

#endif //EX_LIMBO_DATA_LALLOC_H
