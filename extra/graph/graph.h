//
// Created by xd on 2/26/25.
//

#ifndef EX_MOTION_GRAPH_H
#define EX_MOTION_GRAPH_H

#include "../data/data_utils.h"
#include <typeinfo>

namespace limbo::graph {

    struct BaseNode;

    struct Clock;

    struct IO {
        virtual ~IO() = default;

        virtual const std::size_t get_id() const = 0;

        virtual const std::size_t get_type_id() const = 0;

        virtual const char *get_node_name() const = 0;

        virtual void out(int in_idx, int out_idx, int clock_i, void *data) = 0;
    };

    struct GenericDataPtr {

        virtual ~GenericDataPtr() = default;

        virtual void *get_generic_data_ptr() {
            return nullptr;
        }
    };

    template<typename T>
    struct State : virtual GenericDataPtr {

        ~State() override = default;

        virtual T *get() = 0;

        void *get_generic_data_ptr() override {
            return static_cast<void *>(this->get());
        }
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
            unsigned int size;
            unsigned int align;
            unsigned int index;
            bool         aux;
        };

        template<typename T>
        Description describe_io(const int index, const char *name, const bool aux = false) {
            return { .name    = name,
                     .type_id = typeid(T).hash_code(),
                     .size    = sizeof(T),
                     .align   = alignof(T),
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
            ex::data::array<Description> in_descriptions_;
            ex::data::array<Description> out_descriptions_;
            bool                         freeze_ = false;

            ~port_dispatcher() override = default;

            const Description *const describe_input(const int index) const override {
                for (int i = 0; i < in_descriptions_.size; ++i) {
                    if (in_descriptions_[i].index == index)
                        return &in_descriptions_[i];
                }
                return nullptr;
            }

            const Description *const describe_output(const int index) const override {
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

            const Description *const describe_inputs(int *o_inputs_number) const override {
                *o_inputs_number = in_descriptions_.size;
                return in_descriptions_.data;
            }

            const Description *const describe_outputs(int *o_outputs_number) const override {
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

            const std::size_t get_type_id() const override {
                return get_node_id();
            }

            const char *get_node_name() const override {
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

    struct allocator {
        // Memory allocator ------------------------------------
        inline static ex::data::arena arena_ = 1024;

        static void *allocate(const std::size_t size_new_b, const std::size_t alignment_b) {
            return arena_.allocate(size_new_b, alignment_b);
        }

        static void *reallocate(const std::size_t size_old_b, const std::size_t size_new_b,
                                const std::size_t alignment_b, void *ptr) {
            void *const new_data = arena_.allocate(size_new_b, alignment_b);
            memcpy(new_data, ptr, size_old_b);
            return new_data;
        }

        static void free(const int size_new_b = 1024) {
            arena_.clean();
            if (size_new_b >= 0)
                arena_.reserve(size_new_b);
        }

        inline static ex::data::allocator arena = { &allocate, &reallocate, nullptr };
        // -----------------------------------------------------
    };

    struct BaseNode : virtual IO, virtual io::IODescriptor, virtual GenericDataPtr {

#define NODE_INIT_CAP 64

        using assignment_fn_t = void (*)(int output_idx, const void *src_ptr, void *dst_ptr);

        virtual void on_input(int in_idx,int out_idx, int clock_i, const void *data) = 0;

        virtual void on_output(int in_idx, int out_idx, int clock_i, void *data) = 0;

        assignment_fn_t                assignment_fn_ = nullptr;
        ex::data::map<ushort, io_data> cache_map      = { NODE_INIT_CAP, allocator::arena };
        ex::data::map<ushort, void *>  stack_map      = { NODE_INIT_CAP, allocator::arena };
        ex::data::array<io_node>       connections    = { NODE_INIT_CAP, allocator::arena };
        unsigned int                   clock_i        = 0;
        std::size_t                    id_            = 0;

        ~BaseNode() override = default;

        BaseNode() {
            reallocate(false);
        }

        void reallocate(const bool cleanup = true) {
            if (cleanup) {
                cache_map.clean();
                stack_map.clean();
                connections.clean();
                cache_map.reserve(NODE_INIT_CAP);
                stack_map.reserve(NODE_INIT_CAP);
                connections.reserve(NODE_INIT_CAP);
            }

            int size;

            const auto outputs = this->describe_outputs(&size);
            for (int i = 0; i < size; ++i) {
                const auto &output = outputs[i];
                cache_map.put(output.index, {
                    allocator::arena_.allocate(output.size, output.align),
                    output.size
                });
            }

            const auto inputs = this->describe_inputs(&size);
            for (int i = 0; i < size; ++i) {
                const auto &input = inputs[i];
                stack_map.put(input.index, allocator::arena_.allocate(input.size, input.align));
            }
        }

        void set_assignment_function(const assignment_fn_t fn) {
            this->assignment_fn_ = fn;
        }

        void set_id(const std::size_t id) {
            id_ = id;
        }

        void attach(BaseNode *node, const int this_index_in, const int other_index_out) {
            const auto input = this->describe_input(this_index_in);
            if (input == nullptr)
                return;
            connections.push(
                    { node,
                        static_cast<unsigned short>(other_index_out),
                        static_cast<unsigned short>(this_index_in) });
        }

        void detach(const BaseNode *node, const int index_in, const int index_out) {
            for (int i = 0; i < connections.size; ++i) {
                if (const io_node &ref = connections[i];
                    ref.node_ptr == node && ref.index_in == index_in && ref.index_out == index_out)
                    connections.swap_remove(i--);
            }
        }

        void detach_io(const BaseNode *node) {
            for (int i = 0; i < connections.size; ++i) {
                if (const io_node &ref = connections[i]; ref.node_ptr == node)
                    connections.swap_remove(i--);
            }
        }

        void detach_inputs(const int index_in) {
            for (int i = 0; i < connections.size; ++i) {
                if (const io_node &ref = connections[i]; ref.index_in == index_in)
                    connections.swap_remove(i--);
            }
        }

        void detach_outputs(const int index_out) {
            for (int i = 0; i < connections.size; ++i) {
                if (const io_node &ref = connections[i]; ref.index_out == index_out)
                    connections.swap_remove(i--);
            }
        }

        void detach_all() {
            for (int i = 0; i < connections.size; ++i) {
                const io_node &ref = connections[i];
                ref.node_ptr->detach_all();
                connections.swap_remove(i--);
            }
        }

        const std::size_t get_id() const override {
            return id_;
        }

        void pull(const int clock_i) {
            for (int i = 0; i < connections.size; ++i) {
                const io_node &connection = connections[i];
                void *const stack_ptr = stack_map.at(connection.index_in);
                connection.node_ptr->out(connection.index_in, connection.index_out, clock_i, stack_ptr);
                on_input(connection.index_in, connection.index_out, clock_i, stack_ptr);
            }
        }

        void out(const int in_idx, const int out_idx, const int clock_i, void *data) final {

            const io_data &cache_ptr = cache_map.at(out_idx);

            if (this->clock_i == clock_i) {
                assign_(out_idx, cache_ptr.data_ptr, data, cache_ptr.data_size);
                return;
            }

            for (int i = 0; i < connections.size; ++i) {
                const io_node &connection = connections[i];
                void *const stack_ptr = stack_map.at(connection.index_in);
                connection.node_ptr->out(connection.index_in, connection.index_out, clock_i, stack_ptr);
                on_input(connection.index_in, connection.index_out, clock_i, stack_ptr);
            }

            on_output(in_idx, out_idx, clock_i, data);
            assign_(out_idx, data, cache_ptr.data_ptr, cache_ptr.data_size);

            this->clock_i = clock_i;
        }

        void assign_(const int out_idx, const void *src_ptr, void *dst_ptr, const std::size_t size) const {
            if (assignment_fn_ != nullptr) {
                assignment_fn_(out_idx, src_ptr, dst_ptr);
                return;
            }
            memcpy(dst_ptr, src_ptr, size);
        }
    };

    struct Clock final: virtual IO {

#define EX_INT16_MAX 32767
#define CLOCK_INIT_CAP 64

        ex::data::array<BaseNode *> connections = { CLOCK_INIT_CAP, allocator::arena };
        unsigned int                clock_i     = 0;

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

        void out(int, int, const int clock_i, void *) override {
            this->clock_i = clock_i;
            pull();
        }

        void reset() {
            detach_all();
            clock_i = 0;
        }

        void attach(BaseNode *node) {
            if (!connections.contains(node))
                connections.push(node);
        }

        void detach(BaseNode *node) {
            connections.remove_element(node);
        }

        void detach_all() {
            connections.clean();
            connections.reserve(CLOCK_INIT_CAP);
        }

        void pull() {
            clock_i = (clock_i > EX_INT16_MAX) ? 0 : (clock_i + 1);
            for (int i = 0; i < connections.size; ++i)
                connections[i]->pull(clock_i);
        }
    };

}// namespace limbo::graph

#endif //EX_MOTION_GRAPH_H
