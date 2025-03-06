//
// Created by xd on 2/28/25.
//

#include <iostream>
#include <doctest/doctest.h>
#include "../../extra/graph/graph.h"
#include "../../extra/data/mempool.h"
#include "../../src/headers/utils/time_utils.h"

struct node_test_one final : limbo::graph::BaseNode,
                             limbo::graph::id::name<"node_test_one">,

                             limbo::graph::io::out<short, 1, "out 1 s">,
                             limbo::graph::io::out<short, 2, "out 2 s">,
                             limbo::graph::io::out<float, 3, "out 3 f">,
                             limbo::graph::io::out<short, 4, "out 4 s">,

                             limbo::graph::io::in<bool, 1, "in 1 b">,
                             limbo::graph::io::in<short, 2, "in 2 s">,
                             limbo::graph::io::in<float, 3, "in 3 f"> {
    short out_short = 0;
    float out_float = 0;

    void on_input(int in_idx, int out_idx, int clock_i, const void *data) override {

    }

    void on_output(const int in_idx, const int out_idx, const int clock_i, void *data) override {
        if (out_idx == 3)
            *(static_cast<float *>(data)) = out_float;
        else
            *(static_cast<short *>(data)) = out_short;
    }
};

struct node_test_two final : limbo::graph::BaseNode,
                             limbo::graph::id::name<"node_test_two">,

                             limbo::graph::io::out<short, 1, "out 1 s">,
                             limbo::graph::io::out<short, 2, "out 2 s">,
                             limbo::graph::io::out<float, 3, "out 3 f">,
                             limbo::graph::io::out<short, 4, "out 4 s">,

                             limbo::graph::io::in<bool, 1, "in 1 b">,
                             limbo::graph::io::in<short, 2, "in 2 s">,
                             limbo::graph::io::in<float, 3, "in 3 f"> {
    short out_short = 0;
    float out_float = 0;

    void on_input(int in_idx, int out_idx, int clock_i, const void *data) override {

    }

    void on_output(const int in_idx, const int out_idx, const int clock_i, void *data) override {
        if (out_idx == 3)
            *(static_cast<float *>(data)) = out_float;
        else
            *(static_cast<short *>(data)) = out_short;
    }
};

struct node_test_tri final : limbo::graph::BaseNode,
                             limbo::graph::id::name<"node_test_tri">,

                             limbo::graph::io::out<short, 1, "out 1 s">,
                             limbo::graph::io::out<short, 2, "out 2 s">,
                             limbo::graph::io::out<float, 3, "out 3 f">,
                             limbo::graph::io::out<short, 4, "out 4 s">,

                             limbo::graph::io::in<bool, 1, "in 1 b">,
                             limbo::graph::io::in<short, 2, "in 2 s">,
                             limbo::graph::io::in<float, 3, "in 3 f"> {
    short out_short = 0;
    float out_float = 0;

    void on_input(int in_idx, int out_idx, int clock_i, const void *data) override {

    }

    void on_output(const int in_idx, const int out_idx, const int clock_i, void *data) override {
        if (out_idx == 3)
            *(static_cast<float *>(data)) = out_float;
        else
            *(static_cast<short *>(data)) = out_short;
    }
};

static int counter_ = 0;
void rec_build(limbo::graph::BaseNode *parent, const int width, const int max_depth) {
    for (int i = 0; i < width; ++i) {
        counter_ += 1;

        limbo::graph::BaseNode *child;
        if (i < 0.3 * width)
            child = new node_test_one();
        else if (i < 0.66 * width)
            child = new node_test_two();
        else
            child = new node_test_tri();

        const bool which = (i < 0.5 * width);

        parent->attach(child, which ? 3 : 2, which ? 3 : 4);

        if (max_depth > 0)
            rec_build(parent, width, max_depth - 1);
    }
}

namespace data {
    static ex::data::mempool::allocator pool;

    inline void *pool_malloc_(const std::size_t size_new_b, const std::size_t alignment_b) {
        return pool.allocate(size_new_b, alignment_b);
    }

    inline void *pool_realloc_(void *ptr, const std::size_t size_new_b) {
        return pool.reallocate(ptr, size_new_b);
    }

    inline void pool_free_(void *ptr) {
        pool.free(ptr);
    }

    inline void pool_watch_add_(void *const ptr, const ex::data::watch_fn_t fn) {
        pool.add_watcher(ptr, fn);
    }

    inline void pool_watch_del_(void *const ptr, const ex::data::watch_fn_t fn) {
        pool.del_watcher(ptr, fn);
    }

    inline long pool_size_total_() {
        return pool.memory_total_b();
    }

    inline long pool_size_used_() {
        return pool.memory_used_b();
    }
}


TEST_CASE("Graph node test") {

    static ex::data::allocator allocator_ = {
        .malloc = &data::pool_malloc_,
        .realloc = &data::pool_realloc_,
        .free = &data::pool_free_,
        .add_watcher = &data::pool_watch_add_,
        .del_watcher = &data::pool_watch_del_,
        .size_total = &data::pool_size_total_,
        .size_used = &data::pool_size_used_,
    };

    limbo::graph::memory::allocator = allocator_;

    ex::time::measure_avg time = 1;
    time.start();
    limbo::graph::BaseNode *root = new node_test_one();
    rec_build(root, 4, 3);
    time.stop();

    std::cout << "build: " << time.result() << " ns" << std::endl;
    std::cout << "total: " << counter_ << std::endl;

    limbo::graph::Clock clock;
    clock.attach(root);

    unsigned long int avg = 0;
    double counter = 0.;
    ex::time::measure_avg measure = 1000;
    for (int i = 0; i < 1000000; i++) {
        measure.start();

        clock.pull();

        if (measure.stop()) {
            avg += measure.result();
            counter++;
        }
    }

    int free_blocks = 0;
    for (int i = 0; i < data::pool.blocks_.size; ++i) {
        const auto block = data::pool.blocks_.val_entries[i];
        if (block.is_free == true)
            free_blocks += 1;
    }

    std::cout << "avg: " << (avg / counter) << " ns\n";
    std::cout << "memory total : " << limbo::graph::memory::allocator.size_total() << " b" << std::endl;
    std::cout << "memory used  : " << limbo::graph::memory::allocator.size_used() << " b" << std::endl;
    std::cout << "blocks total: " << data::pool.blocks_.size << std::endl;
    std::cout << "blocks free: " << free_blocks << std::endl;
    std::cout << "finished" << std::endl;

}