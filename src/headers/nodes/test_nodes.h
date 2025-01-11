//
// Created by xd on 05/12/24.
//

#ifndef EX_MOTION_TEST_NODES_H
#define EX_MOTION_TEST_NODES_H

#include <iostream>


#include "nodes.h"

namespace ex::test {

    class TestSerializer : public nodes::Serializer<std::string> {
    public:
        std::string serialize(const std::vector<nodes::Serializable<std::string> *> &serializable) const override {
            return "ooo: " + serializable[0]->serialize();
        }

        std::vector<nodes::Serializable<std::string> *> deserialize(const std::string &serialized) override {
            return {};
        }
    };

    class Node1:
        public nodes::Node<Node1>,

        public nodes::id::name<"node_1">,

        public nodes::io::out<int, 0, "Output int 1">,
        public nodes::io::out<int, 1, "Output int 2">,
        public nodes::io::out<int, 2, "Output int 3">,
        public nodes::io::out<float, 3, "Output float 1">,
        public nodes::io::out<float, 4, "Output float 2">,
        public nodes::io::out<short, 5, "Output short 1">,
        public nodes::io::out<short, 6, "Output short 2">,
        public nodes::io::in<short, 7, "Input short 1">,
        public nodes::io::in<short, 8, "Input short 2">,

        public nodes::ReactiveIn<short>,
        public nodes::ReactiveOut<float, int, short>,

        public nodes::Serialization<TestSerializer>
    {
    public:
        void deserialize(const std::string &serialized) override {
            std::cout << "test serializer: " << serialized << std::endl;

            const TestSerializer serializer;

            std::vector<Serializable *> list;
            list.push_back(this);

            const auto result = serializer.serialize(list);
            std::cout << "test deserializer: " << result << std::endl;
        }

        std::string serialize() const override {
            return "ABC";
        }

        void *on_any_output(const int output_index, const int clock_i, std::size_t *size) override {
            // TODO
            return nullptr;
        }

        bool on_any_input(const void *data_ptr, const std::size_t size, const int input_index, const  int output_index, const int clock_i) override {
            // TODO
            return true;
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

        bool on_input(const short data, const int input_index, const int output_index, const int clock_i, Output<short> *io) override {
            // TODO
            return true;
        }
    };

    class Node2:
        public nodes::Node<Node2>,

        public nodes::id::name<"node_2">,

        public nodes::io::in<float, 0, "Input float 1">,
        public nodes::io::in<float, 1, "Input float 2">,
        public nodes::io::in<short, 2, "Input short 1">,
        public nodes::io::in<short, 3, "Input short 2">,
        public nodes::io::out<bool, 4, "Output bool">,
        public nodes::io::out<char, 5, "Output char">,

        public nodes::ReactiveIn<float, short>,
        public nodes::ReactiveOut<bool, char> {
    public:
        void *on_any_output(const int output_index, const int clock_i, std::size_t *size) override {
            // TODO
            return nullptr;
        }

        bool on_any_input(const void *data_ptr, const std::size_t size, const int input_index, const  int output_index, const int clock_i) override {
            // TODO
            return true;
        }

        bool on_output(bool *null_ptr, const int output_index, const int clock_i) override {
            // TODO
            return true;
        }

        char on_output(char *null_ptr, const int output_index, const int clock_i) override {
            // TODO
            return 'a';
        }

        bool on_input(float data, const int input_index, const int output_index, const int clock_i, Output<float> *io) override {
            // TODO
            return true;
        }

        bool on_input(short data, const int input_index, const int output_index, const int clock_i, Output<short> *io) override {
            // TODO
            return true;
        }
    };

}

#endif // EX_MOTION_TEST_NODES_H
