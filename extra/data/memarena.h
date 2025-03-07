//
// Created by henryco on 07/03/25.
//

#ifndef EX_DATA_MEMARENA_H
#define EX_DATA_MEMARENA_H

#include "data_utils.h"

#include <cstring>
#include <iostream>

namespace ex::data::memarena {

    #ifndef UNIT_TEST
    #define abort_(msg) do { std::cerr << msg << std::endl; ::abort(); } while(false)
    #else
    #define abort_(msg) throw std::runtime_error(msg)
    #endif

    // ---- DEFAULT FALLBACK (malloc/realloc)
    // TODO: replace with direct call to OS
    struct mem_provider {

        void       *data     = nullptr;
        std::size_t offset   = 0;
        std::size_t capacity = 0;

        mem_provider() = default;

        mem_provider(const std::size_t capacity) {
            reserve(capacity);
        }

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

        void reserve(const int n) {
            if (n <= capacity)
                return;

            capacity = n;

            if (data == nullptr) data = ::malloc(capacity);
            else                 data = ::realloc(data, capacity);
        }

        void *request(const std::size_t size, std::size_t *size_allocated = nullptr, int64_t *addr_diff = nullptr) {
            const std::size_t curr_offset = offset;
            const std::size_t real_size   = capacity + size;

            const void *old_data_ptr = data;
            {
                if (size >= capacity) {
                    capacity = capacity > 0 ? (2 * capacity) : 2;

                    if (data == nullptr) data = ::malloc(capacity);
                    else                 data = ::realloc(data, capacity);
                }
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

    struct memory_block {
        u_int32_t size_b;
        u_int16_t alignment_b;
        u_int8_t  is_free;
    };

    struct allocator {
        ex::data::map<watch_id_t, watch_fn_t> watche_;
        mem_provider                          memory_;
        void                                 *last_ptr;
        u_int64_t                             last_size;

        #define DEFAULT_ARENA_SIZE_B 1048576

        allocator(const std::size_t pre_allocated = DEFAULT_ARENA_SIZE_B) {
            if (pre_allocated <= 0)
                abort_("Allocation size must be greater than 0");
            memory_   = pre_allocated;
            last_ptr  = nullptr;
            last_size = 0;
        }

        void cleanup() {
            last_ptr  = nullptr;
            last_size = 0;
            memory_.clean();
            watche_.clean();
        }

        void add_watcher(const watch_id_t id, const watch_fn_t fn) {
            watche_.put(id, fn);
        }

        void del_watcher(const watch_id_t id, watch_fn_t) {
            watche_.remove(id);
        }

        long memory_used_b() const {
            return memory_.offset;
        }

        long memory_total_b() const {
            return memory_.capacity;
        }

        void free(void *) const {/*NOP*/}

        u_int64_t block_padding(const u_int64_t alignment_b) const {
            if (alignment_b <= 1)
                return 0;
            const u_int64_t offset = memory_.offset;
            if (offset % alignment_b != 0)
                return alignment_b - (offset % alignment_b);
            return 0;
        }

        void *allocate(const std::size_t size_b, const std::size_t alignment_b) {
            if (size_b <= 0)
                abort_("Empty memory block allocation");

            const u_int64_t   padding        = block_padding(alignment_b);
            const std::size_t size_requested = size_b + padding;

            int64_t addr_diff;
            void *allocated = memory_.request(size_requested, nullptr, &addr_diff);

            if (addr_diff != 0) {
                for (int i = 0; i < watche_.size; ++i) {
                    if (const auto fn = watche_.val_entries[i])
                        fn(watche_.key_entries[i], addr_diff);
                }
            }

            void *ptr = static_cast<unsigned char *>(allocated) + padding;
            last_size = size_b;
            last_ptr  = ptr;
            return ptr;
        }

        void *reallocate(void *ptr, const std::size_t size_new_b) {
            if (size_new_b <= 0 || last_ptr == nullptr || ptr == nullptr)
                abort_("Empty memory block allocation");

            const void *old_data = memory_.data;
            const void *const last_ptr_ = last_ptr;
            void *allocated = allocate(size_new_b, 1);
            const void *new_data = memory_.data;

            const int64_t offset = reinterpret_cast<u_int64_t>(new_data) -
                                      reinterpret_cast<u_int64_t>(old_data);


            if (last_ptr_ != ptr) {
                const void *old_src = static_cast<const unsigned char *>(ptr) + offset;
                ::memcpy(allocated, old_src, last_size);
            }

            return allocated;
        }

    };

}

#endif //EX_DATA_MEMARENA_H
