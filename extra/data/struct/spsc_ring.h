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
    struct spsc_ring {
        alignas(EX_DEF_CACHE_LINE_SIZE) volatile int head_ = 0;
        alignas(EX_DEF_CACHE_LINE_SIZE) volatile int tail_ = 0;
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

    // similar to spsc_ring, but never stops
    template <typename T, int N = 64, int ALIGN = EX_DEF_CACHE_LINE_SIZE>
    struct spsc_drop_old {
        using ulong_t = unsigned long int;

        ulong_t ctr__                                      = 0;
        alignas(EX_DEF_CACHE_LINE_SIZE) volatile int head_ = 0;
        alignas(EX_DEF_CACHE_LINE_SIZE) volatile int tail_ = 0;
        struct alignas(ALIGN) {
            ulong_t ctr_;
            T       item;
        } buffer_[N];

        spsc_drop_old()                                        = default;
        spsc_drop_old(const spsc_drop_old &other)              = delete;
        spsc_drop_old(spsc_drop_old &&other)                   = delete;
        spsc_drop_old &operator = (const spsc_drop_old &other) = delete;
        spsc_drop_old &operator = (spsc_drop_old &&other)      = delete;

        void push(const T *item) {
            int next = (head_ + 1) % N;
            if (next == tail_)
                next = (head_ + 2) % N;
            buffer_[head_].ctr_ = ++ctr__;
            buffer_[head_].item = *item;
            head_               = next;
        }

        ulong_t pop(T* item) {
            if (head_ == tail_)
                return 0;
            const ulong_t ctr = buffer_[tail_].ctr_;
            *item             = buffer_[tail_].item;
            tail_             = (tail_ + 1) % N;
            return ctr;
        }
    };

}

#endif //EX_LIMBO_DATA_SPSC_RING_H
