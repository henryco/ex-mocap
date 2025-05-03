//
// Created by henryco on 03/05/25.
//

#include "worker.h"
#include "../data/struct/spsc_ring.h"

#include <stdlib.h>

namespace limbo::worker {

    struct channel_t {
        worker_t *worker_ptr;
    };

    struct worker_t {

    };

    worker_t *create_worker() {
        worker_t *instance = static_cast<worker_t *>(::malloc(sizeof(worker_t)));
        // TODO
        return instance;
    }

    channel_t *create_channel(worker_t *p) {
        channel_t *instance = static_cast<channel_t *>(::malloc(sizeof(channel_t)));
        instance->worker_ptr = p;
        // TODO
        return instance;
    }

    void enqueue(channel_t *, const void (*fn)(void *), void *arg, const int arg_sz_b) {

    }

    void stop(channel_t *) {

    }

    void close(channel_t *) {

    }

    void free_worker(worker_t *p) {
        if (p == nullptr) return;
        ::free(p);
    }

    void free_channel(channel_t *p) {
        if (p == nullptr) return;
        ::free(p);
    }

    enum status status(channel_t *) {
        return status::in_queue;
    }

}
