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
        failure  = 1 << 3,
        aborted  = 1 << 4
    };

    worker create_worker();

    // blocking operation
    channel create_channel(worker);

    // blocking operation
    void close_channel(channel);

    // blocking operation
    void close_worker(worker, bool force = false);

    void enqueue(channel, const void(*)(void *), const void *arg, unsigned arg_sz_b);

    void stop(channel);

    status_t status(channel, int retries);

    status_t status(channel);

}

#endif //EX_LIMBO_WORKER_H
