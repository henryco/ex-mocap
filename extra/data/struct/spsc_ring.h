//
// Created by xd on 5/2/25.
//

#ifndef EX_LIMBO_DATA_SPSC_RING_H
#define EX_LIMBO_DATA_SPSC_RING_H

namespace ex::data {

    template <typename T, int N = 256>
    struct spsc_ring {
        alignas(64) volatile int head = 0;
        alignas(64) volatile int tail = 0;
        T buffer[N];

        spsc_ring()                                    = default;
        spsc_ring(const spsc_ring &other)              = delete;
        spsc_ring(spsc_ring &&other)                   = delete;
        spsc_ring &operator = (const spsc_ring &other) = delete;
        spsc_ring &operator = (spsc_ring &&other)      = delete;

        bool push(const T *item) {
            const int next = (head + 1) % N;
            if (next == tail)
                return false;
            buffer[head] = *item;
            head         = next;
            return true;
        }

        bool pop(T* item) {
            if (head == tail)
                return false;
            *item = buffer[tail];
            tail  = (tail + 1) % N;
            return true;
        }
    };

}

#endif //EX_LIMBO_DATA_SPSC_RING_H
