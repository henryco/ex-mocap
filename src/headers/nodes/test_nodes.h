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

        public nodes::In<short, 7, 8>
    {
    public:
        short out(short *null_ptr, int index, int clock_i) override {
            return 1;
        }

        float out(float *null_ptr, int index, int clock_i) override {
            return 2.f;
        }

        int out(int *null_ptr, int index, int clock_i) override {
            return 3;
        }

        void attach(Output<short> *output_ptr, int index) override {

        }

        void detach(Output<short> *output_ptr, int index) override {

        }
    };

    class Node2:
        public nodes::Node<Node2>,

        public nodes::In<float, 0, 1>,
        public nodes::In<short, 2, 3>,

        public nodes::Out<bool, 0>,
        public nodes::Out<char, 1>
    {
    public:
        char out(char *null_ptr, int index, int clock_i) override {
            return 'a';
        }

        bool out(bool *null_ptr, int index, int clock_i) override {
            return true;
        }

        void attach(Output<short> *output_ptr, int index) override {

        }

        void detach(Output<short> *output_ptr, int index) override {

        }

        void attach(Output<float> *output_ptr, int index) override {

        }

        void detach(Output<float> *output_ptr, int index) override {

        }
    };

}

#endif // EX_MOTION_TEST_NODES_H
