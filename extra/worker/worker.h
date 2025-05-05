//
// Created by henryco on 03/05/25.
//

#ifndef EX_LIMBO_WORKER_H
#define EX_LIMBO_WORKER_H

namespace limbo::worker {

    struct channel_t;
    struct worker_t;

    using channel = channel_t *;
    using worker  = worker_t *;

    enum status_t : unsigned char {
        not_init = 0,
        in_queue = 1 << 0,
        finished = 1 << 1,
        success  = 1 << 2,
        aborted  = 1 << 3,
        failure  = 1 << 4,
    };

    worker create_worker();

    // blocking operation
    channel create_channel(worker);

    // blocking operation
    void close_channel(channel);

    // blocking operation
    void close_worker(worker, bool force = false);

    // memory under *arg* is expected to be managed outside of worker
    void enqueue(channel, const void(*)(void *), void *arg = nullptr);

    // sends stop signal
    void stop(channel);

    status_t status(channel, int retries);

    status_t status(channel);


    /** OR */
    constexpr status_t operator | (const status_t &lhs, const status_t &rhs) {
        return static_cast<status_t>(static_cast<unsigned char>(lhs) | static_cast<unsigned char>(rhs));
    }

    /** AND */
    constexpr status_t operator & (const status_t &lhs, const status_t &rhs) {
        return static_cast<status_t>(static_cast<unsigned char>(lhs) & static_cast<unsigned char>(rhs));
    }

    /** contains */
    constexpr bool operator >>= (const status_t &lhs, const status_t &rhs) {
        return (static_cast<unsigned char>(lhs) & static_cast<unsigned char>(rhs)) != 0;
    }
}

#endif //EX_LIMBO_WORKER_H
