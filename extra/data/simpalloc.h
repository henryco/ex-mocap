//
// Created by xd on 3/4/25.
//

#ifndef EX_DATA_SIMPALLOC_H
#define EX_DATA_SIMPALLOC_H

#include "data_utils.h"

#include <cstring>
#include <iostream>

namespace ex::data::simpalloc {

    inline void abort(const char *msg) {
        std::cerr << msg << std::endl;
        ::abort();
    }

    // ---- DEFAULT FALLBACK (malloc/realloc)
    // TODO: replace with direct call to OS
    struct arena {

        void       *data     = nullptr;
        std::size_t offset   = 0;
        std::size_t capacity = 0;

        arena() = default;

        ~arena() {
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

        const void *data_ptr() const {
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
    // ---- DEFAULT FALLBACK (malloc/realloc)

    using watch_id_t = void *;
    using block_id_t = void *;

    struct memory_block {
        u_int32_t size_b;
        u_int16_t alignment_b;
        u_int8_t  is_free;
    };

    struct allocator {

        ex::data::map<block_id_t, memory_block> blocks_;
        ex::data::map<watch_id_t, watch_fn_t>   watche_;
        arena                                   memory_;

        #define DEFAULT_ARENA_SIZE_B 1048576

        allocator(const std::size_t pre_allocated = DEFAULT_ARENA_SIZE_B) {

            if (pre_allocated <= 0)
                abort("Allocation size must be greater than 0");

            std::size_t allocated_size = 0;
            void *const allocated_ptr  = memory_.request(pre_allocated, &allocated_size);

            blocks_.key_entries.push(allocated_ptr);
            blocks_.val_entries.push({ static_cast<u_int32_t>(allocated_size), 1, true });
        }

        block_id_t block_acquire(const int block_idx, const std::size_t size_b, const std::size_t alignment_b) {
            const memory_block block  = blocks_.val_entries[block_idx];
            if (block.is_free != true)
                return nullptr;

            const block_id_t block_id = blocks_.key_entries[block_idx];
            const u_int64_t  padding  = block_padding(block_id, alignment_b);
            const int64_t    space_b  = block.size_b - padding;
            const int64_t    rest_b   = space_b - size_b;

            if (space_b < size_b)
                return nullptr;

            // [block_b]: [padding|space_b] -> [padding] [space_b]
            const block_id_t ptr_space = block_split(block_id, space_b);

            // [space_b]: [size_b|rest_b]   -> [size_b] [rest_b]
            const block_id_t ptr_rest  = block_split(ptr_space, rest_b);

            if (padding != 0 && block_id != ptr_space) {
                block_free(block_id); // free padding
            }

            if (rest_b != 0 && ptr_rest != nullptr && ptr_rest != ptr_space) {
                block_free(ptr_rest); // free rest
            }

            const int ptr_idx = blocks_.index_at(ptr_space);

            blocks_.val_entries[ptr_idx].alignment_b = alignment_b;
            blocks_.val_entries[ptr_idx].size_b      = size_b;
            blocks_.val_entries[ptr_idx].is_free     = false;

            return ptr_space;
        }

        void *allocate(const std::size_t size_b, const std::size_t alignment_b) {
            if (size_b <= 0) {
                abort("Empty memory block allocation");
                return nullptr;
            }

            for (int i = 0; i < blocks_.size; i++) {
                if (const block_id_t block_id = block_acquire(i, size_b, alignment_b))
                    return block_id;
            }

            block_request(size_b + alignment_b);

            if (const block_id_t block_id = block_acquire(blocks_.size - 1, size_b, alignment_b))
                return block_id;

            abort("Out of free space");
            return nullptr;
        }

        void *reallocate(void *ptr, const std::size_t size_new_b) {
            // TODO
            return nullptr;
        }

        void free(void *ptr) {
            if (ptr == nullptr)
                return;

            if (blocks_.at(ptr).is_free == true) {
                abort("Double free: Attempting to free a block that is already has been freed");
                return;
            }

            block_free(ptr);
        }

        long memory_used_b() const {
            long used_b = 0;
            for (int i = 0; i < blocks_.size; ++i) {
                const memory_block mem = blocks_.val_entries[i];
                used_b += mem.is_free ? 0 : mem.size_b;
            }
            return used_b;
        }

        long memory_total_b() const {
            return memory_.size();
        }

        u_int64_t block_padding(const block_id_t block_id, const u_int64_t alignment_b) const {
            if (alignment_b <= 1)
                return 0;
            const u_int64_t offset = reinterpret_cast<u_int64_t>(block_id) -
                                     reinterpret_cast<u_int64_t>(memory_.data_ptr());
            if (offset % alignment_b != 0)
                return alignment_b - (offset % alignment_b);
            return 0;
        }

        /**
         * Allocate at least additional <b>min_size_b</b> bytes of memory
         */
        block_id_t block_request(const std::size_t min_size_b) {
            const int          last_idx   = blocks_.size - 1;
            const block_id_t   last_ptr   = blocks_.key_entries[last_idx];
            const memory_block last_block = blocks_.val_entries[last_idx];

            int64_t     addr_diff;
            std::size_t real_size;

            void *allocated_ptr = memory_.request(min_size_b, &real_size, &addr_diff);

            // validate data continuity
            if ((static_cast<unsigned char *>(last_ptr) + addr_diff + last_block.size_b) != allocated_ptr) {
                abort("Data fragmentation error: requested block alignment error");
                return nullptr;
            }

            // update pointers in case of data relocation
            if (addr_diff != 0) {
                for (int i = 0; i < blocks_.size; ++i) {
                    blocks_.key_entries[i] = static_cast<unsigned char *>(blocks_.key_entries[i]) + addr_diff;
                }

                for (int i = 0; i < watche_.size; ++i) {
                    if (const auto fn = watche_.val_entries[i])
                        fn(watche_.key_entries[i], addr_diff);
                }
            }

            // resize last free block
            if (last_block.is_free) {
                blocks_.val_entries[last_idx].size_b += real_size;
                return blocks_.key_entries[last_idx];
            }

            // append new free block
            blocks_.key_entries.push(allocated_ptr);
            blocks_.val_entries.push({ static_cast<u_int32_t>(real_size), 1, true });
            return allocated_ptr;
        }

        // Example memory layout
        // Alignment: 4
        //
        //                 2                   5
        //  0 1 2 3     [ - - ]         [ - - - -   - ]
        //  o o o o | o o - - | x x x x | - - - - | - o o o | o o o o
        //                ^ ^ [ - - - - ]
        //                p p      4
        //

        void block_resize(block_id_t *lhs, block_id_t *rhs, const u_int32_t size_rhs_b) {
            if (*lhs == nullptr && *rhs == nullptr) {
                abort("block_resize called with nullptr");
                return;
            }

            if (*rhs == nullptr) {
                // Split L-HS block, allocate new R-HS (mostly by shrinking L-HS)

                if (size_rhs_b == 0) {
                    // nothing ever happens, im all in
                    return;
                }

                const int          index_lhs  = blocks_.index_at(*lhs);
                const memory_block block_lhs  = blocks_.val_entries[index_lhs];
                const int32_t      lhs_remain = block_lhs.size_b - size_rhs_b;

                if (lhs_remain == 0) {
                    // rhs block takes full space of lhs
                    *rhs = *lhs;
                    *lhs = nullptr;
                    return;
                }

                if (lhs_remain < 0) {
                    abort("rhs memory block size out of bounds");
                    return;
                }

                // split L-HS block into two parts, shrink L-HS, make second one R-HS

                const block_id_t rhs_ptr = static_cast<unsigned char *>(*lhs) + lhs_remain;

                blocks_.val_entries[index_lhs].size_b = lhs_remain;

                blocks_.key_entries.insert(index_lhs + 1, rhs_ptr);
                blocks_.val_entries.insert(index_lhs + 1, { size_rhs_b, 1, true });

                *rhs = rhs_ptr;
                return;
            }

            if (*lhs == nullptr) {
                // Split R-HS block, allocate new L-HS (mostly by shrinking R-HS)

                const int          index_rhs  = blocks_.index_at(*rhs);
                const memory_block block_rhs  = blocks_.val_entries[index_rhs];
                const int32_t      lhs_remain = block_rhs.size_b - size_rhs_b;

                if (size_rhs_b == 0) {
                    // L-HS block takes full space of R-HS
                    *lhs = *rhs;
                    *rhs = nullptr;
                    return;
                }

                if (lhs_remain < 0) {
                    abort("rhs memory block size out of bounds");
                    return;
                }

                // split R-HS block into two parts, shrink R-HS, make second one R-HS

                const block_id_t rhs_ptr = static_cast<unsigned char *>(*rhs) + lhs_remain;

                blocks_.key_entries[index_rhs]        = rhs_ptr;
                blocks_.val_entries[index_rhs].size_b = size_rhs_b;

                blocks_.key_entries.insert(index_rhs, rhs);
                blocks_.val_entries.insert(index_rhs, { static_cast<u_int32_t>(lhs_remain), 1, true });

                *lhs = *rhs;
                *rhs = rhs_ptr;
                return;
            }

            // Resizing two existing blocks

            const int index_lhs = blocks_.index_at(*lhs);
            const int index_rhs = blocks_.index_at(*rhs);

            const memory_block block_lhs = blocks_.val_entries[index_lhs];
            const memory_block block_rhs = blocks_.val_entries[index_rhs];

            if (const block_id_t ptr_should = static_cast<unsigned char *>(*lhs) + block_lhs.size_b; ptr_should != rhs) {
                abort("memory fragmentation error: next block misalignment");
            }

            if (const block_id_t ptr_should = static_cast<unsigned char *>(*rhs) - block_rhs.size_b; ptr_should != lhs) {
                abort("memory fragmentation error: next block misalignment");
            }

            if (size_rhs_b == 0) {
                // merge rhs into lhs

                blocks_.val_entries[index_lhs].size_b += block_rhs.size_b;
                blocks_.collapse_at_pos(index_rhs);
                *rhs = nullptr;
                return;
            }

            const int32_t size_have = block_rhs.size_b + block_lhs.size_b;
            const int32_t size_need = size_have > size_rhs_b ? size_have : size_rhs_b;
            const int32_t new_lhs_b = size_have - size_rhs_b;
            const int32_t new_rhs_b = size_rhs_b;

            if (size_need > size_have) {
                abort("rhs memory block size out of bounds");
                return;
            }

            // resize / collapse
            blocks_.val_entries[index_rhs].size_b = static_cast<u_int32_t>(new_rhs_b);
            blocks_.val_entries[index_lhs].size_b = static_cast<u_int32_t>(new_lhs_b);

            if (new_lhs_b <= 0) {
                blocks_.key_entries[index_rhs] = static_cast<unsigned char *>(*lhs);
                blocks_.collapse_at_pos(index_lhs);
                *rhs = *lhs;
                *lhs = nullptr;
                return;
            }

            const block_id_t new_rhs_ptr   = static_cast<unsigned char *>(*lhs) + new_lhs_b;
            blocks_.key_entries[index_rhs] = new_rhs_ptr;
            *rhs                           = new_rhs_ptr;
        }

        void block_free(const block_id_t block) {
            const int index_block = blocks_.index_at(block);
            blocks_.val_entries[index_block].alignment_b = 1;
            blocks_.val_entries[index_block].is_free     = true;

            // . . n 1 2 . .  [1 <- 2]
            // . . n 1 _ . .  [n <- 1]
            // . . n _ _ . .

            if (const int index_next = index_block + 1; index_next <= blocks_.size - 1) {
                if (const memory_block next = blocks_.val_entries[index_next]; next.is_free == true) {
                    block_merge(blocks_.key_entries[index_next], block);
                }
            }

            if (const int index_prev = index_block - 1; index_prev > 0) {
                if (const memory_block prev = blocks_.val_entries[index_prev]; prev.is_free == true) {
                    block_merge(block, blocks_.key_entries[index_prev]);
                }
            }
        }

        block_id_t block_split(const block_id_t src, const u_int32_t size_new_b) {
            block_id_t cpy = src;
            block_id_t dst = nullptr;
            block_resize(&cpy, &dst, size_new_b);
            return dst;
        }

        void block_merge(block_id_t src, block_id_t dst) {
            block_resize(&dst, &src, 0);
        }
    };


}

#endif //EX_DATA_SIMPALLOC_H
