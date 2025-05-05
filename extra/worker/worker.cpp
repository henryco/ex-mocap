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

        using command_buff_t = ex::data::dcas_buffer<command_t>;
        using status_buff_t  = ex::data::dcas_buffer<status_t>;
        using spinlock_t     = ex::data::spinlock;

        struct command_t {
            const void (*fn)(void *);
            void    *args;
            unsigned id;
            status_t status;
        };

        struct buffers_t {
            command_buff_t command; // host   ->  worker
            status_buff_t  status;  // worker <-  host
        };

        struct record_t {
            command_t *command; // allocated outside
            buffers_t *buffers; // allocated outside
            channel_t *channel; // allocated outside
        };

        struct workload_t {
            #define WL_SZ 127
            record_t data[WL_SZ]; // stack allocated
            unsigned size;
        };
    }

    struct channel_t {
        worker_t  *worker_ptr; // allocated outside
        buffers_t *buffer_ptr; // allocated outside
        unsigned   counter;
    };

    struct worker_t {
        workload_t    jobs;
        ::pthread_t   pthr;
        spinlock_t    spl1;
        spinlock_t    spl2;
        volatile bool live;
    };

    namespace {
        void *worker_routine(void *arg) {
            worker_t *ctx = static_cast<worker_t *>(arg);
            if (ctx == nullptr)
                ::abort(); // wtf

            while (ctx->live) {
                ctx->spl2.lock();

                workload_t *work = &ctx->jobs;
                for (int i = 0; i < work->size; ++i) {

                    record_t  *job          = &work->data[i];
                    command_t *command_have = job->command;

                    command_t command_in;
                    job->buffers->command.load(&command_in);

                    if (command_in.id == command_have->id)
                        continue;

                    if (command_have->args != nullptr)
                        ::free(command_have->args);

                    if (command_in.status & status_t::aborted) {

                        command_have->id = command_in.id;
                        if (command_in.args != nullptr)
                            ::free(command_in.args);

                        constexpr status_t total_abort = status_t::finished | status_t::aborted;
                        command_have->status           = total_abort;
                        job->buffers->status.store(&total_abort);
                        continue;
                    }

                    if (command_in.status & status_t::in_queue) {

                        // TODO

                        command_have->id = command_in.id;
                        if (command_in.args != nullptr)
                            ::free(command_in.args);

                        constexpr status_t total_success = status_t::finished | status_t::success;
                        command_have->status             = total_success;
                        job->buffers->status.store(&total_success);
                        continue;
                    }

                    command_have->id = command_in.id;
                    if (command_in.args != nullptr)
                        ::free(command_in.args);

                    constexpr status_t total_failure = status_t::finished | status_t::failure;
                    command_have->status             = total_failure;
                    job->buffers->status.store(&total_failure);
                }

                ctx->spl2.unlock();
            }

            return nullptr;
        }

        void *copy_mem_(const void *src, const unsigned size) {
            void *mem = ::malloc(size);
            ::memcpy(mem, src, size);
            return mem;
        }
    }

    worker_t *create_worker() {
        worker_t *instance = static_cast<worker_t *>(::malloc(sizeof(worker_t)));
        ::memset(instance, 0, sizeof(worker_t));
        spinlock_t::reset(&instance->spl1);
        spinlock_t::reset(&instance->spl2);
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
                if (size >= WL_SZ) {
                    // not enough space
                    p->spl2.unlock();
                    p->spl1.unlock();
                    return nullptr;
                }

                command_t *command = static_cast<command_t *>(::malloc(sizeof(command_t)));
                ::memset(command, 0, sizeof(command_t));

                p->jobs.data[size].command = command;
                p->jobs.data[size].buffers = buffers;
                p->jobs.data[size].channel = channel;

                p->spl2.unlock();
            }
            p->spl1.unlock();
        }

        return channel;
    }

    void enqueue(channel_t *ch_ptr, const void (*fn)(void *), const void *arg, const unsigned arg_sz_b) {
        const command_t command = {
            .fn = fn,
            .args = copy_mem_(arg, arg_sz_b),
            .id = ++(ch_ptr->counter),
            .status = status_t::in_queue
        };
        ch_ptr->buffer_ptr->command.store(&command);
    }

    void enqueue(channel_t *ch_ptr, const void (*fn)(void *)) {
        const command_t command = {
            .fn = fn,
            .args = nullptr,
            .id = ++(ch_ptr->counter),
            .status = status_t::in_queue
        };
        ch_ptr->buffer_ptr->command.store(&command);
    }

    void stop(channel_t *ch_ptr) {
        const command_t command = { .id = ++(ch_ptr->counter), .status = status_t::aborted };
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
                        if (curr_ptr->buffers != nullptr)
                            ::free(curr_ptr->buffers);
                        if (curr_ptr->command != nullptr) {
                            if (curr_ptr->command->args != nullptr)
                                ::free(curr_ptr->command->args);
                            ::free(curr_ptr->command);
                        }

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

        if (force)
            ::pthread_cancel(p->pthr);

        p->live = false;
        p->spl1.lock();
        p->spl2.lock();

        for (int i = 0; i < p->jobs.size; ++i) {
            if (p->jobs.data[i].buffers != nullptr)
                ::free(p->jobs.data[i].buffers);
            if (p->jobs.data[i].channel != nullptr)
                ::free(p->jobs.data[i].channel);
            if (p->jobs.data[i].command != nullptr) {
                if (p->jobs.data[i].command->args != nullptr)
                    ::free(p->jobs.data[i].command->args);
                ::free(p->jobs.data[i].command);
            }
        }

        ::free(p);
    }

    status_t status(channel_t *ch_ptr) {
        status_t s;
        ch_ptr->buffer_ptr->status.load(&s);
        return s;
    }

    status_t status(channel_t *ch_ptr, const int retries) {
        status_t s;
        ch_ptr->buffer_ptr->status.load(&s, retries);
        return s;
    }

}
