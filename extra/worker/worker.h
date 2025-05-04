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
        un_init   = 0,
        in_queue  = 1 << 0,
        completed = 1 << 1,
        aborted   = 1 << 2,
        failed    = 1 << 3,
        closed    = 1 << 4
    };

    worker create_worker();

    channel create_channel(worker);

    void enqueue(channel, const void(*)(void *), const void *arg, int arg_sz_b);

    void stop(channel);

    void close(channel);

    status_t status(channel, int retries);

    status_t status(channel);

    void release_worker(worker);

}

#endif //EX_LIMBO_WORKER_H
