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

    struct channel_t;
    struct worker_t;

    using channel_p = channel_t *;
    using worker_p  = worker_t *;

    enum status_t : unsigned char {
        not_init = 0U,
        consumed = 1U << 0,
        finished = 1U << 1,
        success  = 1U << 2,
        aborted  = 1U << 3,
        failure  = 1U << 4,
        timeout  = 1U << 5
    };

    // Worker guarantee thread-safety for operations on channels (ie enqueue, stop)
    // as long as channels not being shared between threads.
    //
    // [ idle_after ] - # of busy-wait spins before idling
    // [ idle_fn ]    - idle routine
    worker_p create_worker(unsigned long idle_after = 0, void (*idle_fn)(worker_p) = nullptr);

    // blocking operation
    // Operations on channels are generally thread-safe
    // as long as channels not being shared between threads,
    // as it is assumed that any given thread can have many channels, but not vice versa.
    //
    // thread 1 ──> channel 1 ──────────┐
    // │ └────────> channel 2 ────────┐ │
    // └──────────> channel 3 ──────v v v
    //                              WORKER ──> thread_worker
    // thread 2 ──> channel 4 ────────^ ^
    // thread N ──> channel N ──────────┘
    channel_p create_channel(worker_p);

    // blocking operation
    void close_channel(channel_p);

    // blocking operation
    void close_worker(worker_p, bool force = false);

    // memory under *arg* is expected to be managed outside of worker
    void enqueue(channel_p, void *(*) (void *), void *arg = nullptr);

    // sends stop signal
    void stop(channel_p);

    // must use '&' and '|' operators in order to check status
    status_t status(channel_p);

    // must use '&' and '|' operators in order to check status
    status_t status(channel_p, int retries);

    // must use '&' and '|' operators in order to check status
    status_t result(channel_p, void **dst);

    // must use '&' and '|' operators in order to check status
    status_t result(channel_p, void **dst, int retries);

    // status OR
    status_t operator | (status_t lhs, status_t rhs);

    // status AND
    status_t operator & (status_t lhs, status_t rhs);
}

#endif //EX_LIMBO_WORKER_H
