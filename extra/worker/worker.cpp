//
// Created by henryco on 03/05/25.
//

#include "worker.h"

#include <cstring>
#include <cstdlib>

#include "../data/struct/dcas_buffer.h"

namespace limbo::worker {

    namespace {
        struct command_t;
        struct record_t;

        using command_buff_t = ex::data::dcas_buffer<command_t>;
        using status_buff_t  = ex::data::dcas_buffer<status_t>;

        struct command_t {
            const void (*fn)(void *);
            const void *args;
            unsigned    size;
            status_t    status;
        };

        struct record_t {
            command_t      state;
            command_buff_t *command_ptr; // host   ->  worker
            status_buff_t  *status_ptr;  // worker <-  host
        };

        struct array_t {
            record_t data[128];
            unsigned size;
        };
    }

    struct channel_t {
        command_buff_t *command_buff_ptr; // host   ->  worker
        status_buff_t  *status_buff_ptr;  // worker <-  host
    };

    struct worker_t {
        array_t records[2];

    };

    worker_t *create_worker() {
        worker_t *instance = static_cast<worker_t *>(::malloc(sizeof(worker_t)));
        ::memset(instance, 0, sizeof(worker_t));
        // TODO
        return instance;
    }

    channel_t *create_channel(worker_t *p) {
        channel_t *instance = static_cast<channel_t *>(::malloc(sizeof(channel_t)));

        // TODO
        return instance;
    }

    void enqueue(channel_t *ch_ptr, const void (*fn)(void *), const void *arg, const unsigned arg_sz_b) {
        void *mem = ::malloc(arg_sz_b);
        ::memcpy(mem, arg, arg_sz_b);
        const command_t command = { .fn = fn, .args = mem, .size = arg_sz_b, .status = status_t::in_queue };
        ch_ptr->command_buff_ptr->store(&command);
    }

    void stop(channel_t *ch_ptr) {
        constexpr command_t command = { .status = status_t::aborted };
        ch_ptr->command_buff_ptr->store(&command);
    }

    void close(channel_t *ch_ptr) {
        constexpr command_t command = { .status = status_t::closed };
        ch_ptr->command_buff_ptr->store(&command);
    }

    void release_worker(worker_t *p) {
        if (p == nullptr) return;
        // TODO
        ::free(p);
    }

    status_t status(channel_t *ch_ptr) {
        status_t s;
        ch_ptr->status_buff_ptr->load(&s);
        return s;
    }

    status_t status(channel_t *ch_ptr, const int retries) {
        status_t s;
        ch_ptr->status_buff_ptr->load(&s, retries);
        return s;
    }

}
