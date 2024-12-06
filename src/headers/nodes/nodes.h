//
// Created by xd on 05/12/24.
//

#ifndef EX_MOTION_NODES_H
#define EX_MOTION_NODES_H

#include <typeindex>
#include <typeinfo>

namespace ex::nodes {

    template <typename T>
    std::type_index get_type_index() {
        return std::type_index(typeid(T));
    }

    template<int N>
    class IndexedNode {
    public:
        virtual ~IndexedNode() = default;
    };

    class Indexer {
    public:
        virtual ~Indexer() = default;

        virtual std::vector<int> indexes() const = 0;

        virtual bool has(int idx) const = 0;
    };

    template<typename O>
    class Output {
    public:
        virtual ~Output() = default;

        virtual O out(O *null_ptr, int output_index, int clock_i) = 0;
    };

    template<typename I>
    class Input {
    public:
        virtual ~Input() = default;

        virtual bool attach(Output<I> *output_ptr, int output_index, int input_index) = 0;

        virtual bool detach(Output<I> *output_ptr, int output_index, int input_index) = 0;

        virtual bool detach_all(int input_index) = 0;
    };

    template<typename T>
    class InIndexer : public Indexer {
    public:
        ~InIndexer() override = default;
    };

    template<typename T>
    class OutIndexer : public Indexer {
    public:
        ~OutIndexer() override = default;
    };

    template<typename O, int N>
    class NodeOut : public virtual Output<O>, public virtual IndexedNode<N> {
    public:
        ~NodeOut() override = default;

        O output(int clock_i) { return this->out(static_cast<O *>(nullptr), N, clock_i); }
    };

    template<typename I, int N>
    class NodeIn : public virtual Input<I>, public virtual IndexedNode<N> {
    public:
        ~NodeIn() override = default;

        bool add_node(Output<I> *output_ptr, const int output_index) {
            return this-> attach(output_ptr, output_index, N);
        }

        bool remove_node(Output<I> *output_ptr, const int output_index) {
            return this-> detach(output_ptr, output_index, N);
        }

        template<int i>
        bool add_output(NodeOut<I, i> *output_ptr) {
            return this-> attach(output_ptr, i, N);
        }

        template<int i>
        bool remove_output(NodeOut<I, i> *output_ptr) {
            return this-> detach(output_ptr, i, N);
        }

        bool remove_all() {
            return this -> detach_all(N);
        }
    };

    template<typename I, int... Indexes>
    class Inputs;

    template<typename I>
    class Inputs<I> {};

    template <typename I, int First, int... Rest>
    class Inputs<I, First, Rest...>
        : public NodeIn<I, First>,
          public Inputs<I, Rest...>
    {
    public:
        ~Inputs() override = default;
    };

    template<typename O, int... Indexes>
    class Outputs;

    template<typename O>
    class Outputs<O> {};

    template <typename O, int First, int... Rest>
    class Outputs<O, First, Rest...>
        : public NodeOut<O, First>,
          public Outputs<O, Rest...> {
    public:
        ~Outputs() override = default;
    };



    template<typename I, int... Indexes>
    class In : public InIndexer<I>, public Inputs<I, Indexes...> {
    public:
        ~In() override = default;

        constexpr std::vector<int> indexes() const override {
            return {Indexes...};
        }

        constexpr bool has(int idx) const override {
            return ((idx == Indexes) || ...);
        }
    };

    template<typename O, int... Indexes>
    class Out : public OutIndexer<O>, public Outputs<O, Indexes...> {
    public:
        ~Out() override = default;

        constexpr std::vector<int> indexes() const override {
            return {Indexes...};
        }

        constexpr bool has(int idx) const override {
            return ((idx == Indexes) || ...);
        }
    };

    template <typename N>
    class Node {
    public:
        template <typename T>
        T pull(const int index, const int clock_i) {
            return static_cast<Output<T> *>(static_cast<N *>(this))->out(static_cast<T *>(nullptr), index, clock_i);
        }

        template <typename T, int index>
        T pull(const int clock_i) {
            return static_cast<NodeOut<T, index> *>(static_cast<N *>(this))->output(clock_i);
        }

        template<typename T, int input_index>
        bool attach_node(Output<T> *out_ptr, const int output_index) {
            return static_cast<NodeIn<T, input_index> *>(static_cast<N *>(this))->add_node(out_ptr, output_index);
        }

        template<typename T, int input_index, int output_index>
        bool attach_node(NodeOut<T, output_index> *out_ptr) {
            return static_cast<NodeIn<T, input_index> *>(static_cast<N *>(this))->add_output(out_ptr);
        }

        template<int input_index, int output_index>
        bool attach_node(IndexedNode<output_index> *out_node) {
            // TODO
            return true;
        }

        template<typename T, int input_index>
        bool detach_node(Output<T> *out_ptr, const int output_index) {
            return static_cast<NodeIn<T, input_index> *>(static_cast<N *>(this))->remove_node(out_ptr, output_index);
        }

        template<typename T, int input_index, int output_index>
        bool detach_node(NodeOut<T, output_index> *out_ptr) {
            return static_cast<NodeIn<T, input_index> *>(static_cast<N *>(this))->remove_output(out_ptr);
        }

        template<int input_index, int output_index>
        bool detach_node(IndexedNode<output_index> *out_node) {
            // TODO
            return true;
        }

        template<typename T, int input_index>
        bool detach_nodes() {
            return static_cast<NodeIn<T, input_index> *>(static_cast<N *>(this))->remove_all();
        }

        template<typename T, int idx>
        bool has_output() {
            return static_cast<OutIndexer<T> *>(static_cast<N *>(this))->has(idx);
        }

        template<typename T>
        bool has_output(const int idx) {
            return static_cast<OutIndexer<T> *>(static_cast<N *>(this))->has(idx);
        }

        template<typename T, int idx>
        bool has_input() {
            return static_cast<InIndexer<T> *>(static_cast<N *>(this))->has(idx);
        }

        template<typename T>
        bool has_input(const int idx) {
            return static_cast<InIndexer<T> *>(static_cast<N *>(this))->has(idx);
        }
    };

}

#endif // EX_MOTION_NODES_H
