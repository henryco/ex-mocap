//
// Created by xd on 2/28/25.
//

#include <doctest/doctest.h>
#include "../../extra/graph/graph.h"

struct node_test_one final : limbo::graph::BaseNode,
                             limbo::graph::id::name<"node_test_one">,

                             limbo::graph::io::out<short, 1, "out 1 s">,
                             limbo::graph::io::out<short, 2, "out 2 s">,
                             limbo::graph::io::out<float, 3, "out 3 f">,
                             limbo::graph::io::out<short, 4, "out 4 s">,

                             limbo::graph::io::in<bool, 1, "in 1 b">,
                             limbo::graph::io::in<char, 2, "in 2 c">,
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
                             limbo::graph::io::in<char, 2, "in 2 c">,
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
                             limbo::graph::io::in<char, 2, "in 2 c">,
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

TEST_CASE("Graph node test") {

    limbo::graph::Clock clock;

    // TODO
}