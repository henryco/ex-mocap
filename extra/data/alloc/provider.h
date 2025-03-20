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
            data = nullptr;
        }

        long size() const {
            return offset;
        }

        void *data_ptr() const {
            return data;
        }

        void *request(const std::size_t size, std::size_t *size_allocated = nullptr) {
            const std::size_t curr_offset = offset;
            const std::size_t real_size   = capacity + size;

            const bool is_nullptr = data == nullptr;

            const auto  old_offset   = reinterpret_cast<unsigned long int>(data);
            {
                capacity += real_size;
                offset   += real_size;
                if (data == nullptr) data = ::malloc(capacity);
                else                 data = ::realloc(data, capacity);
            }
            const auto  new_offset   = reinterpret_cast<unsigned long int>(data);

            if (!is_nullptr && old_offset != new_offset) {
                abort_("discontinuity (TODO)");
            }

            if (size_allocated != nullptr) {
                *size_allocated = real_size;
            }

            return static_cast<unsigned char *>(data) + curr_offset;
        }
    };
}

#endif //EX_LIMBO_DATA_MEMPROVIDER_H
