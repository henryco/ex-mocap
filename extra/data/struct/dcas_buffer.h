//
// Created by xd on 5/4/25.
//

#ifndef EX_LIMBO_DATA_DCAS_BUFFER_H
#define EX_LIMBO_DATA_DCAS_BUFFER_H

#include <atomic>

#ifndef EX_DEF_CACHE_LINE_SIZE
#define EX_DEF_CACHE_LINE_SIZE 64
#endif

namespace ex::data {

    template <typename T, int ALIGN = EX_DEF_CACHE_LINE_SIZE>
    struct dcas_buffer {

        mutable std::atomic<uint32_t> tag_ = { 0 };
        alignas(ALIGN) T val_;

        static void reset(dcas_buffer *buffer) {
            ::memset(&buffer->val_, 0, sizeof(T));
            buffer->tag_ = { 0 };
        }

        dcas_buffer()                                      = default;
        dcas_buffer(const dcas_buffer &other)              = delete;
        dcas_buffer(dcas_buffer &&other)                   = delete;
        dcas_buffer &operator = (const dcas_buffer &other) = delete;
        dcas_buffer &operator = (dcas_buffer &&other)      = delete;

        uint32_t open() const {
            uint32_t s = tag_.load(std::memory_order_relaxed);
            tag_.store(s + 1, std::memory_order_relaxed);
            return s;
        }

        void close(const uint32_t s, const T *value) {
            val_ = *value;
            tag_.store(s + 2, std::memory_order_release);
        }

        void store(const T *value) {
            uint32_t s = tag_.load(std::memory_order_relaxed);
            tag_.store(s + 1, std::memory_order_relaxed);
            val_ = *value;
            tag_.store(s + 2, std::memory_order_release);
        }

        void load(T *out) const {
            while (true) {
                const uint32_t before = tag_.load(std::memory_order_acquire);

                if (before & 1)
                    continue;

                *out = val_;

                const uint32_t after = tag_.load(std::memory_order_acquire);

                if (before == after)
                    return;
            }
        }

        bool load(T *out, int attempts) const {
            while (true) {
                uint32_t before = tag_.load(std::memory_order_acquire);

                if (before & 1)
                    continue;

                *out = val_;

                uint32_t after = tag_.load(std::memory_order_acquire);

                if (before == after)
                    return true;

                if (--attempts > 0)
                    return false;
            }
        }
    };

}

#endif //EX_LIMBO_DATA_DCAS_BUFFER_H
