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

    struct alignas(16) memory_block {
        void     *prev_ptr;
        u_int32_t size_b;
        u_int16_t alignment_b;
        u_int8_t  is_free;
        u_int8_t  magic_b;
    };

    using block_addr_t = void *;
    using block_size_t = u_int32_t;
    using block_list_t = linked_list<memory_block *>;

#define BLOCK_MAX_SIZE_B (4294967295U)
#define STACK_BLOCK_SIZE (sizeof(block_list_t::list_node))
#define BLOCK_MAGIC_BYTE (69)
// nice

#ifndef UNIT_TEST
    #define validate_header_(ptr) validate_header__(ptr)
    #define block_init_(ptr, padding) block_eager__(ptr, padding)
//    #define validate_header_(ptr)     ;
//    #define block_init_(ptr, padding) ;
#else
    #define validate_header_(ptr) ;
    #define block_init_(ptr, padding) ;
#endif

    struct allocator {

        static int8_t ptr_compare(memory_block *const *a, memory_block *const *b) {
            const u_int64_t aa = reinterpret_cast<u_int64_t>(*a);
            const u_int64_t bb = reinterpret_cast<u_int64_t>(*b);
            return (aa > bb) - (aa < bb);
        }

        ex::data::stackarena::allocator arena_ = {
            STACK_BLOCK_SIZE,
            STACK_ARENA_SIZE
        };

        block_list_t blocks_ = { arena_.compat(), &ptr_compare };

        mem_provider memory_;

        std::size_t def_size_ = 0;
        std::size_t grow_mem_ = 0;
        std::size_t used_mem_ = 0;

        std::size_t blocks_total_ = 0;

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
            arena_.clean();
            memory_.clean();
        }

        void clean() {
            blocks_.clean();
            arena_.clean();
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
            block_init_(block, 0);

            unsigned char *null_space = static_cast<unsigned char *>(allocated_ptr) + allocated_size - sizeof(memory_block);
            ::memset(null_space, 0, sizeof(memory_block));

            blocks_.put(block);

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

            if (block->magic_b != BLOCK_MAGIC_BYTE)
                abort_("Invalid block address, block magic byte invalid: " << block->magic_b);

            if (block->is_free == true)
                abort_("Reallocation of free block");

            // TODO CONTINUITY
            const block_addr_t allocated = malloc(size_new_b, block->alignment_b);

            if (allocated != ptr) {
                validate_header_(allocated);
                const int32_t padding = static_cast<unsigned char *>(ptr) - (h_ptr + sizeof(memory_block));
                const int32_t block_s = block->size_b - padding;
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

            if (block->magic_b != BLOCK_MAGIC_BYTE)
                abort_("Invalid block address, block magic byte invalid: " << block->magic_b);

            if (block->is_free == true)
                abort_("Memory block double free");

            block->alignment_b = 1;
            block->is_free     = 1;

//            if (memory_block *next = next_block_(block, ptr); next != nullptr) {
//                if (next->prev_ptr == nullptr)
//                    abort_("Invalid block reference [NULL]");
//
//                if (!continuous_(block, next, block->size_b + sizeof(memory_block)))
//                    abort_("Invalid block alignment [block -> next]");
//
//                block_list_t::list_node *node = reinterpret_cast<block_list_t::list_node *>(next->prev_ptr);
//                block->size_b += (next->size_b + sizeof(memory_block));
//                blocks_.remove_node(node);
//            }
//
//            if (block->prev_ptr != nullptr) {
//                block_list_t::list_node *node = reinterpret_cast<block_list_t::list_node *>(block->prev_ptr);
//                memory_block            *prev = node->key;
//                if (!continuous_(prev, block, prev->size_b + sizeof(memory_block)))
//                    abort_("Invalid block alignment [prev -> block]");
//
//                prev->size_b += (block->size_b + sizeof(memory_block));
//                used_mem_ -= (prev->size_b + sizeof(memory_block));
//                return;
//            }

            used_mem_ -= (block->size_b + sizeof(memory_block));
            blocks_.put_front(block);
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
            return blocks_.size();
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
            block_init_(block, 0);

            unsigned char *null_space = static_cast<unsigned char *>(allocated_ptr) + allocated_size - sizeof(memory_block);
            ::memset(null_space, 0, sizeof(memory_block));

            blocks_.put(block);
        }

        block_addr_t block_acquire_(const std::size_t size_b, const std::size_t alignment_b) {
            if (size_b == 0)
                abort_("Requested blocks size must be non-zero");
            if ((size_b + alignment_b) > BLOCK_MAX_SIZE_B)
                abort_("Requested allocation size overflow");
            if (alignment_b == 0 || (alignment_b & (alignment_b - 1)) != 0)
                abort_("Alignment must be non-zero and a power of two");

            for (block_list_t::list_node *head = blocks_.root_; head != nullptr; head = head->next) {
                memory_block *block = head->key;
                if (block->magic_b != BLOCK_MAGIC_BYTE)
                    abort_("Invalid block address, block magic byte invalid: " << block->magic_b);

                if (block->size_b < size_b)
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

                const int32_t size_have = static_cast<int32_t>(block->size_b) - offset;
                if (size_have < size_b)
                    continue;

                unsigned char *start_ptr = byte_ptr + offset;

                int32_t p3, // empty "dead" space or padding before FREE block (if p4 exists)
                        p4; // size of free block (after)
                calculate_post_(start_ptr, size_have, size_b, &p3, &p4);

                memory_block *work_block = block;
                bool          remove_og  = false;

                if (p1 < 0 && p2 < 0) {
                    block->size_b = static_cast<u_int32_t>(size_b);
                    remove_og = true;
                }

                else if (p1 >= 0 && p2 < 0) {
                    block->size_b = static_cast<u_int32_t>(size_b + p1);
                    remove_og     = true;
                }

                else if (p1 >= 0 && p2 >= 0) {
                    // free og block
                    block->alignment_b = static_cast<u_int16_t>(1);
                    block->magic_b     = static_cast<u_int8_t>(BLOCK_MAGIC_BYTE);
                    block->size_b      = static_cast<u_int32_t>(p1);
                    block->is_free     = static_cast<u_int8_t>(true);

                    // new padded block
                    work_block = block_create_(byte_ptr + p1, size_b + p2);
                    work_block->prev_ptr = head;
                }

                if (p3 >= 0) {
                    work_block->size_b += p3;
                }

                if (p4 >= 0) {
                    memory_block *end_block = block_create_(start_ptr + size_b + p3, p4);
                    block_list_t::list_node *inserted_node = blocks_.put_front(end_block);

//                    unsigned char *end_data = reinterpret_cast<unsigned char *>(end_block) + sizeof(memory_block);
//                    memory_block  *next     = next_block_(end_block, end_data);
//
//                    if (next != nullptr)
//                        next->prev_ptr = inserted_node;
                }

//                if (p4 < 0 && p1 >= 0 && p2 >= 0) {
//                    unsigned char *end_data = reinterpret_cast<unsigned char *>(work_block) + sizeof(memory_block);
//                    memory_block  *next     = next_block_(work_block, end_data);
//                    if (next != nullptr)
//                        next->prev_ptr = nullptr; // not free anymore
//                }

                work_block->alignment_b = static_cast<u_int16_t>(alignment_b);
                work_block->is_free     = static_cast<u_int8_t>(false);

                used_mem_ += (work_block->size_b + sizeof(memory_block));

                validate_header_(start_ptr);
                block_init_(work_block, (p1 >= 0 && p2 >= 0 ? p2 : p1));

                if (remove_og) {
                    blocks_.remove_node(head);
                }

                return start_ptr;
            }
            return nullptr;
        }

        memory_block *block_create_(void *mem, const u_int32_t size) {
            blocks_total_ += 1;
            memory_block *new_block = reinterpret_cast<memory_block *>(mem);
            new_block->prev_ptr     = nullptr;
            new_block->size_b       = static_cast<u_int32_t>(size);
            new_block->alignment_b  = static_cast<u_int16_t>(1);
            new_block->is_free      = static_cast<u_int8_t>(true);
            new_block->magic_b      = static_cast<u_int8_t>(BLOCK_MAGIC_BYTE);
            return new_block;
        }

        static void calculate_post_(void *ptr, const int32_t size_have, const int32_t size_need, int32_t *p3, int32_t *p4) {
            const int32_t gap = size_have - size_need;

            if (gap == 0) {
                *p3 = -1;
                *p4 = -1;
                return;
            }

            if (gap <= sizeof(memory_block)) {
                *p3 = gap;
                *p4 = -1;
                return;
            }

            unsigned char *gap_ptr = reinterpret_cast<unsigned char *>(ptr) + size_need;
            const int32_t  padding = block_padding_(gap_ptr, sizeof(memory_block));
            const int32_t  free_b  = gap - padding - sizeof(memory_block);

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
            if (padding_1 < sizeof(memory_block)) {
                *p1 = padding_1;
                *p2 = -1;
                return true;
            }

            unsigned char *zero_ptr = reinterpret_cast<unsigned char *>(ptr);
            unsigned char *data_ptr = zero_ptr + padding_1;
            unsigned char *head_ptr = reinterpret_cast<unsigned char *>(header_ptr_(data_ptr));

            const int32_t space_1 = head_ptr - zero_ptr;
            if (space_1 < 0) // should be impossible in practice, I guess?
                return false; // replace with abort_ ?

            const int32_t space_2 = data_ptr - head_ptr - sizeof(memory_block);
            if (space_2 < 0 || space_2 >= sizeof(memory_block)) // WTF? HOW?
                return false; // replace with abort_ ?

            *p1 = space_1;
            *p2 = space_2;
            return true;
        }

        static memory_block *next_block_(memory_block *block, void *data_ptr) {
            memory_block  *next_prev = nullptr;
            unsigned char *next_ptr  = static_cast<unsigned char *>(data_ptr) + block->size_b;
            ::memcpy(&next_prev, next_ptr, sizeof(memory_block *));
            if (next_prev == nullptr)
                return nullptr;
            return reinterpret_cast<memory_block *>(next_ptr);
        }

        static bool continuous_(void *prev, void *next, const u_int64_t prev_size) {
            if (next == nullptr || prev == nullptr)
                return false;
            return (static_cast<unsigned char *>(prev) + prev_size) == next;
        }

        static u_int32_t block_padding_(const void *ptr, const u_int64_t alignment_b) {
            const u_int64_t address = reinterpret_cast<u_int64_t>(ptr);
            return (alignment_b - (address % alignment_b)) % alignment_b;
        }

        static void *header_ptr_(void *ptr) {
            unsigned char  *data_ptr = reinterpret_cast<unsigned char *>(ptr) - sizeof(memory_block);
            const u_int64_t address  = reinterpret_cast<u_int64_t>(data_ptr);
            const u_int64_t offset   = address % sizeof(memory_block);
            return data_ptr - offset;
        }

        static void validate_header__(void *ptr) {
            const memory_block *block = reinterpret_cast<memory_block *>(header_ptr_(ptr));
            if (block->magic_b != BLOCK_MAGIC_BYTE)
                abort_("block header corruption");
        }

        static void block_eager__(memory_block *block, const u_int32_t padding = 0) {
            unsigned char *ptr_0 = reinterpret_cast<unsigned char *>(block) + sizeof(memory_block);
            if (padding <= 0) {
                ::memset(ptr_0, 0, block->size_b);
                return;
            }
            ::memset(ptr_0, 42, padding);
            ::memset(ptr_0 + padding, 0, block->size_b);
        }
    };

}

#endif //EX_LIMBO_DATA_LALLOC_H
