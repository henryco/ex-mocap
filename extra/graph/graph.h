//
// Created by xd on 2/26/25.
//

#ifndef EX_MOTION_GRAPH_H
#define EX_MOTION_GRAPH_H

#include "../data/struct/array.h"
#include <typeinfo>

namespace limbo::graph {

#define GRAPH_SKIP ((void **) -1)

    struct memory {
        /* Default allocator */
        inline static ex::data::allocator allocator;
    };

    struct BaseNode;

    struct Clock;

    struct IO {
        virtual ~IO() = default;

        virtual const std::size_t get_id() const = 0;

        virtual const std::size_t get_type_id() const = 0;

        virtual const char *get_node_name() const = 0;

        virtual void out(int in_idx, int out_idx, int clock_i, void **data) = 0;
    };

    template<std::size_t N>
    struct string_literal {

        char        value[N]{};
        std::size_t size;

        constexpr string_literal(const char (&str)[N]) {
            for (int i = 0; i < N; i++)
                value[i] = str[i];
            size = sizeof(value);
        }
    };

    namespace io {

        struct Description {
            const char  *name;
            std::size_t  type_id;
            unsigned int index;
            bool         aux;
        };

        template<typename T>
        Description describe_io(const int index, const char *name, const bool aux = false) {
            return { .name    = name,
                     .type_id = typeid(T).hash_code(),
                     .index   = static_cast<unsigned int>(index),
                     .aux     = aux };
        }

        struct InputDescriptor {

            virtual ~InputDescriptor() = default;

            /**
             * Guarantee of data being sorted ASC by type and then by index
             * @param o_inputs_number size of the array returned
             * @return raw (pointer) array
             */
            virtual Description const *const describe_inputs(int *o_inputs_number) const = 0;

            virtual Description const *const describe_input(int index) const = 0;
        };

        struct OutputDescriptor {

            virtual ~OutputDescriptor() = default;

            /**
             * Guarantee of data being sorted ASC by type and then by index
             * @param o_outputs_number size of the array returned
             * @return raw (pointer) array
             */
            virtual Description const *const describe_outputs(int *o_outputs_number) const = 0;

            virtual Description const *const describe_output(int index) const = 0;
        };

        struct IODescriptor : virtual InputDescriptor, virtual OutputDescriptor {
            ~IODescriptor() override = default;
        };

        struct port_dispatcher : virtual IODescriptor {

#define DATA_INIT_CAP 8

            ex::data::array<Description> in_descriptions_  = { DATA_INIT_CAP, memory::allocator };
            ex::data::array<Description> out_descriptions_ = { DATA_INIT_CAP, memory::allocator };
            bool                         freeze_ = false;

            ~port_dispatcher() override = default;

            const Description *const describe_input(const int index) const final {
                for (int i = 0; i < in_descriptions_.size; ++i) {
                    if (in_descriptions_[i].index == index)
                        return &in_descriptions_[i];
                }
                return nullptr;
            }

            const Description *const describe_output(const int index) const final {
                for (int i = 0; i < out_descriptions_.size; ++i) {
                    if (out_descriptions_[i].index == index)
                        return &out_descriptions_[i];
                }
                return nullptr;
            }

            void register_in_descriptor(const Description &descriptor) {
                // O(n^2) time, but its often very small array (<10)
                const std::size_t type_ = descriptor.type_id;
                const int         index = descriptor.index;
                for (int i = in_descriptions_.size - 1; i >= 0; i--) {
                    if (in_descriptions_[i].type_id < type_) {
                        in_descriptions_.insert(i + 1, descriptor);
                        return;
                    }
                    if (in_descriptions_[i].type_id == type_ && in_descriptions_[i].index <= index) {
                        in_descriptions_.insert(i + 1, descriptor);
                        return;
                    }
                }
                in_descriptions_.insert(0, descriptor);
            }

            void register_out_descriptor(const Description &descriptor) {
                // O(n^2) time, but its often very small array (<10)
                const std::size_t type_ = descriptor.type_id;
                const int         index = descriptor.index;
                for (int i = out_descriptions_.size - 1; i >= 0; i--) {
                    if (out_descriptions_[i].type_id < type_) {
                        out_descriptions_.insert(i + 1, descriptor);
                        return;
                    }
                    if (out_descriptions_[i].type_id == type_ && out_descriptions_[i].index <= index) {
                        out_descriptions_.insert(i + 1, descriptor);
                        return;
                    }
                }
                out_descriptions_.insert(0, descriptor);
            }

            const Description *const describe_inputs(int *o_inputs_number) const final {
                *o_inputs_number = in_descriptions_.size;
                return in_descriptions_.data;
            }

            const Description *const describe_outputs(int *o_outputs_number) const final {
                *o_outputs_number = out_descriptions_.size;
                return out_descriptions_.data;
            }
        };

        template<typename T, int index, bool input, bool aux, string_literal name>
        struct descriptor : virtual port_dispatcher {

            descriptor() {
                constexpr auto name_ = name.value;
                if (input)
                    register_in_descriptor(describe_io<T>(index, static_cast<const char *>(name_), aux));
                else
                    register_out_descriptor(describe_io<T>(index, static_cast<const char *>(name_), aux));
            }

            ~descriptor() override = default;
        };

        template<typename T, int index, string_literal name>
        struct in : virtual descriptor<T, index, true, false, name> {};

        template<typename T, int index, string_literal name>
        struct out : virtual descriptor<T, index, false, false, name> {};

        namespace aux {
            template<typename T, int index, string_literal name>
            struct in : virtual descriptor<T, index, true, true, name> {};

            template<typename T, int index, string_literal name>
            struct out : virtual descriptor<T, index, false, true, name> {};
        } // namespace aux

    } // namespace io

    namespace id {

        template<string_literal name_>
        struct name : virtual IO {

            static constexpr std::size_t FNV_OFFSET_BASIS =
                    (sizeof(std::size_t) == 4) ? 2166136261u : 14695981039346656037ull;
            static constexpr std::size_t FNV_PRIME = (sizeof(std::size_t) == 4) ? 16777619u : 1099511628211ull;

            static constexpr std::size_t fnv1aHash() {
                constexpr auto name__ = name_.value;
                auto           str    = static_cast<const char *>(name__);
                std::size_t    hash   = FNV_OFFSET_BASIS;
                while (*str) {
                    hash ^= static_cast<std::size_t>(*str);
                    hash *= FNV_PRIME;
                    ++str;
                }
                return hash;
            }

            ~name() override = default;

            static constexpr std::size_t get_node_id() {
                constexpr std::size_t id_ = fnv1aHash();
                return id_;
            }

            const std::size_t get_type_id() const final {
                return get_node_id();
            }

            const char *get_node_name() const final {
                constexpr auto name__ = name_.value;
                static auto    str    = static_cast<const char *>(name__);
                return str;
            }
        };

    } // namespace id

    struct io_node {
        BaseNode      *node_ptr;
        unsigned short index_out;
        unsigned short index_in;
    };

    struct io_data {
        void        *data_ptr;
        unsigned int data_size;
    };

    struct BaseNode : virtual IO, virtual io::IODescriptor {

#define NODE_INIT_CAP 8

        virtual void on_input(int in_idx,int out_idx, int clock_i, const void *data) = 0;

        virtual void on_update(int clock_i) = 0;

        virtual void *on_output(int in_idx, int out_idx, int clock_i) = 0;

        virtual void *data_ptr() { return nullptr; }

        ex::data::array<io_node> connections_ = { NODE_INIT_CAP, memory::allocator };
        std::size_t              id_          = 0;
        unsigned int             clock_i_     = 0;

        ~BaseNode() override = default;

        BaseNode() = default;

        bool attach(BaseNode *node, const int this_index_in, const int other_index_out) {
            if (node == this)
                abort_("Self reference attachment");

            if (node == nullptr)
                return false;
            const auto input = this->describe_input(this_index_in);
            if (input == nullptr)
                return false;

            const io_node input_node = { node, static_cast<unsigned short>(other_index_out),
                                         static_cast<unsigned short>(this_index_in) };
            connections_.push(input_node);
            return true;
        }

        void detach(const BaseNode *node, const int index_in, const int index_out) {
            for (int i = 0; i < connections_.size; ++i) {
                if (const io_node &ref = connections_[i];
                    ref.node_ptr == node && ref.index_in == index_in && ref.index_out == index_out)
                    connections_.swap_remove(i--);
            }
        }

        void detach_node(const BaseNode *node) {
            for (int i = 0; i < connections_.size; ++i) {
                if (const io_node &ref = connections_[i]; ref.node_ptr == node)
                    connections_.swap_remove(i--);
            }
        }

        void detach_inputs(const int index_in) {
            for (int i = 0; i < connections_.size; ++i) {
                if (const io_node &ref = connections_[i]; ref.index_in == index_in)
                    connections_.swap_remove(i--);
            }
        }

        void detach_outputs(const int index_out) {
            for (int i = 0; i < connections_.size; ++i) {
                if (const io_node &ref = connections_[i]; ref.index_out == index_out)
                    connections_.swap_remove(i--);
            }
        }

        void detach_rec() {
            for (int i = 0; i < connections_.size; ++i) {
                const io_node &ref = connections_[i];
                ref.node_ptr->detach_rec();
                connections_.swap_remove(i--);
            }
            connections_.clean();
            connections_.reserve(NODE_INIT_CAP);
        }

        void detach_all() {
            connections_.clean();
            connections_.reserve(NODE_INIT_CAP);
        }

        void set_id(const std::size_t id) {
            id_ = id;
        }

        const std::size_t get_id() const final {
            return id_;
        }

        void out(const int in_idx, const int out_idx, const int clock_i, void **data) final {
            if (this->clock_i_ == clock_i) {
                if (data == GRAPH_SKIP)
                    return;
                *data = on_output(in_idx, out_idx, clock_i);
                return;
            }

            for (int i = 0; i < connections_.size; ++i) {
                void          *stack_ptr  = nullptr;
                const io_node &connection = connections_[i];
                connection.node_ptr->out(connection.index_in, connection.index_out, clock_i, &stack_ptr);
                on_input(connection.index_in, connection.index_out, clock_i, stack_ptr);
            }

            on_update(clock_i);

            if (data != GRAPH_SKIP) {
                *data = on_output(in_idx, out_idx, clock_i);
            }

            this->clock_i_ = clock_i;
        }
    };

    struct Clock final : virtual IO {
#define CLOCK_INIT_CAP 2

        ex::data::array<BaseNode *> connections_ = { CLOCK_INIT_CAP, memory::allocator };
        unsigned int                clock_i_     = 0;

        ~Clock() override = default;

        const std::size_t get_id() const override {
            return 0;
        }

        const std::size_t get_type_id() const override {
            return 0;
        }

        const char *get_node_name() const override {
            return "clock";
        }

        void out(int, int, const int clock_i, void **) override {
            this->clock_i_ = clock_i;
            pull();
        }

        void reset() {
            detach_rec();
            clock_i_ = 0;
        }

        bool attach(BaseNode *node) {
            if (node == nullptr)
                return false;

            if (!connections_.contains(node)) {
                connections_.push(node);
                return true;
            }

            return false;
        }

        void detach(BaseNode *node) {
            connections_.remove_element(node);
        }

        void detach_rec() {
            for (int i = 0; i < connections_.size; ++i) {
                connections_[i]->detach_rec();
                connections_.swap_remove(i--);
            }
            connections_.clean();
            connections_.reserve(CLOCK_INIT_CAP);
        }

        void detach_all() {
            connections_.clean();
            connections_.reserve(CLOCK_INIT_CAP);
        }

        void pull() {
            for (int i = 0; i < connections_.size; ++i)
                connections_[i]->out(-1, -1, clock_i_, GRAPH_SKIP);
            ++clock_i_;
        }
    };

    struct UpdateState {
        bool state = true;

        bool fresh() {
            return state;
        }

        bool refresh() {
            bool init = state;
            state     = true;
            return init;
        }

        bool update() {
            bool init = state;
            state     = false;
            return init;
        }
    };

}// namespace limbo::graph

#endif //EX_MOTION_GRAPH_H
