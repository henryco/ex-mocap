//
// Created by henryco on 03/05/25.
//

#ifndef EX_LIMBO_WORKER_H
#define EX_LIMBO_WORKER_H

// ---- Can be redefined if needed ----------
#ifndef EX_LIMBO_WORKER_CHANNELS_MAX
#define EX_LIMBO_WORKER_CHANNELS_MAX (127)
#endif
// ------------------------------------------

namespace limbo::worker {

#define EX_LIMBO_WORKER_IDLE_SPIN_DEFAULT (1UL << 26)

    struct channel_t;
    struct worker_t;

    using channel_p = channel_t *;
    using worker_p  = worker_t *;

    enum status_t : unsigned char {
        not_init = 0U,
        in_queue = 1U << 0,
        finished = 1U << 1,
        success  = 1U << 2,
        aborted  = 1U << 3,
        failure  = 1U << 4,
        timeout  = 1U << 5
    };

    worker_p create_worker(unsigned long idle_us = 0, unsigned long idle_after = EX_LIMBO_WORKER_IDLE_SPIN_DEFAULT);

    // blocking operation
    channel_p create_channel(worker_p);

    // blocking operation
    void close_channel(channel_p);

    // blocking operation
    void close_worker(worker_p, bool force = false);

    // memory under *arg* is expected to be managed outside of worker
    void enqueue(channel_p, void *(*) (void *), void *arg = nullptr);

    // sends stop signal
    void stop(channel_p);

    // must use & and | operators to check status
    status_t status(channel_p);

    // must use & and | operators to check status
    status_t status(channel_p, int retries);

    // must use & and | operators to check status
    status_t result(channel_p, void **dst);

    // must use & and | operators to check status
    status_t result(channel_p, void **dst, int retries);

    /** OR */
    constexpr status_t operator | (const status_t lhs, const status_t rhs) {
        return static_cast<status_t>(static_cast<unsigned char>(lhs) | static_cast<unsigned char>(rhs));
    }

    /** AND */
    constexpr status_t operator & (const status_t lhs, const status_t rhs) {
        return static_cast<status_t>(static_cast<unsigned char>(lhs) & static_cast<unsigned char>(rhs));
    }
}

#endif //EX_LIMBO_WORKER_H
