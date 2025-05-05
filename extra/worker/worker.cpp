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
            command_t  state;
            buffers_t *buffers;
        };

        struct array_t {
            record_t data[127];
            unsigned size;
        };
    }

    struct channel_t {
        worker_t  *worker_ptr;
        buffers_t *buffers;
    };

    struct worker_t {
        spinlock_t spl_1;
        spinlock_t spl_2;
        array_t    records[2];
    };

    worker_t *create_worker() {
        worker_t *instance = static_cast<worker_t *>(::malloc(sizeof(worker_t)));
        ::memset(instance, 0, sizeof(worker_t));
        // TODO
        return instance;
    }

    channel_t *create_channel(worker_t *p) {
        channel_t *instance = nullptr;
        buffers_t *buffers  = nullptr;

        if (p->spl_1.lock()) {
            instance  = static_cast<channel_t *>(::malloc(sizeof(channel_t)));
            buffers   = static_cast<buffers_t *>(::malloc(sizeof(buffers_t)));
            instance->buffers    = buffers;
            instance->worker_ptr = p;

            command_buff_t::reset(&buffers->command);
            status_buff_t::reset(&buffers->status);

            if (p->spl_2.lock()) {

                // TODO update array

                p->spl_2.unlock();
            }
            p->spl_1.unlock();
        }

        return instance;
    }

    void enqueue(channel_t *ch_ptr, const void (*fn)(void *), const void *arg, const unsigned arg_sz_b) {
        void *mem = ::malloc(arg_sz_b);
        ::memcpy(mem, arg, arg_sz_b);
        const command_t command = { .fn = fn, .args = mem, .size = arg_sz_b, .status = status_t::in_queue };
        ch_ptr->buffers->command.store(&command);
    }

    void stop(channel_t *ch_ptr) {
        constexpr command_t command = { .status = status_t::aborted };
        ch_ptr->buffers->command.store(&command);
    }

    void close(channel_t *ch_ptr) {
        if (worker_t *wpt = ch_ptr->worker_ptr;
            wpt->spl_1.lock()) {
            if (wpt->spl_2.lock()) {

                // TODO update array

                wpt->spl_2.unlock();
            }
            wpt->spl_1.unlock();
        }
    }

    void release_worker(worker_t *p) {
        if (p == nullptr) return;
        // TODO
        ::free(p);
    }

    status_t status(channel_t *ch_ptr) {
        status_t s;
        ch_ptr->buffers->status.load(&s);
        return s;
    }

    status_t status(channel_t *ch_ptr, const int retries) {
        status_t s;
        ch_ptr->buffers->status.load(&s, retries);
        return s;
    }

}
