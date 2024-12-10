//
// Created by xd on 05/12/24.
//

#ifndef EX_MOTION_NODES_H
#define EX_MOTION_NODES_H

#include <mutex>

namespace ex::nodes {

    class IO {
    public:
        virtual ~IO() = default;
    };

    class OUT_ : virtual public IO {
    public:
        ~OUT_() override = default;

        virtual void *out_any(int output_index, int clock_i, std::size_t* size) = 0;
    };

    class IN_ : virtual public IO {
    public:
        ~IN_() override = default;

        virtual bool attach_any(IO *io_ptr, int output_index, int input_index) = 0;

        virtual bool detach_any(IO *output_ptr, int output_index, int input_index) = 0;

        virtual bool is_attached_any(IO *io_ptr, int output_index, int input_index) = 0;

        virtual bool is_attached_any(int input_index) = 0;

        virtual void detach_all_any(int input_index) = 0;
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

        bool is_attached_io(IO *io_ptr, const int output_index, const int input_index) {
            auto ptr = dynamic_cast<Output<I> *>(io_ptr);
            if (ptr == nullptr)
                return false;
            return is_attached(ptr, output_index, input_index);
        }

        virtual bool attach(Output<I> *output_ptr, int output_index, int input_index) = 0;

        virtual bool detach(Output<I> *output_ptr, int output_index, int input_index) = 0;

        virtual bool is_attached(Output<I> *output_ptr, int output_index, int input_index) = 0;

        virtual bool is_attached(int input_index) = 0;

        virtual void detach_all(int input_index) = 0;
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

        void remove_all() {
            this -> detach_all(N);
        }

        template<int i>
        bool add_node(NodeOut<I, i> *output_ptr) {
            return this -> attach(output_ptr, i, N);
        }

        template<int i>
        bool remove_node(NodeOut<I, i> *output_ptr) {
            return this -> detach(output_ptr, i, N);
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

        bool is_attached_any(IO *io_ptr, const int output_index, const int input_index) override {
            bool result = false;
            bool stop = false;
            ((_is_attached_io<Types>(io_ptr, output_index, input_index, &result, &stop)), ...);
            return result;
        }

        void detach_all_any(const int input_index) override {
            ((_detach_all<Types>(input_index)), ...);
        }

        bool is_attached_any(const int input_index) override {
            bool result = false;
            bool stop = false;
            ((_is_attached_any<Types>(input_index, &result, &stop)), ...);
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

        template<typename T>
        void _is_attached_io(IO *io_ptr, const int output_index, const int input_index, bool *result, bool *stop) {
            if (*stop)
                return;

            auto in = dynamic_cast<Input<T> *>(this);
            if (in == nullptr) {
                *stop = false;
                return;
            }

            *result = in->is_attached_io(io_ptr, output_index, input_index);
            *stop = true;
        }

        template<typename T>
        void _detach_all(const int input_index) {
            auto in = dynamic_cast<Input<T> *>(this);
            if (in == nullptr)
                return;
            in->detach_all(input_index);
        }

        template<typename T>
        void _is_attached_any(const int input_index, bool *result, bool *stop) {
            if (*stop)
                return;

            auto in = dynamic_cast<Input<T> *>(this);
            if (in == nullptr) {
                *stop = false;
                return;
            }

            *result = in->is_attached(input_index);
            *stop = true;
        }
    };

    struct alignas(16) Metadata {
        int   output_index = -1; // 4
        int   input_index  = -1; // 4
        int   item_index   = -1; // 4
        short main_index   = -1; // 2
        short main_size    = -1; // 2
    };

    struct null_mutex {
        void lock() { /*empty*/ }
        void unlock() noexcept { /*empty*/ }
        bool try_lock() { return true; }
    };

    template<typename T>
    class InputStore : public virtual Input<T> {
    private:
        Output<T> *(**output_array_2) = nullptr;
        Metadata *metadata_array      = nullptr;
        int       metadata_count      = 0;
        int       output_count        = 0;

        // TODO MUTEX provider
        null_mutex mutex_;

    public:
        ~InputStore() override {
            if (metadata_array != nullptr) {
                free(metadata_array);
            }
            if (output_array_2 != nullptr) {
                for (int i = 0; i < output_count; i++) {
                    if (output_array_2[i] != nullptr)
                        free(output_array_2[i]);
                }
                free(output_array_2);
            }
        }

        const Metadata * get_metadata(int *return_size) {
            std::lock_guard guard(mutex_);

            if (metadata_array == nullptr || metadata_count <= 0) {
                *return_size = 0;
                return nullptr;
            }
            *return_size = metadata_count;
            return metadata_array;
        }

    private:
        void find_output_array(const int input_index, const int output_index, int *main_index_o, int *main_size_o, int *meta_index_o = nullptr) {
            if (input_index < 0
                || metadata_array == nullptr
                || output_array_2 == nullptr)
            {
                *main_index_o = -1;
                *main_size_o  = -1;
                if (meta_index_o != nullptr)
                    *meta_index_o = -1;
                return;
            }

            for (int i = 0; i < metadata_count; i++) {
                if (metadata_array[i].input_index == input_index
                    && ((output_index >= 0 && metadata_array[i].output_index == output_index)
                        || (output_index < 0 && metadata_array[i].output_index > (-1)))
                    && metadata_array[i].item_index   > (-1)
                    && metadata_array[i].main_index   > (-1)
                    && metadata_array[i].main_size    > (-1))
                {
                    const int main_index = metadata_array[i].main_index;
                    const int item_index = metadata_array[i].item_index;
                    const int main_size  = metadata_array[i].main_size;
                    if (output_array_2[main_index] != nullptr
                        && output_array_2[main_index][item_index] != nullptr)
                    {
                        *main_index_o = main_index;
                        *main_size_o  = main_size;
                        if (meta_index_o != nullptr)
                            *meta_index_o = i;
                        return;
                    }
                }
            }

            *main_index_o = -1;
            *main_size_o  = -1;
            if (meta_index_o != nullptr)
                *meta_index_o = -1;
        }

    public:

        Output<T> *get_connection(const Metadata &metadata) {
            std::lock_guard guard(mutex_);

            if (output_array_2 == nullptr || output_count <= 0)
                return nullptr;

            return output_array_2[metadata.main_index][metadata.item_index];
        }

        Output<T> **get_connections(const int input_index, int *out_size = nullptr) {
            std::lock_guard guard(mutex_);

            int main_index_o = -1;
            int main_size_o  = -1;
            find_output_array(input_index, -1, &main_index_o, &main_size_o);

            if (main_index_o == -1 || main_size_o == -1)
                return nullptr;

            if (out_size != nullptr)
                *out_size = main_size_o;

            return output_array_2[main_index_o];
        }

        void remove_connection(const int input_index) {
            detach_all(input_index);
        }

        bool is_attached(Output<T> *output_ptr, const int output_index, const int input_index) override {
            std::lock_guard guard(mutex_);
            if (output_ptr == nullptr)
                return false;

            int main_index = -1;
            int main_size  = -1;

            find_output_array(input_index, output_index, &main_index, &main_size);
            if (main_index < 0 || main_size < 0)
                return false;

            for (int i = 0; i < main_size; i++) {
                if (output_array_2[main_index][i] == output_ptr)
                    return true;
            }

            return false;
        }

        bool is_attached(const int input_index) override {
            std::lock_guard guard(mutex_);
            int main_index = -1;
            int main_size  = -1;
            find_output_array(input_index, -1, &main_index, &main_size);
            return !((main_index < 0) || (main_size < 0));
        }

        bool attach(Output<T> *output_ptr, const int output_index, const int input_index) override {
            std::lock_guard guard(mutex_);

            if (metadata_array == nullptr) {

                if (output_array_2 != nullptr) {
                    for (int i = 0; i < output_count; i++) {
                        if (output_array_2[i] != nullptr)
                            free(output_array_2[i]);
                    }
                    free(output_array_2);
                }

                metadata_array       = static_cast<Metadata *>(malloc(sizeof(Metadata)));
                output_array_2       = static_cast<Output<T> ** *>(malloc(sizeof(Output<T> **)));
                output_array_2[0]    = static_cast<Output<T> **>(malloc(sizeof(Output<T> *)));
                output_array_2[0][0] = output_ptr;

                metadata_array[0].output_index = output_index;
                metadata_array[0].input_index  = input_index;
                metadata_array[0].item_index   = 0;
                metadata_array[0].main_index   = 0;
                metadata_array[0].main_size    = 1;

                metadata_count     = 1;
                output_count       = 1;

                return true;
            }

            if (is_attached(output_ptr, output_index, input_index)) {
                return false;
            }

            int main_index = -1;
            int main_size  = -1;
            find_output_array(input_index, output_index, &main_index, &main_size);

            const int new_size = std::max(metadata_count + 1, 0);
            const int i        = metadata_count;

            if (main_index < 0 || main_size < 0) {
                const int size_2 = std::max(output_count + 1, 0);
                const int j      = output_count;

                metadata_array = static_cast<Metadata *>(realloc(metadata_array, new_size * sizeof(Metadata)));
                metadata_array[i].output_index = output_index;
                metadata_array[i].input_index  = input_index;
                metadata_array[i].main_index   = j;
                metadata_array[i].item_index   = 0;
                metadata_array[i].main_size    = 1;

                output_array_2       = static_cast<Output<T> ** *>(realloc(output_array_2, size_2 * sizeof(Output<T> **)));
                output_array_2[j]    = static_cast<Output<T> **>(malloc(sizeof(Output<T> *)));
                output_array_2[j][0] = output_ptr;

                metadata_count = new_size;
                output_count   = size_2;

                return true;
            }

            const int size_2 = main_size + 1;
            const int j      = main_index;
            const int k      = main_size;

            metadata_array = static_cast<Metadata *>(realloc(metadata_array, new_size * sizeof(Metadata)));
            metadata_array[i].output_index = output_index;
            metadata_array[i].input_index  = input_index;
            metadata_array[i].main_size    = size_2;
            metadata_array[i].main_index   = j;
            metadata_array[i].item_index   = k;

            output_array_2[j]    = static_cast<Output<T> **>(realloc(output_array_2[j], size_2 * sizeof(Output<T> *)));
            output_array_2[j][k] = output_ptr;

            metadata_count = new_size;

            for (int p = 0; p < metadata_count; p++) {
                if (metadata_array[p].input_index == input_index) {
                    metadata_array[p].main_size = size_2;
                }
            }

            return true;
        }

        bool detach(Output<T> *output_ptr, const int output_index, const int input_index) override {
            std::lock_guard guard(mutex_);

            if (output_ptr == nullptr) {
                return false;
            }

            int meta_index = -1;
            int main_index = -1;
            int main_size  = -1;

            find_output_array(input_index, output_index, &main_index, &main_size, &meta_index);
            if (main_index < 0 || main_size < 0) {
                return true;
            }

            int j = -1;
            for (int i = 0; i < main_size; i++) {
                if (output_array_2[main_index][i] == output_ptr) {
                    j = i;
                    break;
                }
            }

            if (j < 0) {
                return false;
            }

            const int size_1 = std::max(metadata_count - 1, 0);
            const int size_2 = std::max(main_size - 1, 0);
            const int l      = meta_index;
            const int k      = main_index;

            metadata_array[l]    = metadata_array[size_1];
            output_array_2[k][j] = output_array_2[k][size_2];

            metadata_array    = static_cast<Metadata *>(realloc(metadata_array, size_1 * sizeof(Metadata)));
            output_array_2[k] = static_cast<Output<T> **>(realloc(output_array_2[k], size_2 * sizeof(Output<T> *)));

            if (size_2 <= 0) {
                const int new_size = std::max(output_count - 1, 0);

                output_array_2[k] = output_array_2[new_size];
                output_array_2 = static_cast<Output<T> ** *>(realloc(output_array_2, new_size * sizeof(Output<T> **)));

                for (int p = 0; p < metadata_count; p++) {
                    if (metadata_array[p].main_index == new_size) {
                        metadata_array[p].main_index = k;
                    }
                }

                output_count = new_size;
            }

            metadata_count = size_1;

            for (int p = 0; p < metadata_count; p++) {
                if (metadata_array[p].input_index == input_index) {
                    metadata_array[p].main_size = size_2;
                }
            }

            if (metadata_count <= 0)
                metadata_array = nullptr;
            if (output_count <= 0)
                output_array_2 = nullptr;

            return false;
        }

        void detach_all(const int input_index) override {
            std::lock_guard guard(mutex_);
            if (!is_attached(input_index)) {
                return;
            }

            int main_index = -1;
            int main_size  = -1;

            find_output_array(input_index, -1, &main_index, &main_size);
            if (main_index < 0 || main_size < 0) {
                return;
            }

            int count = 0;
            for (int i = 0; i < metadata_count; i++) {
                if (metadata_array[i].input_index == input_index) {
                    count++;
                }
            }

            const int new_size_1 = std::max(metadata_count - count, 0);
            auto      new_meta_1 = static_cast<Metadata *>(malloc(new_size_1 * sizeof(Metadata)));

            int m = 0;
            for (int i = 0; i < metadata_count; i++) {
                if (metadata_array[i].input_index != input_index) {
                    new_meta_1[m++] = metadata_array[i];
                }
            }

            const int new_size = std::max(output_count - 1, 0);
            const int k        = main_index;

            output_array_2[k] = output_array_2[new_size];
            output_array_2 = static_cast<Output<T> ** *>(realloc(output_array_2, new_size * sizeof(Output<T> **)));


            free(metadata_array);
            metadata_array = new_meta_1;
            metadata_count = new_size_1;

            for (int p = 0; p < metadata_count; p++) {
                if (metadata_array[p].main_index == new_size) {
                    metadata_array[p].main_index = k;
                }
            }

            output_count = new_size;

            if (metadata_count <= 0)
                metadata_array = nullptr;
            if (output_count <= 0)
                output_array_2 = nullptr;
        }
    };

    template<typename ... Types>
    class KeepInput;

    template <typename ... Types>
    class KeepInput : public InputStore<Types>... {
    public:
        ~KeepInput() override = default;

        template<typename T>
        Output<T> **connections(const int input_index, int *out_size = nullptr) {
            return static_cast<InputStore<T>*>(this)->get_connections(input_index, out_size);
        }

        template<typename T>
        Output<T> *connection(const int input_index) {
            int size = -1;
            Output<T> **r = connections<T>(input_index, &size);
            return size <= 0 ? nullptr : r[0];
        }

        template<typename T>
        bool disconnect(const int input_index) {
            return static_cast<InputStore<T>*>(this)->remove_connection(input_index);
        }
    };

    template<typename... Types>
    class KeepIn : public AnyIn<Types...>, public KeepInput<Types...> {
    public:
        ~KeepIn() override = default;
    };

    template<typename T>
    class CallbackInput  {
    public:
        virtual ~CallbackInput() = default;

        virtual void on_input(T data, int input_index, int output_index, int clock_i) = 0;
    };

    class ReactiveState {
    public:
        virtual ~ReactiveState() = default;

        virtual void update_state(int clock_i, bool once) = 0;
    };

    template<typename ... Types>
    class ReactiveIn : public virtual ReactiveState, public KeepIn<Types...>, public virtual CallbackInput<Types>... {
    private:
        int last_clock_i = -1;

        template<typename T>
        void _pull_inputs(const int clock_i) {
            int             size     = -1;
            const Metadata *metadata = static_cast<InputStore<T> *>(this)->get_metadata(&size);
            for (int i = 0; i < size; i++) {
                const auto &meta       = metadata[i];
                Output<T>  *connection = static_cast<InputStore<T> *>(this)->get_connection(meta);
                const T     data       = connection->out(static_cast<T *>(nullptr), meta.output_index, clock_i);
                static_cast<CallbackInput<T> *>(this)->on_input(data, meta.input_index, meta.output_index, clock_i);
            }
        }

    public:
        ~ReactiveIn() override = default;

        void update_state(const int clock_i, const bool once = true) override {
            if (last_clock_i == clock_i && !once)
                return;
            ((_pull_inputs<Types>(clock_i)), ...);
            last_clock_i = clock_i;
        }

        void reset_internal_clock() {
            last_clock_i = -1;
        }
    };

    template<typename T>
    class ReactiveOutput : public virtual ReactiveState, public virtual Output<T> {
    public:
        ~ReactiveOutput() override = default;

        virtual T on_output(T *null_ptr, int output_index, int clock_i) = 0;

        T out(T *null_ptr, const int output_index, const int clock_i) override {
            update_state(clock_i, true);
            return on_output(null_ptr, output_index, clock_i);
        }
    };

    template<typename ... Types>
    class ReactiveOut : public ReactiveOutput<Types>... {
    public:
        ~ReactiveOut() override = default;
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
        void detach_nodes() {
            static_cast<NodeIn<T, input_index> *>(static_cast<N *>(this))->remove_all();
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
