//
// Created by henryco on 14/02/25.
//

#ifndef DATA_UTILS_H
#define DATA_UTILS_H
#include <cstdlib>

namespace ex::data {

    template<typename element>
    struct array {
        element *data     = nullptr;
        int      size     = 0;
        int      capacity = 0;

        array() = default;

        ~array() {
            if (data == nullptr)
                return;
            free(data);
        }

        array(const int capacity) {
            this->capacity = capacity;
            this->data     = static_cast<element *>(malloc(capacity * sizeof(element)));
        }

        element &operator [] (const int index) {
            return data[index];
        }

        const element &operator [] (const int index) const {
            return data[index];
        }

        void push(const element &value) {
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

        void swap_remove(const int index) {
            const element tmp = data[index];
            data[index]       = data[size - 1];
            data[size - 1]    = tmp;
            size              = (size > 0) ? (size - 1) : 0;
        }
    };

}

#endif //DATA_UTILS_H
