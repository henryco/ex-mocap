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


    template<typename T>
    class Indexer {
    public:
        virtual ~Indexer() = default;

        virtual bool has_index(int idx) const = 0;
    };

    template<typename I, int... Indexes>
    class In : public Input<I> {
    public:
        virtual ~In() = default;

    };

    template<typename O, int... Indexes>
    class Out : public Output<O>, public Indexer<O> {
    public:
        virtual ~Out() = default;

        constexpr bool has_index(int idx) const override {
            return ((idx == Indexes) || ...);
        }
    };

    template <typename N>
    class Node {
    public:
        template <typename T>
        T pull(int index, int clock_i) {
            return static_cast<N*>(this)->out(static_cast<T*>(nullptr), index, clock_i);
        }

        template<typename T, int idx>
        constexpr void check() {
//            static_cast<Indexer<T> *>(static_cast<N *>(this))->has(idx);
//            static_assert((static_cast<Indexer<T> *>(static_cast<N *>(this))->has(idx)), "NOPE");

//            static_cast<Indexer<T> *>(static_cast<N *>(this))->has(idx);
        }
    };

}

#endif // EX_MOTION_NODES_H
