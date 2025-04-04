//
// Created by xd on 2/28/25.
//

#include <iostream>
#include <doctest/doctest.h>
#include "../../extra/graph/graph.h"
//#define USE_VALGRIND
//#define LALLOC_AGGRESSIVE
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
        if (i < 0.3 * width) {
            child = new
            (limbo::graph::memory::allocator.malloc(&limbo::graph::memory::allocator, sizeof(node_test_one), alignof(node_test_one)))
            node_test_one();
        }
        else if (i < 0.66 * width) {
            child = new
            (limbo::graph::memory::allocator.malloc(&limbo::graph::memory::allocator, sizeof(node_test_two), alignof(node_test_two)))
            node_test_two();
        }
        else {
            child = new
            (limbo::graph::memory::allocator.malloc(&limbo::graph::memory::allocator, sizeof(node_test_tri), alignof(node_test_tri)))
            node_test_tri();
        }

        const bool which = (i < 0.5 * width);

        parent->attach(child, which ? 3 : 2, which ? 3 : 4);

        if (max_depth > 0)
            rec_build(parent, width, max_depth - 1);
    }
}

TEST_SUITE("Grap node tests") {

    TEST_CASE("Graph allocation test") {
//        ex::data::rbtalloc::allocator alloc_2;
        ex::data::lalloc::allocator alloc_2 = { 1 MB };
        limbo::graph::memory::allocator = alloc_2.compat();

        {
            limbo::graph::BaseNode *root = new
            (limbo::graph::memory::allocator.malloc(&limbo::graph::memory::allocator, sizeof(node_test_one), alignof(node_test_one)))
            node_test_one();
            ex::time::measure_avg time = 1;
            time.start();

//            rec_build(root, 6, 7);
//            rec_build(root, 5, 5);
            rec_build(root, 4, 3);

            limbo::graph::Clock clock;
            clock.attach(root);
            time.stop();

            // TODO
            auto mem_t = limbo::graph::memory::allocator.size_total(&limbo::graph::memory::allocator);
            auto mem_u = limbo::graph::memory::allocator.size_used(&limbo::graph::memory::allocator);

            std::cout << "build: " << time.result() << " ns" << std::endl;
            std::cout << "total: " << counter_ << std::endl;
            std::cout << "size t: " << mem_t << ", " << (mem_t / (1 MB)) << std::endl;
            std::cout << "size u: " << mem_u << ", " << (mem_u / (1 MB)) << std::endl;
            std::cout << "ratio: " << (static_cast<double>(mem_u) / static_cast<double>(mem_t)) << std::endl;

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

            const auto total_ = alloc_2.blocks_total();
            const auto free_ = alloc_2.blocks_free();

            std::cout << "avg: " << (avg / counter) << " ns\n";
            std::cout << "memory total : " << mem_t << " b" << std::endl;
            std::cout << "memory used  : " << mem_u << " b" << std::endl;
            std::cout << "blocks total: " << total_ << std::endl;
            std::cout << "blocks free: " << free_ << std::endl;
            std::cout << "blocks used: " << (total_ - free_) << std::endl;
            std::cout << "utilization: " << (static_cast<double>(total_ - free_) / static_cast<double>(total_)) << std::endl;
            std::cout << "finished" << std::endl;
        }

        alloc_2.clean();
    }
}
