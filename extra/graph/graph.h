//
// Created by xd on 2/26/25.
//

#ifndef EX_MOTION_GRAPH_H
#define EX_MOTION_GRAPH_H

#include "../data/struct/array.h"

namespace limbo {

    // --- sanity check ---
    static_assert(sizeof(unsigned long int) == 8);
    static_assert(sizeof(unsigned short) == 2);
    static_assert(sizeof(unsigned char) == 1);
    static_assert(sizeof(unsigned int) == 4);
    static_assert(sizeof(void *) == 8);
    // --- sanity check ---

    using uid64_t = unsigned long int;

    static constexpr uid64_t fnv1aHash(const char *str) {
        constexpr uid64_t FNV_OFFSET_BASIS = 14695981039346656037ull;
        uid64_t hash = FNV_OFFSET_BASIS;
        while (*str) {
            constexpr uid64_t FNV_PRIME = 1099511628211ull;
            hash ^= static_cast<uid64_t>(*str);
            hash *= FNV_PRIME;
            ++str;
        }
        return hash;
    }

    template<long N>
    struct string_literal {

        char value[N]{};
        long size;

        constexpr string_literal(const char (&str)[N]) {
            for (int i = 0; i < N; i++)
                value[i] = str[i];
            size = sizeof(value);
        }
    };
}

namespace limbo::graph {

#define GRAPH_SKIP ((void **) -1)

    struct memory {
        /* Default allocator */
        inline static ex::data::allocator allocator;
    };

    struct BaseNode;

    struct Clock;

    struct NodeType {
        virtual ~NodeType() = default;

        virtual const uid64_t get_type_id() const = 0;

        virtual const char *get_type_name() const = 0;
    };

    namespace id {

        template<string_literal name_>
        struct name : virtual NodeType {

            ~name() override = default;

            static constexpr uid64_t get_static_type_id() {
                constexpr uid64_t id_ = fnv1aHash(static_cast<const char *>(name_.value));
                return id_;
            }

            const uid64_t get_type_id() const final {
                return get_static_type_id();
            }

            const char *get_type_name() const final {
                constexpr auto name__ = name_.value;
                static auto    str    = static_cast<const char *>(name__);
                return str;
            }
        };
    }

    namespace io {

        struct Description {
            const char  *name;
            uid64_t      type_id;
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

        template<typename T, int index, string_literal name, bool aux = false>
        constexpr Description describe_io() {
            constexpr auto name_ = name.value;
            return { .name    = static_cast<const char *>(name_),
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
                const uid64_t type_ = descriptor.type_id;
                const int     index = descriptor.index;
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
                const uid64_t type_ = descriptor.type_id;
                const int     index = descriptor.index;
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
                if (input)
                    register_in_descriptor(describe_io<T, index, name, aux>());
                else
                    register_out_descriptor(describe_io<T, index, name, aux>());
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
        }
    }

    struct io_node {
        BaseNode      *node_ptr;
        unsigned short index_out;
        unsigned short index_in;
    };

    struct io_data {
        void        *data_ptr;
        unsigned int data_size;
    };

    struct BaseNode : virtual NodeType, virtual io::IODescriptor {

//        static int8_t default_comparator(const io_node *node_a, const io_node *node_b) {
//            const unsigned long a = reinterpret_cast<unsigned long>(node_a->node_ptr);
//            const unsigned long b = reinterpret_cast<unsigned long>(node_b->node_ptr);
//            return (a > b) - (a < b);
//        }

#define NODE_INIT_CAP 8

        virtual void on_input(int in_idx,int out_idx, int clock_i, const void *data) = 0;

        virtual void on_update(int clock_i) = 0;

        virtual void *on_output(int in_idx, int out_idx, int clock_i) = 0;

        ex::data::array<io_node> inputs_  = { NODE_INIT_CAP, memory::allocator };
        ex::data::array<io_node> outputs_ = { NODE_INIT_CAP, memory::allocator };
        uid64_t                  id_          = 0;
        unsigned int             clock_i_     = 0;

        ~BaseNode() override = default;

        BaseNode() = default;

        void set_node_id(const uid64_t id) {
            id_ = id;
        }

        uid64_t get_node_id() const {
            return id_;
        }

        // Attaches output of a given node to current inputs at a given input_index
        bool attach(BaseNode *node, const int this_index_in, const int other_index_out) {
            if (node == this)
                abort_("Self reference attachment");

            if (node == nullptr)
                return false;
            const auto input = this->describe_input(this_index_in);
            if (input == nullptr)
                return false;

            const io_node output_node = { node, static_cast<unsigned short>(other_index_out),
                                         static_cast<unsigned short>(this_index_in) };
            outputs_.push(output_node);

            const io_node input_node = { this, static_cast<unsigned short>(other_index_out),
                                         static_cast<unsigned short>(this_index_in) };
            node->inputs_.push(input_node);
            return true;
        }

        // Detaches output of given node from current inputs at a given input_index
        void detach(const BaseNode *node, const int index_in, const int index_out) {
            for (int i = 0; i < outputs_.size; ++i) {
                if (const io_node &ref = outputs_[i];
                    ref.node_ptr == node && ref.index_in == index_in && ref.index_out == index_out) {

                    auto &ref_outs = ref.node_ptr->inputs_;
                    for (int k = 0; k < ref_outs.size; ++k) {
                        if (ref_outs[k].node_ptr == this && ref_outs[k].index_in == index_in && ref_outs[k].index_out == index_out)
                            ref_outs.swap_remove(k--);
                    }

                    outputs_.swap_remove(i--);
                }
            }
        }

        // Detaches given node from current inputs
        void detach_node(const BaseNode *node) {
            for (int i = 0; i < outputs_.size; ++i) {
                if (const io_node &ref = outputs_[i]; ref.node_ptr == node) {

                    auto &ref_outs = ref.node_ptr->inputs_;
                    for (int k = 0; k < ref_outs.size; ++k) {
                        if (ref_outs[k].node_ptr == this && ref_outs[k].index_in)
                            ref_outs.swap_remove(k--);
                    }

                    outputs_.swap_remove(i--);
                }
            }
        }

        void detach_rec() {
            for (int i = 0; i < outputs_.size; ++i) {
                const io_node &ref = outputs_[i];
                ref.node_ptr->detach_rec();
            }
            detach_all();
        }

        void detach_all() {
            for (int i = 0; i < outputs_.size; ++i) {
                const io_node &ref = outputs_[i];
                auto &ref_outs = ref.node_ptr->inputs_;
                for (int k = 0; k < ref_outs.size; ++k) {
                    if (ref_outs[k].node_ptr == this)
                        ref_outs.swap_remove(k--);
                }
            }

            outputs_.clean();
            outputs_.reserve(NODE_INIT_CAP);
        }

        void out(const int in_idx, const int out_idx, const int clock_i, void **data) {
            if (this->clock_i_ == clock_i) {
                if (data == GRAPH_SKIP)
                    return;
                *data = on_output(in_idx, out_idx, clock_i);
                return;
            }

            for (int i = 0; i < outputs_.size; ++i) {
                void          *stack_ptr  = nullptr;
                const io_node &connection = outputs_[i];
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

    struct Clock final {
#define CLOCK_INIT_CAP 2

        ex::data::array<BaseNode *> connections_ = { CLOCK_INIT_CAP, memory::allocator };
        unsigned int                clock_i_     = 0;

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

        void pull(const int clock_i) {
            this->clock_i_ = clock_i;
            pull();
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
