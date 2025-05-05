//
// Created by xd on 5/5/25.
//

#ifndef EX_LIMBO_DATA_SPINLOCK_H
#define EX_LIMBO_DATA_SPINLOCK_H

#include <atomic>

namespace ex::data {

    struct spinlock {
        std::atomic_flag flag_ = ATOMIC_FLAG_INIT;

        bool lock() {
            while (flag_.test_and_set(std::memory_order_acquire)) {}
            return true;
        }

        bool lock(int attempts) {
            --attempts;
            while (flag_.test_and_set(std::memory_order_acquire)) {
                if (--attempts < 0)
                    return false;
            }
            return true;
        }

        void unlock() {
            flag_.clear(std::memory_order_release);
        }
    };

}

#endif //EX_LIMBO_DATA_SPINLOCK_H
