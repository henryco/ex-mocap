//
// Created by henryco on 14/02/25.
//

#ifndef EX_LIMBO_DATA_UTILS_H
#define EX_LIMBO_DATA_UTILS_H

#include <cstdlib>
#include <cstring>

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

        ~buffer() {
            if (this->data != nullptr)
                free(this->data);
            this->data = nullptr;
            this->size = 0;
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

        buffer(const buffer &other) : size(other.size) {
            if (other.size <= 0) {
                data = nullptr;
                return;
            }
            data = static_cast<element_t *>(malloc(other.size * sizeof(element_t)));
            memcpy(data, other.data, other.size * sizeof(element_t));
        }

        buffer(buffer &&other) noexcept : size(other.size) {
            this->data = other.data;
            other.data = nullptr;
        }

        buffer &operator = (const buffer &other) {
            if (this == &other)
                return *this;

            if (data != nullptr)
                free(data); // can use realloc...

            size = other.size;

            if (other.size <= 0) {
                data = nullptr;
                return *this;
            }

            data = static_cast<element_t *>(malloc(other.size * sizeof(element_t)));
            memcpy(data, other.data, other.size * sizeof(element_t));

            return *this;
        }

        buffer &operator = (buffer &&other) noexcept {
            if (this == &other)
                return *this;

            if (this->data != nullptr)
                free(this->data);

            this->data = other.data;
            this->size = other.size;
            other.data = nullptr;
            return *this;
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

        array(const int &capacity) {
            this->capacity = capacity;
            this->data     = static_cast<element_t *>(malloc(capacity * sizeof(element_t)));
        }

        element_t &operator[](const int &index) {
            return data[index];
        }

        const element_t &operator[](const int &index) const {
            return data[index];
        }

        element_t &front() {
            return data[0];
        }

        const element_t &front() const {
            return data[0];
        }

        element_t &back() {
            return data[size - 1];
        }

        const element_t &back() const {
            return data[size - 1];
        }

        /**
         * This method functionally works as your arena-memory-allocator
         */
        element_t &create() {
            if (size >= capacity) {
                capacity = capacity > 0 ? (2 * capacity) : 2;
                data     = static_cast<element_t *>(realloc(data, capacity * sizeof(element_t)));
            }
            data[size++] = element_t();
            return data[size - 1];
        }

        void insert(const int index, const element_t &value) {
            if (size >= capacity) {
                capacity = capacity > 0 ? (2 * capacity) : 2;
                data     = static_cast<element_t *>(realloc(data, capacity * sizeof(element_t)));
            }
            memmove(data + index + 1, data + index, (size - index) * sizeof(element_t));
            data[index] = value;
            size++;
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

        void swap(const int index_1, const int index_2) {
            const element_t tmp = data[index_1];
            data[index_1]       = data[index_2];
            data[index_2]       = tmp;
        }

        void swap_remove(const int &index) {
            const element_t tmp = data[index];
            data[index]         = data[size - 1];
            data[size - 1]      = tmp;
            size                = (size > 0) ? (size - 1) : 0;
        }

        array(const array &other) : size(other.size), capacity(other.capacity) {
            if (other.capacity <= 0) {
                data = nullptr;
                return;
            }

            data = static_cast<element_t *>(malloc(other.capacity * sizeof(element_t)));

            if (other.size <= 0)
                return;

            memcpy(data, other.data, other.size * sizeof(element_t));
        }

        array(array &&other) noexcept : size(other.size), capacity(other.capacity) {
            this->data = other.data;
            other.data = nullptr;
        }

        array &operator = (const array &other) {
            if (this == &other)
                return *this;

            if (data != nullptr)
                free(data);

            size     = other.size;
            capacity = other.capacity;

            if (other.capacity <= 0) {
                data = nullptr;
                return *this;
            }

            data = static_cast<element_t *>(malloc(other.capacity * sizeof(element_t)));

            if (other.size <= 0)
                return *this;

            memcpy(data, other.data, other.size * sizeof(element_t));
            return *this;
        }

        array &operator = (array &&other) noexcept {
            if (this == &other)
                return *this;

            if (this->data != nullptr)
                free(this->data);

            this->data     = other.data;
            this->size     = other.size;
            this->capacity = other.capacity;
            other.data     = nullptr;
            return *this;
        }
    };

    template<typename key_t, typename element_t>
    struct entry {
        key_t     key;
        element_t element;
    };

    template<typename key_t, typename element_t>
    struct map { // linear time map, perfect for small data sets ( < ~1000) due to cache locality

        array<key_t>     key_entries;
        array<element_t> val_entries;

        const int &capacity = key_entries.capacity;
        const int &size     = key_entries.size;

        map() = default;

        ~map() = default;

        map(int capacity) : key_entries(capacity), val_entries(capacity) {}

        map(const map &src) : key_entries(src.key_entries), val_entries(src.val_entries) {}

        map(map &&src) noexcept :
            key_entries(static_cast<array<key_t> &&>(src.key_entries)),
            val_entries(static_cast<array<element_t> &&>(src.val_entries)) {
        }

        map &operator = (const map &other) {
            if (this == &other)
                return *this;
            key_entries = other.key_entries;
            val_entries = other.val_entries;
            return *this;
        }

        map &operator = (map &&other) noexcept {
            if (this == &other)
                return *this;
            key_entries = static_cast<array<key_t> &&>(other.key_entries);
            val_entries = static_cast<array<element_t> &&>(other.val_entries);
            return *this;
        }

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
            key_entries.push(key);
            val_entries.push(element_t());
            return val_entries[val_entries.size - 1];
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

        void remove_at_pos(const int &idx) {
            if (idx >= key_entries.size || idx < 0)
                return;
            key_entries.swap_remove(idx);
            val_entries.swap_remove(idx);
        }

        bool empty() const {
            return key_entries.empty();
        }
    };

    template<typename key_t, typename element_t>
    struct cache_line {

#define READY   (0)
#define FRESH   (1 << 1)
#define EVICTED (1 << 2)

        using Functor = element_t(*)(const key_t&);

        Functor supplier;

        ex::data::array<element_t> store_data;
        ex::data::array<key_t>     store_id;
        unsigned char              status = FRESH;
        unsigned int               index  = 0;

        cache_line() = delete;

        cache_line(const Functor supplier, const int reserve = 10) :
            supplier(supplier), store_data(reserve), store_id(reserve) {
        }

        element_t &operator [] (const key_t &id) {
            if (status == READY)
                return store_data[index++];

            if (status == EVICTED) {
                store_data[0] = supplier(id);
                return store_data[0];
            }

            store_id.push(id);
            store_data.push(supplier(id));
            return store_data[index];
        }

        element_t operator () (const key_t &id) {
            if (status == EVICTED)
                return supplier(id);

            if (status == FRESH) {
                const element_t &element = supplier(id);
                store_id.push(id);
                store_data.push(element);
                return element;
            }

            if (index >= store_id.size) {
                status = EVICTED;
                return supplier(id);
            }

            if (store_id[index] != id) {
                status = EVICTED;
                return supplier(id);
            }

            return store_data[index++];
        }

        void begin() {
            index = 0;
        }

        void end() {
            if (status == EVICTED) {
                evict();
                status = FRESH;
                return;
            }
            status = READY;
        }

        void evict() {
            store_data.size = 0;
            store_id.size   = 0;
            index           = 0;
            status          = EVICTED;
        }

        void reset() {
            evict();
        }
    };

}

#endif //EX_LIMBO_DATA_UTILS_H
