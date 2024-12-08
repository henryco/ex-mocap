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

        public nodes::KeepIn<short>
    {
    public:
        void *out(const int output_index, const int clock_i, std::size_t *size) override {
            return nullptr;
        }

        short out(short *null_ptr, const int index, const int clock_i) override {
            return 1;
        }

        float out(float *null_ptr, const int index, const int clock_i) override {
            return 2.f;
        }

        int out(int *null_ptr, const int index, const int clock_i) override {
            return 3;
        }

        bool is_attached(IO *io_ptr, int input_index) override {
            return false;
        }
    };

    class Node2:
        public nodes::Node<Node2>,

        public nodes::Out<bool, 4>,
        public nodes::Out<char, 5>,

        public nodes::In<float, 0, 1>,
        public nodes::In<short, 2, 3>,

        public nodes::KeepIn<float, short>
    {
    public:
        void *out(const int output_index, const int clock_i, std::size_t *size) override {
            auto a = this->connection<float>(1);
            auto b = this->connection<short>(2);
            return nullptr;
        }

        char out(char *null_ptr, const int index, const int clock_i) override {
            return 'a';
        }

        bool out(bool *null_ptr, const int index, const int clock_i) override {
            return true;
        }

        bool is_attached(IO *io_ptr, int input_index) override {
            return false;
        }
    };

}

#endif // EX_MOTION_TEST_NODES_H
