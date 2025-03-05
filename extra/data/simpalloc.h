//
// Created by xd on 3/4/25.
//

#ifndef EX_DATA_SIMPALLOC_H
#define EX_DATA_SIMPALLOC_H

#include "data_utils.h"
#include <iostream>
#include <cstring>

namespace ex::data::simpalloc {

    inline void abort(const char *msg) {
        std::cerr << msg << std::endl;
        ::abort();
    }

    // ---- DEFAULT FALLBACK (malloc/realloc)
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

        arena(const std::size_t &reserve) {
            expand(reserve);
        }

        void expand(const std::size_t n) {
            reserve(capacity + n);
        }

        void reserve(const std::size_t n) {
            if (n <= capacity)
                return;

            capacity = n;

            if (data == nullptr) data = ::malloc(capacity);
            else                 data = ::realloc(data, capacity);
        }

        void clean() {
            offset   = 0;
            capacity = 0;
            if (data == nullptr)
                return;
            ::free(data);
        }

        void *request(const std::size_t size, const std::size_t alignment = 1, std::size_t *padding = nullptr,
                      int64_t *moved = nullptr) {

            std::size_t padding_ = 0;

            if (offset % alignment != 0)
                padding_ = alignment - (offset % alignment);

            const std::size_t required = padding_ + size;
            const long int    remains  = capacity - offset;

            const void *old_data_ptr = data;

            if (required > remains)
                expand(capacity + required);

            const void *new_data_ptr = data;

            offset += required;

            if (padding != nullptr) {
                *padding = padding_;
            }

            if (moved != nullptr) {
                if (old_data_ptr == new_data_ptr)
                    *moved = 0;
                else
                    *moved = reinterpret_cast<unsigned long int>(new_data_ptr) -
                             reinterpret_cast<unsigned long int>(old_data_ptr);
            }

            return static_cast<unsigned char *>(data) + offset - size;
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

#define DEFAULT_ARENA_SIZE_B 1048576

        ex::data::map<block_id_t, memory_block> blocks_;
        ex::data::map<watch_id_t, watch_fn_t>   watche_;
        arena                                   memory_;

        allocator(const std::size_t pre_allocated = DEFAULT_ARENA_SIZE_B) {
            if (pre_allocated <= 0)
                abort("Allocation size must be greater than 0");
            memory_ = pre_allocated;
            memory_.request(pre_allocated);
            blocks_.put(memory_.data, { static_cast<u_int32_t>(pre_allocated), 1, true });
        }

        void *request_memory(const std::size_t size, const std::size_t alignment = 1, std::size_t *padding = nullptr) {
            int64_t offset;
            void *allocated = memory_.request(size, alignment, padding, &offset);

            // TODO

            if (offset == 0)
                return allocated;

            for (int i = 0; i < watche_.size; ++i) {
                if (const auto fn = watche_.val_entries[i])
                    fn(watche_.key_entries[i], offset);
            }

            return allocated;
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

                const int          index_lhs  = blocks_.index_at(*lhs);
                const memory_block block_lhs  = blocks_.val_entries[index_lhs];
                const int32_t      lhs_remain = size_rhs_b - block_lhs.size_b;

                if (lhs_remain == 0) {
                    // rhs block takes full space of lhs
                    *rhs = *lhs;
                    *lhs = nullptr;
                    return;
                }

                if (lhs_remain < 0) {

                    if (index_lhs != blocks_.size - 1) {
                        // cannot expand block in the middle of arena
                        abort("lhs memory block size out of bounds");
                        return;
                    }

                    // expand last block size
                    const void *allocated = request_memory((-1) * lhs_remain);

                    if (static_cast<unsigned char *>(*lhs) + block_lhs.size_b != allocated) {
                        abort("memory allocation error: next block misaligned");
                    }

                    // rhs block takes full space of expanded lhs
                    blocks_.val_entries[index_lhs].size_b = size_rhs_b;

                    *rhs = *lhs;
                    *lhs = nullptr;

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

                    if (index_rhs != blocks_.size - 1) {
                        // cannot expand block in the middle of arena
                        abort("rhs memory block size out of bounds");
                        return;
                    }

                    // expand last block size
                    const void *allocated = request_memory((-1) * lhs_remain);

                    if (static_cast<unsigned char *>(*rhs) + block_rhs.size_b != allocated) {
                        abort("memory allocation error: next block misaligned");
                    }

                    // expanded R-HS block new size
                    blocks_.val_entries[index_rhs].size_b = size_rhs_b;
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
                if (index_rhs != blocks_.size - 1) {
                    // cannot expand in the middle of arena
                    abort("rhs memory block size out of bounds");
                    return;
                }

                // expand last block size
                const void *allocated = request_memory(size_need - size_have);

                if (static_cast<unsigned char *>(*rhs) + block_rhs.size_b != allocated) {
                    abort("memory allocation error: next block misaligned");
                }

                // merge lhs into rhs, collapse lhs
                blocks_.key_entries[index_rhs]        = lhs;
                blocks_.val_entries[index_rhs].size_b = size_rhs_b;

                blocks_.collapse_at_pos(index_lhs);
                *rhs = *lhs;
                *lhs = nullptr;
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

        block_id_t block_split(block_id_t src, const u_int32_t size_new_b) {
            block_id_t dst = nullptr;
            block_resize(&src, &dst, size_new_b);
            return dst;
        }

        void block_merge(block_id_t src, block_id_t dst) {
            block_resize(&dst, &src, 0);
        }
    };


}

#endif //EX_DATA_SIMPALLOC_H
