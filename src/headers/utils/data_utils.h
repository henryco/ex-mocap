//
// Created by henryco on 14/02/25.
//

#ifndef DATA_UTILS_H
#define DATA_UTILS_H

#include <cstdlib>

namespace ex::data {

    template<typename element_t>
    struct buffer {
        element_t *data = nullptr;
        long       size = 0;

        buffer() = default;

        buffer(const long &size) {
            this->data = static_cast<element_t *>(malloc(size * sizeof(element_t)));
            this->size = size;
        }

        void allocate(const long &size) {
            if (size <= this->size)
                return;
            if (this->size <= 0) {
                this->data = static_cast<element_t *>(malloc(size * sizeof(element_t)));
                this->size = size;
            }
            else {
                this->data = static_cast<element_t *>(realloc(this->data, size * sizeof(element_t)));
                this->size = size;
            }
        }

        ~buffer() {
            if (this->data != nullptr)
                std::free(this->data);
            this->data = nullptr;
            this->size = 0;
        }
    };

    template<typename element_t>
    struct array {
        element_t *data     = nullptr;
        int        size     = 0;
        int        capacity = 0;

        array() = default;

        ~array() {
            if (data == nullptr)
                return;
            free(data);
        }

        array(const int capacity) {
            this->capacity = capacity;
            this->data     = static_cast<element_t *>(malloc(capacity * sizeof(element_t)));
        }

        element_t &operator[](const int index) {
            return data[index];
        }

        const element_t &operator[](const int index) const {
            return data[index];
        }

        void push(const element_t &value) {
            if (size >= capacity) {
                capacity = capacity > 0 ? (2 * capacity) : 2;
                data     = static_cast<element_t *>(realloc(data, capacity * sizeof(element_t)));
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
            const element_t tmp = data[index];
            data[index]         = data[size - 1];
            data[size - 1]      = tmp;
            size                = (size > 0) ? (size - 1) : 0;
        }
    };

    template<typename key_t, typename element_t>
    struct entry {
        key_t     key;
        element_t element;
    };

    template<typename key_t, typename element_t>
    struct map { // linear time map, perfect for small data sets

        array<key_t>     key_entries;
        array<element_t> val_entries;

        map() = default;

        map(int capacity) : key_entries(capacity), val_entries(capacity) {}

        void put(const key_t &key, const element_t &element) {
            for (int i = 0; i < key_entries.size; ++i) {
                if (key_entries[i] == key) {
                    val_entries[i] = element;
                    return;
                }
            }
            key_entries.push(key);
            val_entries.push(element);
        }

        element_t &operator [] (const key_t &key) {
            for (int i = 0; i < key_entries.size; ++i) {
                if (key_entries[i] == key)
                    return val_entries[i];
            }
            abort();
        }

        const element_t &operator [] (const key_t &key) const {
            for (int i = 0; i < key_entries.size; ++i) {
                if (key_entries[i] == key)
                    return val_entries[i];
            }
            abort();
        }

        element_t &at(const key_t &key) {
            return this->operator[](key);
        }

        const element_t &at(const key_t &key) const {
            return this->operator[](key);
        }

        bool contains(const key_t &key) const {
            for (int i = 0; i < key_entries.size; ++i) {
                if (key_entries[i] == key)
                    return true;
            }
            return false;
        }

        void remove(const key_t &key) {
            for (int i = 0; i < key_entries.size; ++i) {
                if (key_entries[i] == key) {
                    key_entries.swap_remove(i);
                    val_entries.swap_remove(i);
                    return;
                }
            }
        }

        bool empty() const {
            return key_entries.empty();
        }
    };

}

#endif //DATA_UTILS_H
