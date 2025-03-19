//
// Created by henryco on 18/03/25.
//

#ifndef EX_LIMBO_DATA_MEMPROVIDER_H
#define EX_LIMBO_DATA_MEMPROVIDER_H

#include <cstdlib>

namespace ex::data {

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
            const auto  old_offset   = reinterpret_cast<unsigned long int>(data);
            {
                capacity += real_size;
                offset   += real_size;
                if (data == nullptr) data = ::malloc(capacity);
                else                 data = ::realloc(data, capacity);
            }
            const auto  new_offset   = reinterpret_cast<unsigned long int>(data);

            if (addr_diff != nullptr && old_data_ptr != nullptr) {
                if (old_offset == new_offset) *addr_diff = 0;
                else                          *addr_diff = new_offset - old_offset;
            }

            if (size_allocated != nullptr) {
                *size_allocated = real_size;
            }

            return static_cast<unsigned char *>(data) + curr_offset;
        }
    };
}

#endif //EX_LIMBO_DATA_MEMPROVIDER_H
