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

        nodes::Description const *describe_inputs(int *o_inputs_number) const override {
            static const nodes::Description inputs_[] = {
                nodes::describe_io<short>(7, "Some short input")
            };
            *o_inputs_number = std::size(inputs_);
            return inputs_;
        }

        nodes::Description const *describe_outputs(int *o_outputs_number) const override {
            static const nodes::Description outputs_[] = {
                nodes::describe_io<int>(0, "Output int 1"),
                nodes::describe_io<int>(1, "Output int 2"),
                nodes::describe_io<int>(2, "Output int 3"),
                nodes::describe_io<float>(3, "Output float 1"),
                nodes::describe_io<float>(4, "Output float 2"),
                nodes::describe_io<short>(5, "Output short 1"),
                nodes::describe_io<short>(6, "Output short 2"),
            };
            *o_outputs_number = std::size(outputs_);
            return outputs_;
        }

        void *on_any_output(int output_index, int clock_i, std::size_t *size) override {
            // TODO
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
            // TODO
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

        nodes::Description const *describe_inputs(int *o_inputs_number) const override {
            static const nodes::Description inputs_[] = {
                nodes::describe_io<float>(0, "Input float 1"),
                nodes::describe_io<float>(1, "Input float 2"),
                nodes::describe_io<short>(2, "Input short 1"),
                nodes::describe_io<short>(3, "Input short 2"),
            };
            *o_inputs_number = std::size(inputs_);
            return inputs_;
        }

        nodes::Description const *describe_outputs(int *o_outputs_number) const override {
            static const nodes::Description outputs_[] = {
                nodes::describe_io<bool>(4, "Output bool"),
                nodes::describe_io<char>(5, "Output char"),
            };
            *o_outputs_number = std::size(outputs_);
            return outputs_;
        }

        void *on_any_output(int output_index, int clock_i, std::size_t *size) override {
            // TODO
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
            // TODO
            return true;
        }

        bool on_input(short data, const int input_index, const int output_index, const int clock_i) override {
            // TODO
            return true;
        }
    };

}

#endif // EX_MOTION_TEST_NODES_H
