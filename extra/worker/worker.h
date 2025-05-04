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

    enum status:
    unsigned char {
        in_queue,
        completed,
        aborted,
        failed,
        closed
    };

    worker create_worker();

    channel create_channel(worker);

    void enqueue(channel, const void(*)(void *), const void *arg, int arg_sz_b);

    void stop(channel);

    void close(channel);

    status status(channel);

    void free_worker(worker);

    void free_channel(channel);

}

#endif //EX_LIMBO_WORKER_H
