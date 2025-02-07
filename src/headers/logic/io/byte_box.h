//
// Created by xd on 07/02/25.
//
// ReSharper disable CppNonExplicitConvertingConstructor

#ifndef BYTE_BOX_H
#define BYTE_BOX_H

#include <cstdint>
#include <cstdlib>

namespace ex::io::box {

    #define TYPE_ARRAY 0

    template<typename T>
    struct stack {
        T            *data     = nullptr;
        std::uint32_t size     = 0;
        std::uint32_t capacity = 0;

        stack() = default;

        ~stack() {
            if (data == nullptr)
                return;
            free(data);
        }

        stack(const std::uint32_t capacity) {
            this->capacity = capacity;
            this->data     = static_cast<T *>(malloc(capacity * sizeof(T)));
        }

        T &operator[](const std::uint32_t index) {
            return data[index];
        }

        void push(const T& value) {
            if (size >= capacity) {
                capacity = capacity > 0 ? (2 * capacity) : 2;
                data     = static_cast<T *>(realloc(data, capacity * sizeof(T)));
            }
            data[size++] = value;
        }

        void pop() {
            if (size <= 0)
                return;
            size--;
        }

        bool empty() const {
            return size > 0;
        }
    };

    struct alignas(16) header {
        std::uint32_t size_b;
        std::uint32_t size_n;
        std::uint64_t type;
    };

    struct element {
        header      header;
        const void *data;
    };

    struct buffer {
        stack<element> items;
        stack<buffer>  stack;
    };

    buffer create_buffer();

    void begin_array(buffer &buff);

    void end_array(buffer &buff);

    void object(buffer &buff, std::uint64_t type, std::uint32_t size, const void *data);

    const unsigned char *read_buffer(buffer &buff, std::size_t &size);

}

#endif //BYTE_BOX_H
