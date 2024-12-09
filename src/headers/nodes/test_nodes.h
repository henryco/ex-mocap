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
        void *out_any(const int output_index, const int clock_i, std::size_t *size) override {
            return nullptr;
        }

        short out(short *null_ptr, const int output_index, const int clock_i) override {
            return 1;
        }

        float out(float *null_ptr, const int output_index, const int clock_i) override {
            return 2.f;
        }

        int out(int *null_ptr, const int output_index, const int clock_i) override {
            return 3;
        }

        void on_input(short data, int clock_i, int input_index, int output_index) override {

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
        // OUTPUT
        char pull(const int index, const int clock_i) {
            update_inputs(clock_i);
            return out(static_cast<char *>(nullptr), index, clock_i);
        }

        void *out_any(const int output_index, const int clock_i, std::size_t *size) override {
            return nullptr;
        }

        char out(char *null_ptr, const int output_index, const int clock_i) override {
            return 'a';
        }

        bool out(bool *null_ptr, const int output_index, const int clock_i) override {
            return true;
        }

        void on_input(float data, int clock_i, int input_index, int output_index) override {

        }

        void on_input(short data, int clock_i, int input_index, int output_index) override {

        }
    };

}

#endif // EX_MOTION_TEST_NODES_H
