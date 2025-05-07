//
// Created by henryco on 03/05/25.
//

#include "worker.h"

#include <cstring>
#include <cstdlib>
#include <pthread.h>

#include "../data/struct/dcas_buffer.h"
#include "../data/struct/spinlock.h"

namespace limbo::worker {

    namespace {
        struct command_t;
        struct record_t;
        struct result_t;

        using command_buff_t = ex::data::dcas_buffer<command_t>;
        using status_buff_t  = ex::data::dcas_buffer<result_t>;
        using spinlock_t     = ex::data::spinlock;

        struct command_t {
            void *(*fn)(void *);
            void    *args;          // managed outside
            unsigned counter;       // from channel ctr
            status_t status;
        };

        struct result_t {
            void    *result;        // managed outside
            status_t status;
        };

        struct buffers_t {
            command_buff_t command; // host   ->  worker
            status_buff_t  status;  // worker <-  host
        };

        struct record_t {
            buffers_t *buffers;     // allocated outside
            channel_t *channel;     // allocated outside
            unsigned   counter;     // used as command id
        };

        struct workload_t {
            #define WL_SZ 127
            record_t data[WL_SZ];   // stack allocated
            unsigned size;          // < WL_SZ
        };
    }

    struct channel_t {
        worker_t  *worker_ptr;      // allocated outside
        buffers_t *buffer_ptr;      // allocated outside
        unsigned   counter;         // used as command id
    };

    struct worker_t {
        workload_t    jobs;
        ::pthread_t   pthr;
        spinlock_t    spl1;
        spinlock_t    spl2;
        unsigned      idle;
        unsigned      atmp;
        volatile bool live;
    };

    namespace {
        constexpr result_t r_abort = { .result = nullptr, .status = status_t::finished | status_t::aborted };
        constexpr result_t r_fail  = { .result = nullptr, .status = status_t::finished | status_t::failure };

        #if defined(__GNUC__) || defined(__clang__)
            #define likely(x)   __builtin_expect(!!(x), 1)
            #define unlikely(x) __builtin_expect(!!(x), 0)
        #else
            #define likely(x) (x)
            #define unlikely(x) (x)
        #endif

        void *worker_routine(void *arg) {
            worker_t *ctx = static_cast<worker_t *>(arg);

            if (unlikely(ctx == nullptr))
                ::abort(); // wtf

            for (int attempts = 0; likely(ctx->live);) {
                ctx->spl2.lock();

                workload_t *work = &ctx->jobs;
                for (int i = 0; i < work->size; ++i) {

                    record_t *job = &work->data[i];

                    command_t command_in;
                    job->buffers->command.load(&command_in);

                    if (command_in.counter == job->counter)
                        continue;

                    attempts = 0;

                    job->counter = command_in.counter;

                    if (likely(command_in.status & status_t::in_queue)) {

                        if (unlikely(command_in.fn == nullptr))
                            goto point_of_failure;

                        const result_t result = {
                            .result = command_in.fn(command_in.args),
                            .status = status_t::finished | status_t::success
                        };

                        job->buffers->status.store(&result);
                        continue;
                    }

                    if (unlikely(command_in.status & status_t::aborted)) {
                        job->buffers->status.store(&r_abort);
                        continue;
                    }

                    {
                    point_of_failure:
                        job->buffers->status.store(&r_fail);
                    }
                }

                ctx->spl2.unlock();

                if (ctx->atmp == 0 || ctx->idle == 0)
                    continue;

                if (attempts++ > ctx->atmp)
                    ::usleep(ctx->idle);
            }

            return nullptr;
        }
    }

    worker_t *create_worker(const unsigned idle_us, const unsigned idle_after) {
        worker_t *instance = static_cast<worker_t *>(::malloc(sizeof(worker_t)));
        ::memset(instance, 0, sizeof(worker_t));
        spinlock_t::reset(&instance->spl1);
        spinlock_t::reset(&instance->spl2);
        instance->atmp = idle_after;
        instance->idle = idle_us;
        instance->live = true;

        ::pthread_t thread;
        ::pthread_create(&thread, nullptr, &worker_routine, instance);
        ::pthread_detach(thread);
        instance->pthr = thread;

        return instance;
    }

    channel_t *create_channel(worker_t *p) {
        channel_t *channel = nullptr;
        buffers_t *buffers = nullptr;

        if (p->spl1.lock()) {
            channel = static_cast<channel_t *>(::malloc(sizeof(channel_t)));
            buffers = static_cast<buffers_t *>(::malloc(sizeof(buffers_t)));
            channel->buffer_ptr = buffers;
            channel->worker_ptr = p;
            channel->counter    = 0;

            command_buff_t::reset(&buffers->command);
            status_buff_t::reset(&buffers->status);

            if (p->spl2.lock()) {

                const unsigned size = p->jobs.size;
                if (unlikely((size >= WL_SZ))) {
                    // not enough space
                    p->spl2.unlock();
                    p->spl1.unlock();
                    return nullptr;
                }

                p->jobs.data[size].buffers = buffers;
                p->jobs.data[size].channel = channel;
                p->jobs.size++;

                p->spl2.unlock();
            }
            p->spl1.unlock();
        }

        return channel;
    }

    void enqueue(channel_t *ch_ptr, void *(*fn)(void *), void *arg) {
        static constexpr result_t temp = { .result = nullptr, .status = status_t::in_queue };
        const command_t command = { .fn = fn, .args = arg, .counter = ++(ch_ptr->counter), .status = status_t::in_queue };
        ch_ptr->buffer_ptr->status.store(&temp);
        ch_ptr->buffer_ptr->command.store(&command);
    }

    void stop(channel_t *ch_ptr) {
        const command_t command = { .counter = ++(ch_ptr->counter), .status = status_t::aborted };
        ch_ptr->buffer_ptr->command.store(&command);
    }

    void close_channel(channel_t *ch_ptr) {
        if (ch_ptr == nullptr)
            return;

        if (worker_t *wpt = ch_ptr->worker_ptr;
            wpt->spl1.lock()) {
            if (wpt->spl2.lock()) {

                workload_t *work = &wpt->jobs;
                for (int i = 0; i < work->size; ++i) {
                    if (work->data[i].channel == ch_ptr) {

                        record_t *curr_ptr = &work->data[i];
                        if (likely(curr_ptr->buffers != nullptr))
                            ::free(curr_ptr->buffers);

                        record_t *last_ptr = &work->data[work->size - 1];
                        ::memcpy(curr_ptr, last_ptr, sizeof(record_t));
                        ::memset(last_ptr, 0, sizeof(record_t));
                        work->size--;

                        wpt->spl2.unlock();
                        wpt->spl1.unlock();

                        ::free(ch_ptr);
                        return;
                    }
                }

                wpt->spl2.unlock();
            }
            wpt->spl1.unlock();
        }
    }

    void close_worker(worker_t *p, const bool force) {
        if (p == nullptr)
            return;

        p->live = false;

        if (force) {
            if (p->spl1.lock(1000000) && p->spl2.lock(1000000))
                goto finish;
            ::pthread_cancel(p->pthr);
            p->spl1.lock(1000000);
            p->spl2.lock(1000000);
        } else {
            p->spl1.lock();
            p->spl2.lock();
        }

    finish:
        for (int i = 0; i < p->jobs.size; ++i) {
            if (likely(p->jobs.data[i].buffers != nullptr))
                ::free(p->jobs.data[i].buffers);
            if (likely(p->jobs.data[i].channel != nullptr))
                ::free(p->jobs.data[i].channel);
        }

        ::free(p);
    }

    status_t status(channel_t *ch_ptr) {
        result_t r;
        ch_ptr->buffer_ptr->status.load(&r);
        return r.status;
    }

    status_t status(channel_t *ch_ptr, const int retries) {
        result_t r;
        if (!ch_ptr->buffer_ptr->status.load(&r, retries))
            return status_t::timeout;
        return r.status;
    }

    status_t result(channel_t *ch_ptr, void **dst) {
        result_t r;
        ch_ptr->buffer_ptr->status.load(&r);
        *dst = r.result;
        return r.status;
    }

    status_t result(channel_t *ch_ptr, void **dst, const int retries) {
        result_t r;
        if (!ch_ptr->buffer_ptr->status.load(&r, retries))
            return status_t::timeout;
        *dst = r.result;
        return r.status;
    }

}
