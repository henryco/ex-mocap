//
// Created by xd on 2/28/25.
//

#include <iostream>
#include <doctest/doctest.h>
#include "../../extra/graph/graph.h"
//#include "../../extra/data/alloc/rbtalloc.h"
#include "../../extra/data/alloc/lalloc.h"
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

TEST_SUITE("Grap node tests") {

    TEST_CASE("Graph allocation test") {
//        ex::data::rbtalloc::allocator alloc_2;
        ex::data::lalloc::allocator alloc_2;
        limbo::graph::memory::allocator = alloc_2.compat();

        ex::time::measure_avg time = 1;
        time.start();
        limbo::graph::BaseNode *root = new node_test_one();
        rec_build(root, 6, 7);
        limbo::graph::Clock clock;
        clock.attach(root);
        time.stop();

        // TODO

        std::cout << "build: " << time.result() << " ns" << std::endl;
        std::cout << "total: " << counter_ << std::endl;
        std::cout << "size t: " << alloc_2.size_total() << ", " << (alloc_2.size_total() / (1 MB)) << std::endl;
        std::cout << "size u: " << alloc_2.size_used() << ", " << (alloc_2.size_used() / (1 MB)) << std::endl;

        unsigned long int avg = 0;
        double counter = 0.;
//        ex::time::measure_avg measure = 1000;
//        for (int i = 0; i < 1000000; i++) {
//            measure.start();
//
//            clock.pull();
//
//            if (measure.stop()) {
//                avg += measure.result();
//                counter++;
//            }
//        }

//        {
//            ex::time::measure_avg measure_2 = 1;
//            measure_2.start();
//            alloc_2.trim();
//            measure_2.stop();
//            std::cout << "trim_time: " << measure_2.result() << " ns\n";
//        }

        std::cout << "avg: " << (avg / counter) << " ns\n";
        std::cout << "memory total : " << limbo::graph::memory::allocator.size_total(&limbo::graph::memory::allocator) << " b" << std::endl;
        std::cout << "memory used  : " << limbo::graph::memory::allocator.size_used(&limbo::graph::memory::allocator) << " b" << std::endl;
        std::cout << "blocks total: " << alloc_2.blocks_total() << std::endl;
        std::cout << "blocks free: " << (alloc_2.blocks_total() - alloc_2.blocks_used()) << std::endl;
        std::cout << "finished" << std::endl;
    }
}
