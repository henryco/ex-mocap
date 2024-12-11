//
// Created by xd on 05/12/24.
//

#ifndef EX_MOTION_TEST_NODES_H
#define EX_MOTION_TEST_NODES_H

#include "nodes.h"

namespace ex::test {

    class Node1:
        public nodes::Node<Node1>,

        public nodes::Out<int, 0, 1, 2>,
        public nodes::Out<float, 3, 4>,
        public nodes::Out<short, 5, 6>,

        public nodes::In<short, 7, 8>,

        public nodes::ReactiveIn<short>,
        public nodes::ReactiveOut<float, int, short>
    {
    public:
        const int get_node_id() const override {
            return 1;
        }

        const char *get_node_name() const override {
            return "Node1";
        }

        void *out_any(const int output_index, const int clock_i, std::size_t *size) override {
            return nullptr;
        }

        float on_output(float *null_ptr, const int output_index, const int clock_i) override {
            // TODO
            return 3;
        }

        int on_output(int *null_ptr, const int output_index, const int clock_i) override {
            // TODO
            return 4;
        }

        short on_output(short *null_ptr, const int output_index, const int clock_i) override {
            // TODO
            return 5;
        }

        bool on_input(const short data, const int input_index, const int output_index, const int clock_i) override {
            return true;
        }
    };

    class Node2:
        public nodes::Node<Node2>,

        public nodes::Out<bool, 4>,
        public nodes::Out<char, 5>,

        public nodes::In<float, 0, 1>,
        public nodes::In<short, 2, 3>,

        public nodes::ReactiveIn<float, short>,
        public nodes::ReactiveOut<bool, char>
    {
    public:
        const int get_node_id() const override {
            return 2;
        }

        const char *get_node_name() const override {
            return "Node2";
        }

        void *out_any(const int output_index, const int clock_i, std::size_t *size) override {
            return nullptr;
        }

        bool on_output(bool *null_ptr, const int output_index, const int clock_i) override {
            // TODO
            return true;
        }

        char on_output(char *null_ptr, const int output_index, const int clock_i) override {
            // TODO
            return 'a';
        }

        bool on_input(float data, const int input_index, const int output_index, const int clock_i) override {
            return true;
        }

        bool on_input(short data, const int input_index, const int output_index, const int clock_i) override {
            return true;
        }
    };

}

#endif // EX_MOTION_TEST_NODES_H
