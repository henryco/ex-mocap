//
// Created by xd on 05/12/24.
//

#ifndef EX_MOTION_NODES_H
#define EX_MOTION_NODES_H

namespace ex::nodes {

    class IO {
    public:
        virtual ~IO() = default;
    };

    class OUT_ : virtual public IO {
    public:
        ~OUT_() override = default;

        virtual void *out(int output_index, int clock_i, std::size_t* size) = 0;
    };

    class IN_ : virtual public IO {
    public:
        ~IN_() override = default;

        virtual bool attach_any(IO *io_ptr, int output_index, int input_index) = 0;

        virtual bool detach_any(IO *output_ptr, int output_index, int input_index) = 0;

        virtual bool detach_all(int input_index) = 0;

        virtual bool is_attached(IO *io_ptr, int input_index) = 0;

        virtual bool is_attached(int input_index) = 0;
    };

    class Indexer {
    public:
        virtual ~Indexer() = default;

        virtual std::vector<int> indexes() const = 0;

        virtual bool has(int idx) const = 0;
    };

    template<typename O>
    class Output: virtual public OUT_ {
    public:
        ~Output() override = default;

        virtual O out(O *null_ptr, int output_index, int clock_i) = 0;
    };

    template<typename I>
    class Input : virtual public IN_ {
    public:
        ~Input() override = default;

        bool attach_io(IO *io_ptr, int output_index, int input_index) {
            auto ptr = dynamic_cast<Output<I> *>(io_ptr);
            if (ptr == nullptr)
                return false;
            return attach(ptr, output_index, input_index);
        }

        bool detach_io(IO *io_ptr, int output_index, int input_index) {
            auto ptr = dynamic_cast<Output<I> *>(io_ptr);
            if (ptr == nullptr)
                return false;
            return detach(ptr, output_index, input_index);
        }

        virtual bool attach(Output<I> *output_ptr, int output_index, int input_index) = 0;

        virtual bool detach(Output<I> *output_ptr, int output_index, int input_index) = 0;

    };

    template<typename T>
    class InIndexer : public Indexer {
        using type = T;
    public:
        ~InIndexer() override = default;
    };

    template<typename T>
    class OutIndexer : public Indexer {
        using type = T;
    public:
        ~OutIndexer() override = default;
    };

    template<typename O, int N>
    class NodeOut : public virtual Output<O> {
    public:
        ~NodeOut() override = default;

        O output(int clock_i) {
            return this->out(static_cast<O *>(nullptr), N, clock_i);
        }
    };

    template<typename I, int N>
    class NodeIn : public virtual Input<I> {
    public:
        ~NodeIn() override = default;

        bool add_io(IO *io_ptr, const int output_index) {
            auto ptr = dynamic_cast<Output<I> *>(io_ptr);
            if (ptr == nullptr)
                return false;
            return add_output(ptr, output_index);
        }

        bool remove_io(IO *io_ptr, const int output_index) {
            auto ptr = dynamic_cast<Output<I> *>(io_ptr);
            if (ptr == nullptr)
                return false;
            return remove_output(ptr, output_index);
        }

        bool add_output(Output<I> *output_ptr, const int output_index) {
            return this -> attach(output_ptr, output_index, N);
        }

        bool remove_output(Output<I> *output_ptr, const int output_index) {
            return this -> detach(output_ptr, output_index, N);
        }

        template<int i>
        bool add_node(NodeOut<I, i> *output_ptr) {
            return this -> attach(output_ptr, i, N);
        }

        template<int i>
        bool remove_node(NodeOut<I, i> *output_ptr) {
            return this -> detach(output_ptr, i, N);
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

    template<typename ... Types>
        class AnyIn : virtual public IN_ {
    public:
        ~AnyIn() override = default;

        bool attach_any(IO *io_ptr, const int output_index, const int input_index) override {
            bool result = false;
            bool stop = false;
            ((_attach_io<Types>(io_ptr, output_index, input_index, &result, &stop)), ...);
            return result;
        }

        bool detach_any(IO *io_ptr, const int output_index, const int input_index) override {
            bool result = false;
            bool stop = false;
            ((_detach_io<Types>(io_ptr, output_index, input_index, &result, &stop)), ...);
            return result;
        }

    private:
        template<typename T>
        void _attach_io(IO *io_ptr, const int output_index, const int input_index, bool *result, bool *stop) {
            if (*stop)
                return;

            auto in = dynamic_cast<Input<T> *>(this);
            if (in == nullptr) {
                *stop = false;
                return;
            }

            *result = in->attach_io(io_ptr, output_index, input_index);
            *stop = true;
        }

        template<typename T>
        void _detach_io(IO *io_ptr, const int output_index, const int input_index, bool *result, bool *stop) {
            if (*stop)
                return;

            auto in = dynamic_cast<Input<T> *>(this);
            if (in == nullptr) {
                *stop = false;
                return;
            }

            *result = in->detach_io(io_ptr, output_index, input_index);
            *stop = true;
        }
    };

    template<typename T>
    class InputStore : public virtual Input<T> {
    public:
        ~InputStore() override = default;

        bool attach(Output<T> *output_ptr, const int output_index, const int input_index) override {
            std::cout << "attach: " << output_index << ", " << input_index << '\n';
            return true;
        }

        bool detach(Output<T> *output_ptr, const int output_index, const int input_index) override {
            return true;
        }

        Output<T> *get_connection(const int input_index) {
            return nullptr;
        }
    };

    template<typename ... Types>
    class KeepInput;

    template <typename ... Types>
    class KeepInput : public InputStore<Types>... {
    public:
        ~KeepInput() override = default;

        template<typename T>
        Output<T> *connection(const int input_index) {
            return static_cast<InputStore<T>*>(this)->get_connection(input_index);
        }
    };

    template<typename... Types>
    class KeepIn : public AnyIn<Types...>, public KeepInput<Types...> {
    public:
        ~KeepIn() override = default;
    };

    class BaseNode : public virtual IN_, public virtual OUT_ {
    public:
        ~BaseNode() override = default;
    };

    template <typename N>
    class Node : public virtual BaseNode {
    public:
        ~Node() override = default;

        template <typename T>
        T pull(const int index, const int clock_i) {
            return static_cast<Output<T> *>(static_cast<N *>(this))->out(static_cast<T *>(nullptr), index, clock_i);
        }

        template <typename T, int index>
        T pull(const int clock_i) {
            return static_cast<NodeOut<T, index> *>(static_cast<N *>(this))->output(clock_i);
        }

        template <typename T>
        bool attach_node(Output<T> *out_ptr, const int output_index, const int input_index) {
            return static_cast<Input<T> *>(static_cast<N *>(this))->attach(out_ptr, output_index, input_index);
        }

        template<typename T, int input_index>
        bool attach_node(Output<T> *out_ptr, const int output_index) {
            return static_cast<NodeIn<T, input_index> *>(static_cast<N *>(this))->add_output(out_ptr, output_index);
        }

        template<typename T, int input_index, int output_index>
        bool attach_node(NodeOut<T, output_index> *out_ptr) {
            return static_cast<NodeIn<T, input_index> *>(static_cast<N *>(this))->add_node(out_ptr);
        }

        template <typename T>
        bool detach_node(Output<T> *out_ptr, const int output_index, const int input_index) {
            return static_cast<Input<T> *>(static_cast<N *>(this))->detach(out_ptr, output_index, input_index);
        }

        template<typename T, int input_index>
        bool detach_node(Output<T> *out_ptr, const int output_index) {
            return static_cast<NodeIn<T, input_index> *>(static_cast<N *>(this))->remove_output(out_ptr, output_index);
        }

        template<typename T, int input_index, int output_index>
        bool detach_node(NodeOut<T, output_index> *out_ptr) {
            return static_cast<NodeIn<T, input_index> *>(static_cast<N *>(this))->remove_node(out_ptr);
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
