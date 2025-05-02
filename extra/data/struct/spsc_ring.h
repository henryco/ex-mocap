//
// Created by xd on 5/2/25.
//

#ifndef EX_LIMBO_DATA_SPSC_RING_H
#define EX_LIMBO_DATA_SPSC_RING_H

#ifndef EX_DEF_CACHE_LINE_SIZE
#define EX_DEF_CACHE_LINE_SIZE 64
#endif

namespace ex::data {

    // Single Producer - Single Consumer Ring Buffer for efficient lock-free inter-threading communication
    template <typename T, int N = 64, int ALIGN = EX_DEF_CACHE_LINE_SIZE>
    struct alignas(EX_DEF_CACHE_LINE_SIZE) spsc_ring {
        volatile int head_ = 0;
        volatile int tail_ = 0;
        struct alignas(ALIGN) {
            T item;
        } buffer_[N];

        spsc_ring()                                    = default;
        spsc_ring(const spsc_ring &other)              = delete;
        spsc_ring(spsc_ring &&other)                   = delete;
        spsc_ring &operator = (const spsc_ring &other) = delete;
        spsc_ring &operator = (spsc_ring &&other)      = delete;

        bool push(const T *item) {
            const int next = (head_ + 1) % N;
            if (next == tail_)
                return false;
            buffer_[head_].item = *item;
            head_               = next;
            return true;
        }

        bool pop(T* item) {
            if (head_ == tail_)
                return false;
            *item = buffer_[tail_].item;
            tail_ = (tail_ + 1) % N;
            return true;
        }
    };

}

#endif //EX_LIMBO_DATA_SPSC_RING_H
