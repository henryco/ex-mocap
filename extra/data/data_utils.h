//
// Created by henryco on 14/02/25.
//

#ifndef EX_LIMBO_DATA_UTILS_H
#define EX_LIMBO_DATA_UTILS_H

#include "datalloc.h"

namespace ex::data {

    template <typename T>
    T *malloc_(void *this_, const allocator &allocator, const std::size_t new_n) {
        return static_cast<T *>(allocator.malloc(this_, new_n * sizeof(T), alignof(T)));
    }

    template <typename T>
    T *realloc_(void *this_, const allocator &allocator, T *const ptr, const std::size_t old_n, const std::size_t new_n) {
        return static_cast<T *>(allocator.realloc(this_, old_n * sizeof(T), new_n * sizeof(T), alignof(T), ptr));
    }

    inline void free_(void *this_, const allocator &allocator, void *ptr) {
        if (allocator.free != nullptr && ptr != nullptr)
            allocator.free(this_, ptr);
    }

    inline void add_watcher_(const allocator &allocator, void *ptr, const watch_fn_t fn) {
        if (allocator.add_watcher != nullptr)
            allocator.add_watcher(ptr, fn);
    }

    inline void del_watcher_(const allocator &allocator, void *ptr, const watch_fn_t fn) {
        if (allocator.del_watcher != nullptr)
            allocator.del_watcher(ptr, fn);
    }

    struct arena {

        void       *data     = nullptr;
        std::size_t offset   = 0;
        std::size_t capacity = 0;

        arena() = default;

        ~arena() {
            if (data == nullptr)
                return;
            free(data);
        }

        arena(const std::size_t &reserve) {
            expand(reserve);
        }

        void reserve(const std::size_t n) {
            if (n <= capacity)
                return;

            capacity = n;

            if (data == nullptr)
                data = malloc(capacity);
            else
                data = realloc(data, capacity);
        }

        void expand(const std::size_t n) {
            reserve(capacity + n);
        }

        void clean() {
            offset   = 0;
            capacity = 0;
            if (data == nullptr)
                return;
            free(data);
        }

        template<typename T>
        T *allocate() {
            std::size_t padding = 0;

            if (offset % alignof(T) != 0)
                padding = alignof(T) - (offset % alignof(T));

            const std::size_t required = padding + sizeof(T);
            const std::size_t remains  = capacity - offset;

            if (required > remains)
                expand(capacity + required);

            offset += required;

            return reinterpret_cast<T *>(static_cast<unsigned char *>(data) + offset - sizeof(T));
        }

        void *allocate(const std::size_t size, const std::size_t alignment = 0) {
            const std::size_t align   = alignment != 0 ? alignment : size;
            std::size_t       padding = 0;

            if (offset % align != 0)
                padding = align - (offset % align);

            const std::size_t required = padding + size;
            const std::size_t remains  = capacity - offset;

            if (required > remains)
                expand(capacity + required);

            offset += required;

            return static_cast<unsigned char *>(data) + offset - size;
        }

        arena(const arena &other) = delete;

        arena &operator = (const arena &other) = delete;

        arena(arena &&other) noexcept :
            offset(other.offset), capacity(other.capacity) {
            this->data = other.data;
            other.data = nullptr;
        }

        arena &operator = (arena &&other) noexcept {
            if (this == &other)
                return *this;

            if (this->data != nullptr)
                free(this->data);

            this->data     = other.data;
            this->offset   = other.offset;
            this->capacity = other.capacity;

            other.data = nullptr;
            return *this;
        }
    };

    template<typename element_t>
    struct buffer {

        allocator allocator_;

        element_t *data = nullptr;
        long       size = 0;

        buffer() {
            add_watcher_(allocator_, this, &watch_allocator);
        }

        buffer(const allocator &allocator_) : allocator_(allocator_) {
            add_watcher_(allocator_, this, &watch_allocator);
        }

        buffer(const long &size) : size(size) {
            add_watcher_(allocator_, this, &watch_allocator);
            this->data = malloc_<element_t>(this, allocator_, size);
        }

        buffer(const long &size, const allocator &allocator_) : allocator_(allocator_), size(size) {
            add_watcher_(allocator_, this, &watch_allocator);
            this->data = malloc_<element_t>(this, allocator_, size);
        }

        ~buffer() {
            del_watcher_(allocator_, this, &watch_allocator);
            free_(this, allocator_, data);
            this->data = nullptr;
            this->size = 0;
        }

        void allocate(const long &size) {
            if (size <= this->size)
                return;

            if (this->size <= 0 || this->data == nullptr) {
                this->data = malloc_<element_t>(this, allocator_, size);
                this->size = size;
            }

            else {
                this->data = realloc_<element_t>(this, allocator_, this->data, this->size, size);
                this->size = size;
            }
        }

        buffer(const buffer &other) : allocator_(other.allocator_), size(other.size) {
            add_watcher_(allocator_, this, &watch_allocator);
            if (other.size <= 0) {
                data = nullptr;
                return;
            }

            data = malloc_<element_t>(this, other.allocator_, other.size);
            memcpy(data, other.data, other.size * sizeof(element_t));
        }

        buffer(buffer &&other) noexcept : allocator_(other.allocator_), size(other.size) {
            del_watcher_(other.allocator_, &other, &watch_allocator);
            add_watcher_(allocator_, this, &watch_allocator);
            this->data = other.data;
            other.data = nullptr;
        }

        buffer &operator = (const buffer &other) {
            if (this == &other)
                return *this;

            del_watcher_(allocator_, this, &watch_allocator);
            free_(this, allocator_, data);

            allocator_ = other.allocator_;
            size       = other.size;

            if (other.size <= 0) {
                data = nullptr;
                return *this;
            }

            add_watcher_(allocator_, this, &watch_allocator);

            data = malloc_<element_t>(this, other.allocator_, other.size);
            memcpy(data, other.data, other.size * sizeof(element_t));

            return *this;
        }

        buffer &operator = (buffer &&other) noexcept {
            if (this == &other)
                return *this;

            del_watcher_(other.allocator_, &other, &watch_allocator);
            del_watcher_(allocator_, this, &watch_allocator);

            free_(this, allocator_, data);

            this->allocator_ = other.allocator_;
            this->size       = other.size;
            this->data       = other.data;
            other.data       = nullptr;

            add_watcher_(allocator_, this, &watch_allocator);
            return *this;
        }

        static void watch_allocator(void *instance, const long int offset) {
            auto this_ = static_cast<buffer *>(instance);
            this_->data = reinterpret_cast<element_t *>(reinterpret_cast<unsigned long int>(this_->data) + offset);
        }
    };

    template<typename element_t>
    struct array {

        allocator allocator_;

        element_t *data     = nullptr;
        int        size     = 0;
        int        capacity = 0;

        array() {
            add_watcher_(allocator_, this, &watch_allocator);
        }

        array(const allocator &allocator_) : allocator_(allocator_) {
            add_watcher_(allocator_, this, &watch_allocator);
        }

        ~array() {
            del_watcher_(allocator_, this, &watch_allocator);
            free_(this, allocator_, data);
        }

        array(const int &capacity) : capacity(capacity) {
            add_watcher_(allocator_, this, &watch_allocator);
            data = malloc_<element_t>(this, allocator_, capacity);
        }

        array(const int &capacity, const allocator &allocator_) : allocator_(allocator_), capacity(capacity) {
            add_watcher_(allocator_, this, &watch_allocator);
            data = malloc_<element_t>(this, allocator_, capacity);
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

        void clean() {
            size     = 0;
            capacity = 0;
            free_(this, allocator_, data);
        }

        void reserve(const int n) {
            if (n <= capacity)
                return;

            if (data == nullptr)
                data = malloc_<element_t>(this, allocator_, n);
            else
                data = realloc_<element_t>(this, allocator_, data, capacity, n);

            capacity = n;
        }

        void insert(const int index, const element_t &value) {
            if (size >= capacity) {
                const int old_cap = capacity;
                capacity = capacity > 0 ? (2 * capacity) : 2;

                if (data == nullptr)
                    data = malloc_<element_t>(this, allocator_, capacity);
                else
                    data = realloc_<element_t>(this, allocator_, data, old_cap, capacity);
            }
            memmove(data + index + 1, data + index, (size - index) * sizeof(element_t));
            data[index] = value;
            size++;
        }

        void push(const element_t &value) {
            if (size >= capacity) {
                const int old_cap = capacity;
                capacity = capacity > 0 ? (2 * capacity) : 2;

                if (data == nullptr)
                    data = malloc_<element_t>(this, allocator_, capacity);
                else
                    data = realloc_<element_t>(this, allocator_, data, old_cap, capacity);
            }
            data[size++] = value;
        }

        void pop() {
            size = (size > 0) ? (size - 1) : 0;
        }

        bool empty() const {
            return size <= 0;
        }

        bool contains(const element_t &value) const {
            for (int i = 0; i < size; ++i) {
                if (data[i] == value)
                    return true;
            }
            return false;
        }

        void remove_element(const element_t &value) {
            for (int i = 0; i < size; ++i) {
                if (data[i] == value) {
                    swap_remove(i);
                    return;
                }
            }
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
            del_watcher_(allocator_, this, &watch_allocator);
            if (other.capacity <= 0) {
                data = nullptr;
                return;
            }

            allocator_ = other.allocator_;
            add_watcher_(allocator_, this, &watch_allocator);

            data = malloc_<element_t>(this, other.allocator_, other.capacity);

            if (other.size <= 0)
                return;

            memcpy(data, other.data, other.size * sizeof(element_t));
        }

        array(array &&other) noexcept : size(other.size), capacity(other.capacity) {
            del_watcher_(other.allocator_, &other, &watch_allocator);
            del_watcher_(allocator_, this, &watch_allocator);

            this->data = other.data;
            other.data = nullptr;

            add_watcher_(allocator_, this, &watch_allocator);
        }

        array &operator = (const array &other) {
            if (this == &other)
                return *this;

            del_watcher_(allocator_, this, &watch_allocator);
            free_(this, allocator_, data);

            size       = other.size;
            capacity   = other.capacity;
            allocator_ = other.allocator_;

            if (other.capacity <= 0) {
                data = nullptr;
                return *this;
            }

            add_watcher_(allocator_, this, &watch_allocator);
            data = malloc_<element_t>(this, other.allocator_, other.capacity);

            if (other.size <= 0)
                return *this;

            memcpy(data, other.data, other.size * sizeof(element_t));
            return *this;
        }

        array &operator = (array &&other) noexcept {
            if (this == &other)
                return *this;

            del_watcher_(other.allocator_, &other, &watch_allocator);
            del_watcher_(allocator_, this, &watch_allocator);
            free_(this, allocator_, data);

            this->data       = other.data;
            this->size       = other.size;
            this->capacity   = other.capacity;
            this->allocator_ = other.allocator_;
            other.data       = nullptr;

            add_watcher_(allocator_, this, &watch_allocator);
            return *this;
        }

        static void watch_allocator(void *instance, const long int offset) {
            auto this_ = static_cast<array *>(instance);
            this_->data = reinterpret_cast<element_t *>(reinterpret_cast<unsigned long int>(this_->data) + offset);
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

        ~map() = default;

        map() = default;

        map(const allocator &allocator_) :
            key_entries(allocator_), val_entries(allocator_) {
        }

        map(int capacity) :
            key_entries(capacity), val_entries(capacity) {
        }

        map(int capacity, const allocator &allocator_) :
            key_entries(capacity, allocator_), val_entries(capacity, allocator_) {
        }

        map(const map &src) :
            key_entries(src.key_entries), val_entries(src.val_entries) {
        }

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
            for (int i = 0; i < key_entries.size; ++i) {
                if (key_entries[i] == key)
                    return val_entries[i];
            }
            abort();
        }

        const element_t &at(const key_t &key) const {
            for (int i = 0; i < key_entries.size; ++i) {
                if (key_entries[i] == key)
                    return val_entries[i];
            }
            abort();
        }

        element_t at_or_default(const key_t &key, const element_t &default_value = element_t()) const {
            for (int i = 0; i < key_entries.size; ++i) {
                if (key_entries[i] == key)
                    return val_entries[i];
            }
            return default_value;
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

        void clean() {
            key_entries.clean();
            val_entries.clean();
        }

        void reserve(const int n) {
            if (n <= capacity)
                return;
            key_entries.reserve(n);
            val_entries.reserve(n);
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
