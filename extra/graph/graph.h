//
// Created by xd on 2/26/25.
//

#ifndef EX_MOTION_GRAPH_H
#define EX_MOTION_GRAPH_H

#include "../data/data_utils.h"

namespace limbo::graph {

    struct IO {
        virtual ~IO() = default;

        virtual const std::size_t get_id() const = 0;

        virtual const std::size_t get_type_id() const = 0;

        virtual const char *get_node_name() const = 0;

        virtual void out(int in_idx, int out_idx, int clock_i, void *data, std::size_t *size) = 0;

        virtual void in(int in_idx, int out_idx, int clock_i, const void *data, std::size_t size) = 0;
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
            memcpy(value, str, N);
            size = sizeof(value);
        }
    };

    namespace io {

        struct Description {
            std::size_t type_id;
            std::size_t type_size;
            int         index;
            bool        aux;
            const char *name;
        };

        template<typename T>
        Description describe_io(const int index, const char *name, const bool aux = false) {
            return {
                .type_id = typeid(T).hash_code(), .type_size = sizeof(T), .index = index, .aux = aux, .name = name
            };
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
                for (int i = 0; i < in_descriptions_.size(); ++i) {
                    if (in_descriptions_[i].index == index)
                        return &in_descriptions_[i];
                }
                return nullptr;
            }

            const Description *const describe_output(const int index) const override {
                for (int i = 0; i < out_descriptions_.size(); ++i) {
                    if (out_descriptions_[i].index == index)
                        return &out_descriptions_[i];
                }
                return nullptr;
            }

            void register_in_descriptor(const Description &descriptor) {
                // O(n^2) time, but its often very small array (<10)
                const std::size_t type_ = descriptor.type_id;
                const int         index = descriptor.index;
                for (int i = in_descriptions_.size() - 1; i >= 0; i--) {
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
                for (int i = out_descriptions_.size() - 1; i >= 0; i--) {
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
                *o_inputs_number = in_descriptions_.size();
                return in_descriptions_.data();
            }

            const Description *const describe_outputs(int *o_outputs_number) const override {
                *o_outputs_number = out_descriptions_.size();
                return out_descriptions_.data();
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

    struct BaseNode;

    struct io_node {
        BaseNode      *node_ptr;
        void          *cache_ptr;
        std::size_t    cache_size;
        unsigned int   clock_i;
        unsigned short index_out;
        unsigned short index_in;
    };

    struct BaseNode : virtual IO, virtual io::IODescriptor, virtual GenericDataPtr {

        // Basically arena allocation --------------------------
        inline static ex::data::array<io_node> allocator_ = 10;
        static void allocator_free() {
            allocator_.clean();
        }
        // -----------------------------------------------------

        ex::data::array<io_node *> connections = 5;
        std::size_t                id_         = 0;

        ~BaseNode() override = default;

        void set_id(const std::size_t id) {
            id_ = id;
        }

        void attach(BaseNode *node, const int index_in, const int index_out) {
            const auto input = this->describe_input(index_in);
            if (input == nullptr)
                return;

            io_node &ref   = allocator_.create();
            ref.node_ptr   = node;
            ref.cache_ptr  = nullptr;
            ref.cache_size = input->type_size;
            ref.clock_i    = 0;
            ref.index_in   = index_in;
            ref.index_out  = index_out;
            connections.push(&ref);
        }

        void detach(const BaseNode *node, const int index_in, const int index_out) {
            for (int i = 0; i < connections.size; ++i) {
                if (io_node *ref = connections[i];
                    ref->node_ptr == node && ref->index_in == index_in && ref->index_out == index_out) {
                    if (ref->cache_ptr != nullptr)
                        free(ref->cache_ptr);
                    ref->cache_ptr = nullptr;
                    connections.swap_remove(i--);
                }
            }
        }

        void detach_io(const BaseNode *node) {
            for (int i = 0; i < connections.size; ++i) {
                if (io_node *ref = connections[i]; ref->node_ptr == node) {
                    if (ref->cache_ptr != nullptr)
                        free(ref->cache_ptr);
                    ref->cache_ptr = nullptr;
                    connections.swap_remove(i--);
                }
            }
        }

        void detach_inputs(const int index_in) {
            for (int i = 0; i < connections.size; ++i) {
                if (io_node *ref = connections[i]; ref->index_in == index_in) {
                    if (ref->cache_ptr != nullptr)
                        free(ref->cache_ptr);
                    ref->cache_ptr = nullptr;
                    connections.swap_remove(i--);
                }
            }
        }

        void detach_outputs(const int index_out) {
            for (int i = 0; i < connections.size; ++i) {
                if (io_node *ref = connections[i]; ref->index_out == index_out) {
                    if (ref->cache_ptr != nullptr)
                        free(ref->cache_ptr);
                    ref->cache_ptr = nullptr;
                    connections.swap_remove(i--);
                }
            }
        }

        void detach_all() {
            for (int i = 0; i < connections.size; ++i) {
                if (io_node *ref = connections[i]) {
                    if (ref->cache_ptr != nullptr)
                        free(ref->cache_ptr);
                    ref->cache_ptr = nullptr;
                    ref->node_ptr->detach_all();
                }
                connections.swap_remove(i--);
            }
        }

        const std::size_t get_id() const override {
            return id_;
        }

        void out(const int in_idx, const int out_idx, const int clock_i, void *data, std::size_t *size) override {
            // TODO
        }

        void in(const int in_idx, const int out_idx, const int clock_i, const void *data, const std::size_t size) override {
            // TODO
        }
    };

}// namespace limbo::graph

#endif //EX_MOTION_GRAPH_H
