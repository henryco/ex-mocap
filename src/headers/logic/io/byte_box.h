//
// Created by xd on 07/02/25.
//
// ReSharper disable CppNonExplicitConvertingConstructor

#ifndef BYTE_BOX_H
#define BYTE_BOX_H

#include <cstdint>
#include <cstdlib>

namespace ex::io::box {

    using index_t = std::uint32_t;
    using type__t = std::uint64_t;

    struct alignas(16) header {
        std::uint32_t size_b;
        std::uint32_t size_n;
        std::uint64_t type;
    };

    #define TYPE_ARRAY 0

    template<typename element>
    struct array {
        element *data     = nullptr;
        index_t  size     = 0;
        index_t  capacity = 0;

        array() = default;

        ~array() {
            if (data == nullptr)
                return;
            free(data);
        }

        array(const index_t capacity) {
            this->capacity = capacity;
            this->data     = static_cast<element *>(malloc(capacity * sizeof(element)));
        }

        element &operator [] (const index_t index) {
            return data[index];
        }

        void push(const element& value) {
            if (size >= capacity) {
                capacity = capacity > 0 ? (2 * capacity) : 2;
                data     = static_cast<element *>(realloc(data, capacity * sizeof(element)));
            }
            data[size++] = value;
        }

        void pop() {
            size = (size > 0) ? (size - 1) : 0;
        }

        bool empty() const {
            return size <= 0;
        }
    };

    namespace out {

        struct element {
            header      header;
            const void *data;
        };

        struct buffer {
            array<element> items;
            array<index_t> stack;
        };

        buffer create_buffer();

        void begin_array(buffer &buff);

        void begin_array(buffer &buff, type__t type);

        void end_array(buffer &buff);

        void object(buffer &buff, type__t type, std::size_t size, const void *data);

        const unsigned char *read_buffer_data(buffer &buff, std::size_t *size);

        void free_buffer_data(const unsigned char *data);

    }

    namespace in {

        struct buffer {
            array<index_t> stack;
            array<header>  items;
            header         front;
            index_t        index;
        };

    }

}

#endif //BYTE_BOX_H
