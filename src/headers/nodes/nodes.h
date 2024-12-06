//
// Created by xd on 05/12/24.
//

#ifndef EX_MOTION_NODES_H
#define EX_MOTION_NODES_H

namespace ex::nodes {

    template<typename O>
    class Output {
    public:
        virtual ~Output() = default;

        virtual O out(O *null_ptr, int index, int clock_i) = 0;
    };

    template<typename I>
    class Input {
    public:
        virtual ~Input() = default;

        virtual void attach(Output<I> *output_ptr, int index) = 0;

        virtual void detach(Output<I> *output_ptr, int index) = 0;
    };

    class Indexer {
    public:
        virtual ~Indexer() = default;

        virtual std::vector<int> indexes() const = 0;

        virtual bool has(int idx) const = 0;
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


    template<typename I, int N>
    class NodeIn {

    };

    template<typename I, int... Indexes>
    class Inputs;

    template<typename I>
    class Inputs<I> {};

    template <typename I, int First, int... Rest>
    class Inputs<I, First, Rest...> : public NodeIn<I, First>, public Inputs<I, Rest...> {};



    template<typename O, int N>
    class NodeOut {

    };

    template<typename I, int... Indexes>
    class Outputs;

    template<typename I>
    class Outputs<I> {};

    template <typename O, int First, int... Rest>
    class Outputs<O, First, Rest...> : public NodeOut<O, First>, public Outputs<O, Rest...> {};



    template<typename I, int... Indexes>
    class In : public Input<I>, public InIndexer<I> {
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
    class Out : public Output<O>, public OutIndexer<O> {
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
            return static_cast<N*>(this)->out(static_cast<T*>(nullptr), index, clock_i);
        }

        template <typename T, int index>
        T pull(const int clock_i) {
            return static_cast<N*>(this)->out(static_cast<T*>(nullptr), index, clock_i);
        }

        template<typename T, int idx>
        constexpr bool has_output() {
            return static_cast<OutIndexer<T> *>(static_cast<N *>(this))->has(idx);
        }

        template<typename T>
        constexpr bool has_output(const int idx) {
            return static_cast<OutIndexer<T> *>(static_cast<N *>(this))->has(idx);
        }

        template<typename T, int idx>
        constexpr bool has_input() {
            return static_cast<InIndexer<T> *>(static_cast<N *>(this))->has(idx);
        }

        template<typename T>
        constexpr bool has_input(const int idx) {
            return static_cast<InIndexer<T> *>(static_cast<N *>(this))->has(idx);
        }
    };

}

#endif // EX_MOTION_NODES_H
