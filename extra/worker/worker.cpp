//
// Created by henryco on 03/05/25.
//

#include "worker.h"

#include <cstring>
#include <cstdlib>

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
            const void *args;
            unsigned    size;
            status_t    status;
        };

        struct buffers_t {
            command_buff_t command; // host   ->  worker
            status_buff_t  status;  // worker <-  host
        };

        struct record_t {
            command_t *command; // allocated outside
            buffers_t *buffers; // allocated outside
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
    };

    struct worker_t {
        workload_t jobs;
        spinlock_t spl1;
        spinlock_t spl2;
    };

    worker_t *create_worker() {
        worker_t *instance = static_cast<worker_t *>(::malloc(sizeof(worker_t)));
        ::memset(instance, 0, sizeof(worker_t));
        spinlock_t::reset(&instance->spl1);
        spinlock_t::reset(&instance->spl2);
        return instance;
    }

    channel_t *create_channel(worker_t *p) {
        channel_t *instance = nullptr;
        buffers_t *buffers  = nullptr;

        if (p->spl1.lock()) {
            instance  = static_cast<channel_t *>(::malloc(sizeof(channel_t)));
            buffers   = static_cast<buffers_t *>(::malloc(sizeof(buffers_t)));
            instance->buffer_ptr = buffers;
            instance->worker_ptr = p;

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

                p->jobs.data[size].buffers = buffers;
                p->jobs.data[size].command = command;

                p->spl2.unlock();
            }
            p->spl1.unlock();
        }

        return instance;
    }

    void enqueue(channel_t *ch_ptr, const void (*fn)(void *), const void *arg, const unsigned arg_sz_b) {
        void *mem = ::malloc(arg_sz_b);
        ::memcpy(mem, arg, arg_sz_b);
        const command_t command = { .fn = fn, .args = mem, .size = arg_sz_b, .status = status_t::in_queue };
        ch_ptr->buffer_ptr->command.store(&command);
    }

    void stop(channel_t *ch_ptr) {
        constexpr command_t command = { .status = status_t::aborted };
        ch_ptr->buffer_ptr->command.store(&command);
    }

    void close_channel(channel_t *ch_ptr) {
        if (worker_t *wpt = ch_ptr->worker_ptr;
            wpt->spl1.lock()) {
            if (wpt->spl2.lock()) {

                // TODO update array

                wpt->spl2.unlock();
            }
            wpt->spl1.unlock();
        }
    }

    void release_worker(worker_t *p) {
        if (p == nullptr) return;
        // TODO
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
