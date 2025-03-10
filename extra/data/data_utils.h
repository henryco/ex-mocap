//
// Created by henryco on 14/02/25.
//

#ifndef EX_LIMBO_DATA_UTILS_H
#define EX_LIMBO_DATA_UTILS_H

#include "datalloc.h"
#include <cstring>

#ifdef UNIT_TEST
#include <cassert>
#endif

namespace ex::data {

    template <typename T>
    T *malloc_(void *, const allocator &allocator, const std::size_t size_n) {
        return static_cast<T *>(allocator.malloc(size_n * sizeof(T), alignof(T)));
    }

    template <typename T>
    T *realloc_(void *, const allocator &allocator, T *const ptr, const std::size_t size_n) {
        return static_cast<T *>(allocator.realloc(ptr, size_n * sizeof(T)));
    }

    inline void free_(void *, const allocator &allocator, void *ptr) {
        if (allocator.free != nullptr && ptr != nullptr)
            allocator.free(ptr);
    }

    inline void add_watcher_(const allocator &allocator, void *watcher, const watch_fn_t fn) {
        if (allocator.add_watcher != nullptr)
            allocator.add_watcher(watcher, fn);
    }

    inline void del_watcher_(const allocator &allocator, void *watcher, const watch_fn_t fn) {
        if (allocator.del_watcher != nullptr)
            allocator.del_watcher(watcher, fn);
    }

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
                this->data = realloc_<element_t>(this, allocator_, this->data, size);
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
            data = nullptr;

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
            data = nullptr;

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

        using comparator_fn = int8_t (*)(const element_t *a, const element_t *b);
        static int8_t default_comparator(const element_t *a, const element_t *b) {
            return (*a > *b) - (*a < *b);
        }

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
            this->data = nullptr;
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
            free_(this, allocator_, data);
            this->data = nullptr;
            size     = 0;
            capacity = 0;
        }

        void reserve(const int n) {
            if (n <= capacity)
                return;

            if (data == nullptr)
                data = malloc_<element_t>(this, allocator_, n);
            else
                data = realloc_<element_t>(this, allocator_, data, n);

            capacity = n;
        }

        int binary_search(const element_t &value, const comparator_fn compare = &default_comparator) const {
            if (data == nullptr)
                return -1;

            //[n: 2]
            // i: 0 1 2 3 4 5 6 7 8
            // d: 0 1 4 5 6 7 7 8 9
            //       *            ^  8
            //       |    ^          4
            //       |^              2
            //      ^|               1
            //       |^              1

            int index = size - 1;
            int width = size;
            while (true) {
                const int8_t r = compare(&value, &data[index]);

                if (r == 0)
                    return index;

                width = (width / 2) > 0
                    ? (width / 2)
                    : 1;

                if (r < 0) index -= width;
                else       index += width;

                if (index >= size)
                    return size;

                if (index < 0)
                    return -1;

                if (width == 1) {
                    const int8_t rr = compare(&value, &data[index]);
                    if (rr == 0)
                        return index;

                    if (rr < 0 && r > 0)
                        return index;

                    if (rr > 0 && r < 0)
                        return index + 1;
                }
            }
        }

        int sort_insert(const element_t &value, const comparator_fn compare = &default_comparator) {
            const int i = binary_search(value, compare);

            if (i >= size) {
                push(value);
                return size - 1;
            }

            if (i <= 0) {
                insert(0, value);
                return 0;
            }

            insert(i, value);
            return i;
        }

        int sort_insert_once(const element_t &value, const comparator_fn compare = &default_comparator) {
            const int i = binary_search(value, compare);

            if (i >= size) {
                push(value);
                return size - 1;
            }

            if (i <= 0) {
                insert(0, value);
                return 0;
            }

            if (compare(&data[i], &value) == 0)
                return i;

            insert(i, value);
            return i;
        }

        void insert(const int index, const element_t &value) {
            if (size >= capacity) {
                capacity = capacity > 0 ? (2 * capacity) : 2;

                if (data == nullptr)
                    data = malloc_<element_t>(this, allocator_, capacity);
                else
                    data = realloc_<element_t>(this, allocator_, data, capacity);
            }
            memmove(data + index + 1, data + index, (size - index) * sizeof(element_t));
            data[index] = value;
            size++;
        }

        void push(const element_t &value) {
            if (size >= capacity) {
                capacity = capacity > 0 ? (2 * capacity) : 2;

                if (data == nullptr)
                    data = malloc_<element_t>(this, allocator_, capacity);
                else
                    data = realloc_<element_t>(this, allocator_, data, capacity);
            }
            data[size++] = value;
        }

        void collapse(const int index) {
            if (index >= size - 1) {
                size--;
                return;
            }

            // 0 1 2 3 4 5 6 : 7
            // x x x x _ x x

            // dst: 4
            // src: 5
            //   n: 2 = size(7) - index(4) - 1

            memmove(data + index, data + index + 1, (size - index - 1) * sizeof(element_t));
            size--;
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

        void collapse_element(const element_t &value) {
            for (int i = 0; i < size; ++i) {
                if (data[i] == value) {
                    collapse(i);
                    return;
                }
            }
        }

        int index_of(const element_t &value) const {
            for (int i = 0; i < size; ++i) {
                if (data[i] == value)
                    return i;
            }
            return -1;
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
            data = nullptr;

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
            data = nullptr;

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
    struct array_map { // linear time map, perfect for small data sets ( < ~1000) due to cache locality

        array<key_t>     key_entries;
        array<element_t> val_entries;

        const int &capacity = key_entries.capacity;
        const int &size     = key_entries.size;

        ~array_map() = default;

        array_map() = default;

        array_map(const allocator &allocator_) :
            key_entries(allocator_), val_entries(allocator_) {
        }

        array_map(int capacity) :
            key_entries(capacity), val_entries(capacity) {
        }

        array_map(int capacity, const allocator &allocator_) :
            key_entries(capacity, allocator_), val_entries(capacity, allocator_) {
        }

        array_map(const array_map &src) :
            key_entries(src.key_entries), val_entries(src.val_entries) {
        }

        array_map(array_map &&src) noexcept :
            key_entries(static_cast<array<key_t> &&>(src.key_entries)),
            val_entries(static_cast<array<element_t> &&>(src.val_entries)) {
        }

        array_map &operator = (const array_map &other) {
            if (this == &other)
                return *this;
            key_entries = other.key_entries;
            val_entries = other.val_entries;
            return *this;
        }

        array_map &operator = (array_map &&other) noexcept {
            if (this == &other)
                return *this;
            key_entries = static_cast<array<key_t> &&>(other.key_entries);
            val_entries = static_cast<array<element_t> &&>(other.val_entries);
            return *this;
        }

        void put(const entry<key_t, element_t> &entry) {
            put(entry.key, entry.element);
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

        int index_at(const key_t &key) const {
            for (int i = 0; i < key_entries.size; ++i) {
                if (key_entries[i] == key)
                    return i;
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

        const element_t &at_index(const int &index) const {
            return val_entries[index];
        }

        element_t &at_index(const int &index) {
            return val_entries[index];
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

        void collapse(const key_t &key) {
            for (int i = 0; i < key_entries.size; ++i) {
                if (key_entries[i] == key) {
                    key_entries.collapse(i);
                    val_entries.collapse(i);
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

        void collapse_at_pos(const int &idx) {
            if (idx >= key_entries.size || idx < 0)
                return;
            key_entries.collapse(idx);
            val_entries.collapse(idx);
        }

        bool empty() const {
            return size <= 0;
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
    struct sorted_map { // sorted map with binary tree search implementation

        using comparator_fn = int8_t (*)(const key_t *a, const key_t *b);
        static int8_t default_comparator(const key_t *a, const key_t *b) {
            return (*a > *b) - (*a < *b);
        }

        comparator_fn    comparator__;
        array<key_t>     key_entries_;
        array<element_t> val_entries_;

        const int &capacity = key_entries_.capacity;
        const int &size     = key_entries_.size;

        ~sorted_map() = default;

        sorted_map(const comparator_fn compare = &default_comparator):
            comparator__(compare) {
        }

        sorted_map(const allocator &allocator_, const comparator_fn compare = &default_comparator) :
            comparator__(compare), key_entries_(allocator_), val_entries_(allocator_) {
        }

        sorted_map(int capacity, const comparator_fn compare = &default_comparator) :
            comparator__(compare), key_entries_(capacity), val_entries_(capacity) {
        }

        sorted_map(int capacity, const allocator &allocator_, const comparator_fn compare = &default_comparator) :
            comparator__(compare), key_entries_(capacity, allocator_), val_entries_(capacity, allocator_) {
        }

        sorted_map(const sorted_map &src) :
            comparator__(src.comparator__),
            key_entries_(src.key_entries_),
            val_entries_(src.val_entries_) {
        }

        sorted_map(sorted_map &&src) noexcept :
            comparator__(src.comparator__),
            key_entries_(static_cast<array<key_t> &&>(src.key_entries_)),
            val_entries_(static_cast<array<element_t> &&>(src.val_entries_)) {
        }

        sorted_map &operator = (const sorted_map &other) {
            if (this == &other)
                return *this;
            key_entries_ = other.key_entries_;
            val_entries_ = other.val_entries_;
            comparator__ = other.comparator__;
            return *this;
        }

        sorted_map &operator = (sorted_map &&other) noexcept {
            if (this == &other)
                return *this;
            key_entries_ = static_cast<array<key_t> &&>(other.key_entries_);
            val_entries_ = static_cast<array<element_t> &&>(other.val_entries_);
            comparator__ = other.comparator__;
            return *this;
        }

        int binary_search(const key_t &key) const {
            return key_entries_.binary_search(key, comparator__);
        }

        void put(const entry<key_t, element_t> &entry) {
            put(entry.key, entry.element);
        }

        void put(const key_t &key, const element_t &element) {
            const int old_size = key_entries_.size;
            const int index = key_entries_.sort_insert_once(key, comparator__);
            if (old_size < size) val_entries_.insert(index, element);
            else                 val_entries_[index] = element;
        }

        int index_at(const key_t &key) const {
            const int index = binary_search(key);
            if (index < 0 || index >= size)
                abort();
            return index;
        }

        key_t key_at_index(const int index) const {
            return key_entries_[index];
        }

        key_t &key_at_index(const int index) {
            return key_entries_[index];
        }

        element_t &at(const key_t &key) {
            const int index = binary_search(key);
            if (index < 0 || index >= size)
                abort();
            return val_entries_[index];
        }

        const element_t &at(const key_t &key) const {
            const int index = binary_search(key);
            if (index < 0 || index >= size)
                abort();
            return val_entries_[index];
        }

        const element_t &at_index(const int &index) const {
            return val_entries_[index];
        }

        element_t &at_index(const int &index) {
            return val_entries_[index];
        }

        bool contains(const key_t &key) const {
            const int index = binary_search(key);
            if (index < 0 || index >= size)
                return false;
            return true;
        }

        void collapse(const key_t &key) {
            const int index = binary_search(key, comparator__);
            if (index < 0 || index >= size)
                return;
            key_entries_.collapse(index);
            val_entries_.collapse(index);
        }

        void collapse_at_pos(const int &idx) {
            if (idx >= key_entries_.size || idx < 0)
                return;
            key_entries_.collapse(idx);
            val_entries_.collapse(idx);
        }

        bool empty() const {
            return key_entries_.empty();
        }

        void clean() {
            key_entries_.clean();
            val_entries_.clean();
        }

        void reserve(const int n) {
            if (n <= capacity)
                return;
            key_entries_.reserve(n);
            val_entries_.reserve(n);
        }
    };


    template<typename key_t, typename val_t>
    struct rb_map {

#ifdef UNIT_TEST
        using metrics_t = int;
        metrics_t metrics_n_ = 0;
#endif

        using col_t = bool;
#define BLACK_        false
#define RED_          true

        struct rb_node {
            rb_node *par;
            rb_node *lns;
            rb_node *rns;
            val_t    val;
            key_t    key;
            col_t    col;
        };

        using comparator_fn = int8_t (*)(const key_t *a, const key_t *b);
        static int8_t default_comparator(const key_t *a, const key_t *b) {
            return (*a > *b) - (*a < *b);
        }

        comparator_fn comp_;
        int64_t       size_;
        rb_node      *root_;

        rb_map(const comparator_fn compare = &rb_map::default_comparator):
                    comp_(compare), size_(0), root_(nullptr) {
        }

        ~rb_map() {
//            recursive_free_(root_);
            iterative_free_(root_);
            root_ = nullptr;
        }

        void clean() {
//            recursive_free_(root_);
            iterative_free_(root_);
            root_ = nullptr;
            size_ = 0;
        }

        bool empty() const {
            return size_ <= 0;
        }

        int64_t size() const {
            return size_;
        }

        bool contains(const key_t &key) const {
            int direction;
            const rb_node *node = descend_(root_, key, &direction);
            if (node == nullptr)
                return false;

            if (direction != 0 || comp_(&key, &(node->key)) != 0)
                return false;

            return true;
        }

        val_t &at(const key_t &key) {
            int direction;

            #ifdef UNIT_TEST
            rb_node *node = descend_(root_, key, &direction, &metrics_n_);
            #else
            rb_node *node = descend_(root_, key, &direction);
            #endif

            if (node == nullptr)
                ::abort(); // all or nothing, avoid exceptions

            if (direction == 0 && comp_(&key, &(node->key)) == 0)
                return node->val;

            ::abort(); // all or nothing, avoid exceptions
        }

        void put(const entry<key_t, val_t> &entry) {
            put(entry.key, entry.element);
        }

        void put(const key_t &key, const val_t &val) {
            #ifdef UNIT_TEST
            if (key == 325849) {
                verify_rb_tree_();
            }
            #endif

            if (root_ == nullptr) {
                ++size_;
                root_ = static_cast<rb_node *>(::malloc(sizeof(rb_node)));
                if (root_ == nullptr)
                    ::abort();
                ::memset(root_, 0, sizeof(rb_node)); // avoid new operator
                root_->par = nullptr;
                root_->lns = nullptr;
                root_->rns = nullptr;
                root_->val = val;
                root_->key = key;
                root_->col = BLACK_;
                return;
            }

            int direction;

            #ifdef UNIT_TEST
            rb_node *base = descend_(root_, key, &direction, &metrics_n_);
            #else
            rb_node *base = descend_(root_, key, &direction);
            #endif

            if (direction == 0 && comp_(&key, &(base->key)) == 0) {
                base->val = val;
                return;
            }

            ++size_;

            rb_node *node = static_cast<rb_node *>(::malloc(sizeof(rb_node)));
            if (node == nullptr)
                ::abort();
            ::memset(node, 0, sizeof(rb_node)); // avoid new operator
            node->par     = base;
            node->lns     = nullptr;
            node->rns     = nullptr;
            node->val     = val;
            node->key     = key;
            node->col     = RED_;

            if (direction >= 0) base->rns = node;
            else                base->lns = node;

            fix_insert_if_needed_(node);

            verify_rb_tree_();

            // ReSharper disable once CppDFAConstantConditions
            if (root_ != nullptr)
                root_->col = BLACK_; // just in case
        }

        void remove(const key_t &key) {
            int direction;

            #ifdef UNIT_TEST
            rb_node *node = descend_(root_, key, &direction, &metrics_n_);
            #else
            rb_node *node = descend_(root_, key, &direction);
            #endif

            if (node == nullptr)
                return;

            if (direction != 0 || comp_(&key, &(node->key)) != 0)
                return;

            remove_node_(node);
        }

        void remove_node_(rb_node *node) {
            if (--size_ == 0) {
                clean();
                return;
            }

            if (node->lns == nullptr && node->rns == nullptr) {
                if (node == root_) {
                    // node is a root, and node has no any children
                    clean();
                    return;
                }

                fix_delete_if_needed_(node, nullptr);
                free_node_(node);

                verify_rb_tree_();
                return;
            }

            if (node->lns != nullptr && node->rns != nullptr) {
                // left and right
                rb_node *largest = get_largest_(node->lns);
                rb_node *child   = largest->lns;

                node->key = largest->key;
                node->val = largest->val;

                replace_with_child_(largest, child);
                fix_delete_if_needed_(largest, child);
                free_node_(largest);

                verify_rb_tree_();
                return;
            }

            if (node->lns != nullptr && node->rns == nullptr) {
                // left
                rb_node *child = node->lns;

                if (node == root_) {
                    root_ = child;
                    root_->par = nullptr;
                    root_->col = BLACK_;
                    free_node_(node);
                    return;
                }

                replace_with_child_(node, child);
                fix_delete_if_needed_(node, child);
                free_node_(node);

                verify_rb_tree_();
                return;
            }

            if (node->rns != nullptr && node->lns == nullptr) {
                // right
                rb_node *child = node->rns;

                if (node == root_) {
                    root_ = child;
                    root_->par = nullptr;
                    root_->col = BLACK_;
                    free_node_(node);
                    return;
                }

                replace_with_child_(node, child);
                fix_delete_if_needed_(node, child);
                free_node_(node);

                verify_rb_tree_();
                return;
            }

            ::abort();
        }

        void fix_delete_if_needed_(rb_node *node, rb_node *child) {
            // case 1: deleted was red
            if (node == nullptr || node->col == RED_)
                return;

            // case 2: deleted was black with a red child
            if (child != nullptr && child->col == RED_) {
                child->col = BLACK_;
                return;
            }

            rb_node *sibling = get_sibling_(node);
            rb_node *parent  = node->par;
            rb_node *current = node;

            fix_double_black_(current, sibling, parent);
            if (root_ != nullptr)
                root_->col = BLACK_; // just in case
        }

        void fix_double_black_(rb_node *current, rb_node *sibling, rb_node *parent) {
            if (parent == nullptr)
                return;

            // case 2: deleted was black with black sibling (NULL is implicitly black)
            if (sibling == nullptr) {
                if (parent == root_)
                    return;

                if (parent->col == RED_) {
                    parent->col = BLACK_;
                    return;
                }

                fix_double_black_(parent, get_sibling_(parent), parent->par);
                return;
            }

            // case 1: sibling is Red
            if (sibling->col == RED_) {
                swap_colors(parent, sibling);

                if (current == parent->lns)
                    rotate_left_(parent);
                else if (current == parent->rns)
                    rotate_right_(parent);

                parent = current->par;
                sibling = get_sibling_(current);

                if (sibling == nullptr) {
                    // sibling is implicitly black (case 2)
                    fix_double_black_(parent, nullptr, parent->par);
                    return;
                }
            }

            // case 2: sibling is black, both of siblings children are black
            if ((sibling->lns == nullptr || sibling->lns->col == BLACK_) &&
                (sibling->rns == nullptr || sibling->rns->col == BLACK_)) {

                sibling->col = RED_;

                if (parent == root_ || parent->col == RED_) {
                    parent->col = BLACK_;
                    return;
                }

                fix_double_black_(parent, get_sibling_(parent), parent->par);
                return;
            }

            const bool is_left = (parent->lns == current);

            // case 3: sibling is black, far child is red
            if (( is_left && (sibling->rns != nullptr && sibling->rns->col == RED_)) ||
                (!is_left && (sibling->lns != nullptr && sibling->lns->col == RED_))) {

                swap_colors(parent, sibling);

                if (is_left) {
                    rotate_left_(parent);
                    if (parent->par != nullptr && parent->par->rns != nullptr)
                        parent->par->rns->col = BLACK_;
                }
                else {
                    rotate_right_(parent);
                    if (parent->par != nullptr && parent->par->lns != nullptr)
                        parent->par->lns->col = BLACK_;
                }
                return;
            }

            // case 4: sibling is black, near child is red
            if (( is_left && (sibling->lns != nullptr && sibling->lns->col == RED_)) ||
                (!is_left && (sibling->rns != nullptr && sibling->rns->col == RED_))) {

                rb_node *near = is_left ? sibling->lns : sibling->rns;
                if (near == sibling->lns) {
                    sibling->lns->col = BLACK_;
                    sibling->col      = RED_;

                    rotate_right_(sibling);

                    sibling = parent->rns;

                    swap_colors(parent, sibling);
                    rotate_left_(parent);

                    if (sibling->rns != nullptr)
                        sibling->rns->col = BLACK_;
                }
                else {
                    sibling->rns->col = BLACK_;
                    sibling->col      = RED_;

                    rotate_left_(sibling);

                    sibling = parent->lns;

                    swap_colors(parent, sibling);
                    rotate_right_(parent);

                    if (sibling->lns != nullptr)
                        sibling->lns->col = BLACK_;
                }
            }
        }

        void fix_insert_if_needed_(rb_node *node) {
            rb_node *parent = node->par;

            if (parent == nullptr) {
                if (node != root_)
                    ::abort();

                // just in case, root is always black
                node->col = BLACK_;
                return;
            }

            if (parent->col == BLACK_)
                return;

            rb_node *grand_parent = parent->par;
            if (grand_parent == nullptr) {
                // just in case, root is always black
                parent->col = BLACK_;
//                return;
            }

            rb_node *uncle = get_sibling_(parent);

            // Case 1: Parent and uncle are both red -> Recolor and recurse up
            if (uncle != nullptr && uncle->col == RED_) {
                parent->col       = BLACK_;
                uncle->col        = BLACK_;

                if (grand_parent != nullptr) {
                    grand_parent->col = RED_;
                    fix_insert_if_needed_(grand_parent);
                }
                return;
            }

            // Case 2a: Left-Left (LL) violation
            if (node == parent->lns && parent == grand_parent->lns) {
                swap_colors(parent, grand_parent);
                rotate_right_(grand_parent);
            }

            // Case 2b: Right-Right (RR) violation
            else if (node == parent->rns && parent == grand_parent->rns) {
                swap_colors(parent, grand_parent);
                rotate_left_(grand_parent);
            }

            // Case 3a: Left-Right (LR) violation
            else if (node == parent->lns && parent == grand_parent->rns) {
                // TODO FIXME
                swap_colors(node, grand_parent);
                rotate_right_(parent);
                rotate_left_(grand_parent);
            }

            // Case 3b: Right-Left (RL) violation
            else if (node == parent->rns && parent == grand_parent->lns) {
                // TODO FIXME
                swap_colors(node, grand_parent);
                rotate_left_(parent);
                rotate_right_(grand_parent);
            }

            if (root_ != nullptr)
                root_->col = BLACK_; // just in case
        }

        rb_node *descend_(rb_node *node, const key_t &key, int *direction, int *n = nullptr) const {

            #ifdef UNIT_TEST
            if (n) *n = 0;
            #endif

            while (node != nullptr) {
                #ifdef UNIT_TEST
                if (n) (*n)++;
                #endif

                *direction = comp_(&key, &(node->key));

                if (*direction == 0)
                    return node;

                if (*direction > 0) {
                    if (node->rns != nullptr) node = node->rns;
                    else return node;
                } else {
                    if (node->lns != nullptr) node = node->lns;
                    else return node;
                }
            }
            return nullptr;
        }

        void rotate_right_(rb_node *node) {
            rb_node *left = node->lns;
            if (left == nullptr)
                return;

            node->lns = left->rns;
            if (left->rns != nullptr)
                left->rns->par = node;

            left->par = node->par;

            if (node->par == nullptr) {
                root_ = left;
            }
            else if (node == node->par->lns) {
                node->par->lns = left;
            }
            else {
                node->par->rns = left;
            }

            left->rns = node;
            node->par = left;
        }

        void rotate_left_(rb_node *node) {
            rb_node *right = node->rns;
            if (right == nullptr)
                return;

            node->rns = right->lns;
            if (right->lns != nullptr)
                right->lns->par = node;

            right->par = node->par;

            if (node->par == nullptr) {
                root_ = right;
            }
            else if (node == node->par->lns) {
                node->par->lns = right;
            }
            else {
                node->par->rns = right;
            }

            right->lns = node;
            node->par  = right;
        }

        void replace_with_child_(rb_node *node, rb_node *child) {
            if (node->par == nullptr)
                root_ = child;
            else if (node == node->par->lns)
                node->par->lns = child;
            else
                node->par->rns = child;

            if (child != nullptr)
                child->par = node->par;
        }

        static void free_node_(rb_node *node) {
            if (node == nullptr)
                return;
            if (node->par != nullptr) {
                if (node->par->lns == node)
                    node->par->lns = nullptr;
                if (node->par->rns == node)
                    node->par->rns = nullptr;
            }
            ::free(node);
        }

        static rb_node *get_largest_(rb_node *node) {
            if (node == nullptr)
                return nullptr;
            if (node->rns == nullptr)
                return node;
            return get_largest_(node->rns);
        }

        static rb_node *get_sibling_(const rb_node *node) {
            if (node == nullptr)
                ::abort();
            const rb_node *parent = node->par;
            if (parent == nullptr)
                return nullptr;
            return parent->lns == node ? parent->rns : parent->lns;
        }

        static void swap_colors(rb_node *a, rb_node *b) {
            const col_t tmp = a->col;
            a->col = b->col;
            b->col = tmp;
        }

        static void recursive_free_(rb_node *node) {
            if (node == nullptr) return;
            recursive_free_(node->lns);
            recursive_free_(node->rns);
            ::free(node);
        }

        static void iterative_free_(rb_node *node) {
            // Morris traversal technique
            rb_node *current = node;
            while (current != nullptr) {
                if (current->lns == nullptr) {
                    rb_node *right = current->rns;
                    ::free(current);
                    current = right;
                } else {
                    // find the inorder predecessor of current
                    rb_node *pre = current->lns;
                    while (pre->rns != nullptr && pre->rns != current)
                        pre = pre->rns;

                    if (pre->rns == nullptr) {
                        // create temporary threaded link
                        pre->rns = current;
                        current = current->lns;
                    } else {
                        // remove temporary threaded link
                        pre->rns = nullptr;

                        rb_node *right = current->rns;
                        ::free(current);
                        current = right;
                    }
                }
            }
        }

        static bool verify_tree_(rb_node* node, int& black_count, int path_black = 0) {
            if (!node) {
                if (black_count == -1)
                    black_count = path_black;
                return path_black == black_count;
            }

            // Red node can't have red children
            if (node->col == RED_) {
                if ((node->lns && node->lns->col == RED_) ||
                    (node->rns && node->rns->col == RED_))
                    return false;
            } else {
                path_black++;
            }

            // Check parent links
            if (node->lns && node->lns->par != node) return false;
            if (node->rns && node->rns->par != node) return false;

            return verify_tree_(node->lns, black_count, path_black) &&
                   verify_tree_(node->rns, black_count, path_black);
        }

        #ifdef UNIT_TEST
        void verify_rb_tree_() {
            int black_count = -1;
            const bool is_ok = verify_tree_(root_, black_count);
            assert(root_ == nullptr || root_->col == BLACK_);
            assert(is_ok);
        }
        #else
        #define verify_rb_tree_(empty) ;
        #endif

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
