//
// Created by henryco on 03/05/25.
//

#include "../data/struct/spsc_ring.h"
#include "worker.h"

namespace limbo::worker {

    struct channel_t {
        worker_t *worker_ptr;
    };

    struct worker_t {

    };

    worker_t *create_worker() {
        return nullptr;
    }

    channel_t *create_channel(worker_t *) {
        return nullptr;
    }

    void enqueue(channel_t *, void (*)(void *), void *arg, int arg_sz_b) {

    }

    void stop(channel_t *) {

    }

    void close(channel_t *) {

    }

    void free_worker(worker_t *) {

    }

    void free_channel(channel_t *) {

    }

    enum status status(channel_t *) {
        return status::in_queue;
    }

}
