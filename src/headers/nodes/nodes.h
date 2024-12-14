//
// Created by xd on 05/12/24.
//

#ifndef EX_MOTION_NODES_H
#define EX_MOTION_NODES_H

#include <algorithm>
#include <cstdlib>
#include <fmt/core.h>
#include <mutex>
#include <thread>
#include <vector>

#define EX_MAX_PORT_CONNECTIONS 256

namespace ex::nodes {

    template<typename I>
    class InIndex {
    protected:
        std::vector<int> indexes;
        using Type = I;
    public:
        virtual ~InIndex() = default;

        void add_input_index(const int index) {
            indexes.push_back(index);
        }

        bool has_input_index(const int index) const {
            for (int i = 0; i < indexes.size(); i++) {
                if (indexes[i] == index)
                    return true;
            }
            return false;
        }
    };

    template<typename O>
    class OutIndex {
    protected:
        std::vector<int> indexes;
        using Type = O;
    public:
        virtual ~OutIndex() = default;

        void add_output_index(const int index) {
            indexes.push_back(index);
        }

        bool has_output_index(const int index) const {
            for (int i = 0; i < indexes.size(); i++) {
                if (indexes[i] == index)
                    return true;
            }
            return false;
        }
    };

    template<typename T>
    class State {
    public:
        virtual ~State() = default;

        virtual const T * get() const = 0;
    };

    class IO {
    public:
        virtual ~IO() = default;

        virtual const std::size_t get_id() const = 0;
    };

    struct io_node {
        IO *const io_ptr;    // 8
        io_node  *next;      // 8
        const int index_in;  // 4
        const int index_out; // 4

        io_node(IO* io_ptr, const int index_in, const int index_out) :
        io_ptr(io_ptr),
        next(nullptr),
        index_in(index_in),
        index_out(index_out) {}
    };

    class OUT : virtual public IO {
    public:
        ~OUT() override = default;

        virtual void *out_any(int output_index, int clock_i, std::size_t* size) = 0;
    };

    class IN : virtual public IO {
    public:
        ~IN() override = default;

        virtual bool attach_any(IO *io_ptr, int output_index, int input_index) = 0;

        virtual bool detach_any(IO *output_ptr, int output_index, int input_index) = 0;

        virtual bool is_attached_any(IO *io_ptr, int output_index, int input_index) = 0;

        virtual bool is_attached_any(int input_index) = 0;

        virtual void detach_all_any(int input_index) = 0;

        virtual void detach_all_any() = 0;

        virtual const io_node* list_connections() const = 0;
    };

    template<typename O>
    class Output: virtual public OUT {
    public:
        ~Output() override = default;

        virtual O out(O *null_ptr, int output_index, int clock_i) = 0;

        bool output_present() const {
            return true;
        }
    };

    template<typename I>
    class Input : virtual public IN {
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

        bool input_present() const {
            return true;
        }

        virtual bool attach(Output<I> *output_ptr, int output_index, int input_index) = 0;

        virtual bool detach(Output<I> *output_ptr, int output_index, int input_index) = 0;

        virtual bool is_attached(Output<I> *output_ptr, int output_index, int input_index) = 0;

        virtual bool is_attached(int input_index) = 0;

        virtual void detach_all(int input_index) = 0;

        virtual void detach_all() = 0;
    };

    template<typename O, int N>
    class NodeOut : public virtual Output<O>,
                    public virtual OutIndex<O> {
    public:
        ~NodeOut() override = default;

        NodeOut() {
            OutIndex<O>::add_output_index(N);
        }

        O output(int clock_i) {
            return this->out(static_cast<O *>(nullptr), N, clock_i);
        }

        bool node_present() const {
            return true;
        }
    };

    template<typename I, int N>
    class NodeIn : public virtual Input<I>,
                   public virtual InIndex<I> {
    public:
        ~NodeIn() override = default;

        NodeIn() {
            InIndex<I>::add_input_index(N);
        }

        bool node_present() const {
            return true;
        }

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

    template<typename ... Types>
    class AnyIn : virtual public IN {
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

        void detach_all_any() override {
            ((_detach_all<Types>()), ...);
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
        void _detach_all() {
            auto in = dynamic_cast<Input<T> *>(this);
            if (in == nullptr)
                return;
            in->detach_all();
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
        // ReSharper disable CppMemberFunctionMayBeConst
        void lock() { /*empty*/ }
        void unlock() noexcept { /*empty*/ }
        bool try_lock() { return true; }
        // ReSharper restore CppMemberFunctionMayBeConst
    };

    template<typename Mutex_, typename T>
    class InputStore : public virtual Input<T> {
    private:
        io_node *store = nullptr;
        Mutex_ mutex_;

    public:
        ~InputStore() override {
            while (store != nullptr) {
                io_node *next_ = store->next;
                delete store;
                store = next_;
            }
        }

        void remove_connection(const int input_index) {
            detach_all(input_index);
        }

        const io_node *list_inputs() const {
            return store;
        }

        bool attach(Output<T> *output_ptr, const int output_index, const int input_index) override {
            std::lock_guard guard(mutex_);

            if (output_ptr == nullptr)
                return false;

            if (is_attached(output_ptr, output_index, input_index))
                return false;

            if (store == nullptr) {
                store = new io_node(static_cast<IO *>(output_ptr), input_index, output_index);
                return true;
            }

            io_node *head = store;
            while (head->next != nullptr) {
                head = head->next;
            }

            head->next = new io_node(static_cast<IO *>(output_ptr), input_index, output_index);
            return true;
        }

        bool detach(Output<T> *output_ptr, const int output_index, const int input_index) override {
            std::lock_guard guard(mutex_);

            if (output_ptr == nullptr)
                return false;

            io_node *prev = nullptr;
            io_node *head = store;
            int      any  = 0;

            while (head != nullptr) {
                if (head->io_ptr == static_cast<IO *>(output_ptr)
                    && (head->index_out == output_index || output_index == -1)
                    && head->index_in == input_index)
                {
                    if (prev == nullptr) {
                        store = head->next;
                        delete head;

                        if (output_index >= 0)
                            return true;

                        head = store;
                    } else {
                        prev->next = head->next;
                        delete head;

                        if (output_index >= 0)
                            return true;

                        head = prev->next;
                    }
                    any++;
                }
                prev = head;
                head = head->next;
            }
            return any > 0;
        }

        void detach_all(const int input_index) override {
            std::lock_guard guard(mutex_);

            io_node *prev = nullptr;
            io_node *head = store;

            while (head != nullptr) {
                if (head->index_in == input_index) {
                    if (prev == nullptr) {
                        store = head->next;
                        delete head;
                        head = store;
                    } else {
                        prev->next = head->next;
                        delete head;
                        head = prev->next;
                    }
                }
                prev = head;
                head = head->next;
            }
        }

        void detach_all() override {
            std::lock_guard guard(mutex_);
            while (store != nullptr) {
                io_node *next_ = store->next;
                delete store;
                store = next_;
            }
        }

        bool is_attached(Output<T> *output_ptr, const int output_index, const int input_index) override {
            std::lock_guard guard(mutex_);
            const io_node  *head = store;
            while (head != nullptr) {
                if (head->index_in == input_index
                    && head->index_out == output_index
                    && head->io_ptr == static_cast<IO *>(output_ptr))
                    return true;
                head = head->next;
            }
            return false;
        }

        bool is_attached(const int input_index) override {
            std::lock_guard guard(mutex_);
            const io_node  *head = store;
            while (head != nullptr) {
                if (head->index_in == input_index)
                    return true;
                head = head->next;
            }
            return false;
        }
    };

    template<typename ... Types>
    class KeepInput;

    template <typename ... Types>
    class KeepInput : public InputStore<null_mutex, Types>... {
    public:
        ~KeepInput() override = default;

        template<typename T>
        bool disconnect(const int input_index) {
            return static_cast<InputStore<null_mutex, T>*>(this)->remove_connection(input_index);
        }

        const io_node *list_connections() const override {
            return nullptr; // TODO
        }
    };

    template<typename... Types>
    class KeepIn : public AnyIn<Types...>,
                   public KeepInput<Types...>
    {
    public:
        ~KeepIn() override = default;
    };

    template<typename T>
    class CallbackInput  {
    public:
        virtual ~CallbackInput() = default;

        virtual bool on_input(T data, int input_index, int output_index, int clock_i) = 0;
    };

    class ReactiveState {
    public:
        virtual ~ReactiveState() = default;

        virtual void update_state(int clock_i, bool once) = 0;
    };

    template<typename ... Types>
    class ReactiveIn : public virtual ReactiveState,
                       public KeepIn<Types...>,
                       public virtual CallbackInput<Types>...
    {
    private:
        constexpr static int bytes = EX_MAX_PORT_CONNECTIONS / 8;
        char                 flags[sizeof...(Types)][bytes] = {};

        int last_clock_i = -1;

        template<typename T>
        void _pull_inputs(const int clock_i, const int type_index) {
            const io_node *inputs = static_cast<InputStore<null_mutex, T> *>(this)->list_inputs();
            const io_node *head = inputs;

            int i = 0;
            while (head != nullptr) {
                const char &byte = flags[type_index][i / bytes];
                const int   n_th = i % bytes;

                if (clock_i == last_clock_i && (byte & (1 << n_th)) != 0) {
                    continue;
                }

                flags[type_index][i / bytes] &= ~(1 << n_th);

                auto connection = dynamic_cast<Output<T> *>(head->io_ptr);
                if (connection != nullptr) {

                    const T    data = connection->out(static_cast<T *>(nullptr), head->index_out, clock_i);
                    const bool once = static_cast<CallbackInput<T> *>(this)->on_input(data, head->index_in,
                                                                                      head->index_out, clock_i);

                    if (once) {
                        // set a bit for given input
                        flags[type_index][i / bytes] |= (1 << n_th);
                    }

                    head = head->next;
                    i++;
                    continue;
                }

                const auto out_ = dynamic_cast<OUT *>(head->io_ptr);
                if (out_ == nullptr) {
                    head = head->next;
                    i++;
                    continue;
                }

                std::size_t size_      = 0;
                const void *result_ptr = out_->out_any(head->index_out, clock_i, &size_);
                if (result_ptr == nullptr || size_ <= 0) {
                    head = head->next;
                    i++;
                    continue;
                }

                if (const auto in_ = dynamic_cast<IN *>(this); in_ == nullptr) {
                    head = head->next;
                    i++;
                    continue;
                }

                if (this->on_any_input(result_ptr, size_, head->index_in, head->index_out, clock_i)) {
                    // set a bit for given input
                    flags[type_index][i / bytes] |= (1 << n_th);
                }

                head = head->next;
                i++;
            }
        }

    public:
        ~ReactiveIn() override = default;

        virtual bool on_any_input(const void *data_ptr, std::size_t size, int input_index, int output_index, int clock_i) = 0;

        void update_state(const int clock_i, const bool once) override {
            if (last_clock_i == clock_i && once)
                return;
            int i = 0;
            ((_pull_inputs<Types>(clock_i, i++)), ...);
            last_clock_i = clock_i;
        }

        void reset_internal_clock() {
            last_clock_i = -1;
        }
    };

    template<typename T>
    class ReactiveOutput : public virtual ReactiveState,
                           public virtual Output<T>
    {
    public:
        ~ReactiveOutput() override = default;

        virtual T on_output(T *null_ptr, int output_index, int clock_i) = 0;

        T out(T *null_ptr, const int output_index, const int clock_i) override {
            update_state(clock_i, false);
            return on_output(null_ptr, output_index, clock_i);
        }
    };

    template<typename ... Types>
    class ReactiveOut : public ReactiveOutput<Types>... {
    public:
        ~ReactiveOut() override = default;

        virtual void *on_any_output(int output_index, int clock_i, std::size_t* size) = 0;

        void *out_any(const int output_index, const int clock_i, std::size_t* size) override {
            static_cast<ReactiveState *>(this)->update_state(clock_i, false);
            return on_any_output(output_index, clock_i, size);
        }
    };

    struct Description {
        const std::size_t type_id;
        const int         index;
        const bool        aux;
        const char       *name;
    };

    template<typename T>
    Description describe_io(const int index, const char *name, const bool aux = false) {
        return {
            .type_id = typeid(T).hash_code(),
            .index = index,
            .aux = aux,
            .name = name
        };
    }

    class InputDescriptor {
    public:
        virtual ~InputDescriptor () = default;

        virtual Description const *describe_inputs(int *o_inputs_number) const = 0;
    };

    class OutputDescriptor {
    public:
        virtual ~OutputDescriptor () = default;

        virtual Description const *describe_outputs(int *o_outputs_number) const = 0;
    };

    class IODescriptor : public virtual InputDescriptor,
                         public virtual OutputDescriptor
    {
    public:
        ~IODescriptor() override = default;
    };

    class NodeDescriptor {
    public:
        virtual ~NodeDescriptor() = default;

        virtual const char *get_node_name() const = 0;
    };

    template<typename T>
    class Serializable : public virtual NodeDescriptor {
    public:
        ~Serializable() override = default;

        virtual T serialize() const = 0;

        virtual void deserialize(const T &serialized) = 0;
    };

    template<typename T>
    class Serializer {
    public:
        virtual ~Serializer() = default;

        virtual T serialize(const std::vector<Serializable<T> *> &serializable) const = 0;

        virtual std::vector<Serializable<T> *> deserialize(const T &serialized) = 0;
    };

    namespace internal_ {
        template<typename T>
        struct is_derived_from_template_Serializer {
        private:
            template<typename U>
            // ReSharper disable once CppFunctionIsNotImplemented
            static std::true_type test(Serializer<U> *);
            // ReSharper disable once CppFunctionIsNotImplemented
            static std::false_type test(...);

            template<typename U>
            // ReSharper disable once CppFunctionIsNotImplemented
            static U inner_type_(Serializer<U> *);
        public:
            using type_check = decltype(test(std::declval<T *>()));
            static constexpr bool value = type_check::value;

            using type = decltype(inner_type_(std::declval<T *>()));
        };

        template<std::size_t N>
        struct string_literal {
            std::size_t size;
            char value[N];
            // ReSharper disable once CppNonExplicitConvertingConstructor
            constexpr string_literal(const char (&str)[N]) {
                std::copy_n(str, N, value);
                size = sizeof(value);
            }
        };

    }

    namespace io {
        class port_dispatcher : public virtual IODescriptor {
        protected:
            std::vector<Description> in_descriptions_;
            std::vector<Description> out_descriptions_;
            bool freeze_ = false;

        public:
            ~port_dispatcher() override = default;

            void register_in_descriptor(const Description &descriptor) {
                in_descriptions_.push_back(descriptor);
            }

            void register_out_descriptor(const Description &descriptor) {
                out_descriptions_.push_back(descriptor);
            }

            const Description *describe_inputs(int *o_inputs_number) const override {
                *o_inputs_number = in_descriptions_.size();
                return in_descriptions_.data();
            }

            const Description *describe_outputs(int *o_outputs_number) const override {
                *o_outputs_number = out_descriptions_.size();
                return out_descriptions_.data();
            }
        };

        template<typename T, int index, bool input, bool aux, internal_::string_literal name>
        struct descriptor : virtual port_dispatcher {
        public:
            descriptor() {
                constexpr auto name_ = name.value;
                if (input)
                    register_in_descriptor(describe_io<T>(index, static_cast<const char *>(name_), aux));
                else
                    register_out_descriptor(describe_io<T>(index, static_cast<const char *>(name_), aux));
            }

            ~descriptor() override = default;
        };

        template<typename T, int index, internal_::string_literal name>
        struct in : virtual descriptor<T, index, true, false, name>, NodeIn<T, index> {};

        template<typename T, int index, internal_::string_literal name>
        struct out : virtual descriptor<T, index, false, false, name>, NodeOut<T, index> {};

        namespace aux {
            template<typename T, int index, internal_::string_literal name>
            struct in : virtual descriptor<T, index, true, true, name>, NodeIn<T, index> {};

            template<typename T, int index, internal_::string_literal name>
            struct out : virtual descriptor<T, index, false, true, name>, NodeOut<T, index> {};
        }
    }

    template<typename T>
    class Serialization
        : public virtual Serializable<typename internal_::is_derived_from_template_Serializer<T>::type>
    {
        static_assert(internal_::is_derived_from_template_Serializer<T>::value,
            "Template argument type should be subtype of nodes::Serializer");
    public:
        ~Serialization() override = default;
    };

    namespace id {
        template<typename T>
        class type : public virtual IO {
        public:
            ~type() override = default;

            const std::size_t get_id() const override {
                return typeid(T).hash_code();
            }
        };

        template<typename T>
        class dynamic : public virtual IO {
        public:
            ~dynamic() override = default;

            const std::size_t get_id() const override {
                return typeid(*(static_cast<T *>(this))).hash_code();
            }
        };

        template<internal_::string_literal name_>
        class name : public virtual IO,
                     public virtual NodeDescriptor
        {
            static constexpr std::size_t FNV_OFFSET_BASIS = (sizeof(std::size_t) == 4) ? 2166136261u : 14695981039346656037ull;
            static constexpr std::size_t FNV_PRIME =  (sizeof(std::size_t) == 4) ? 16777619u : 1099511628211ull;

            std::size_t fnv1aHash() const {
                constexpr auto name__ = name_.value;
                auto           str   = static_cast<const char *>(name__);
                std::size_t hash = FNV_OFFSET_BASIS;
                while (*str) {
                    hash ^= static_cast<std::size_t>(*str);
                    hash *= FNV_PRIME;
                    ++str;
                }
                return hash;
            }

        public:
            ~name() override = default;

            const std::size_t get_id() const override {
                static const std::size_t id_ = fnv1aHash();
                return id_;
            }

            const char *get_node_name() const override {
                constexpr auto name__ = name_.value;
                static auto str = static_cast<const char *>(name__);
                return str;
            }
        };

    }

    class BaseNode : public virtual IN,
                     public virtual OUT,
                     public virtual NodeDescriptor,
                     public virtual IODescriptor
    {
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
            return static_cast<NodeOut<T, idx> *>(static_cast<N *>(this))->node_present();
        }

        template<typename T>
        bool has_output(const int idx) {
            return static_cast<OutIndex<T> *>(static_cast<N *>(this))->has_output_index(idx);
        }

        template<typename T, int idx>
        bool has_input() {
            return static_cast<NodeIn<T, idx> *>(static_cast<N *>(this))->node_present();
        }

        template<typename T>
        bool has_input(const int idx) {
            return static_cast<InIndex<T> *>(static_cast<N *>(this))->has_input_index(idx);
        }
    };

    class clock final : public virtual IN,
                        public virtual OUT
    {
    private:
        std::chrono::time_point<std::chrono::steady_clock> start_time;

        void time_start() {
            if (target_fps <= 0)
                return;
            start_time = std::chrono::steady_clock::now();
        }

        void time_stop() const {
            if (target_fps <= 0)
                return;

            const auto                  stop_time  = std::chrono::steady_clock::now();
            const std::chrono::duration frame_time = stop_time - start_time;
            const double                target     = 1.0 / static_cast<double>(target_fps);

            if (const double sleep_time = target - frame_time.count(); sleep_time > 0)
                std::this_thread::sleep_for(std::chrono::duration<double>(sleep_time));
        }

        io_node   *store      = nullptr;
        int        clock_i    = -1;
        int        target_fps = -1;

    public:
        explicit clock(const int target_fps = -1)
        : target_fps(target_fps) {}

        ~clock() override {
            detach_all_any(0);
        }

        const std::size_t get_id() const override {
            return std::numeric_limits<int>::min() + 1;
        }

        const io_node *list_connections() const override {
            return store;
        }

        void *out_any(const int _, const int clock_i, std::size_t *__) override {
            time_start();
            const io_node *head = store;
            std::size_t    size = 0;
            while (head != nullptr) {
                if (const auto out = dynamic_cast<OUT *>(head->io_ptr); out != nullptr)
                    out->out_any(head->index_out, clock_i, &size);
                head = head->next;
            }
            time_stop();
            return nullptr;
        }

        bool attach_any(IO *io_ptr, const int output_index, const int _) override {
            if (io_ptr == nullptr)
                return false;

            if (is_attached_any(io_ptr, output_index, _))
                return false;

            if (store == nullptr) {
                store = new io_node(io_ptr, 0, output_index);
                return true;
            }

            io_node *head = store;
            while (head->next != nullptr) {
                head = head->next;
            }

            head->next = new io_node(io_ptr, 0, output_index);
            return true;
        }

        bool detach_any(IO *io_ptr, const int output_index, const int _) override {
            if (io_ptr == nullptr)
                return false;

            io_node *prev = nullptr;
            io_node *head = store;
            int any = 0;

            while (head != nullptr) {
                if (head->io_ptr == io_ptr && (head->index_out == output_index || output_index == -1)) {
                    if (prev == nullptr) {
                        store = head->next;
                        delete head;

                        if (output_index >= 0)
                            return true;

                        head = store;
                    } else {
                        prev->next = head->next;
                        delete head;

                        if (output_index >= 0)
                            return true;

                        head = prev->next;
                    }
                    any++;
                }
                prev = head;
                head = head->next;
            }
            return any > 0;
        }

        bool is_attached_any(IO *io_ptr, const int output_index, const int _) override {
            if (io_ptr == nullptr)
                return false;

            const io_node *head = store;
            while (head != nullptr) {
                if (head->io_ptr == io_ptr && head->index_out == output_index)
                    return true;
                head = head->next;
            }
            return false;
        }

        bool is_attached_any(const int _) override {
            return store != nullptr;
        }

        void detach_all_any() override {
            detach_all_any(0);
        }

        void detach_all_any(const int _) override {
            while (store != nullptr) {
                io_node *next_ = store->next;
                delete store;
                store = next_;
           }
        }

        void reset() {
            clock_i = 0;
        }

        void next() {
            if (clock_i >= std::numeric_limits<int>::max() - 1)
                reset();
            out_any(0, clock_i++, nullptr);
        }

        bool is_attached(IO *io_ptr, const int output_index) {
            return is_attached_any(io_ptr, output_index, 0);
        }

        bool is_attached() {
            return is_attached_any(0);
        }

        bool attach(IO *io_ptr, const int output_index) {
            return attach_any(io_ptr, output_index, 0);
        }

        bool detach(IO *io_ptr, const int output_index) {
            return detach_any(io_ptr, output_index, 0);
        }

        bool detach(IO *io_ptr) {
            return detach_any(io_ptr, -1, 0);
        }

        void detach_all() {
            detach_all_any();
        }

        void set_target_fps(const int fps) {
            this->target_fps = fps;
        }
    };

}

#endif // EX_MOTION_NODES_H
