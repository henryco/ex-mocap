//
// Created by xd on 3/24/25.
//

#ifndef EX_LIMBO_DATA_LIST_MAP_H
#define EX_LIMBO_DATA_LIST_MAP_H

#include "../alloc/datalloc.h"
#include <new>

namespace ex::data {

    /**
     * Linked-list backed implementation of Map
     */
    template<typename key_t, typename val_t>
    struct list_map {

        struct entry {
            key_t key;
            val_t val;
        };

        struct list_node {
            list_node *prev;
            list_node *next;
            val_t      val;
            key_t      key;
        };

        using comparator_fn = int8_t (*)(const key_t *a, const key_t *b);
        static int8_t default_comparator(const key_t *a, const key_t *b) {
            return (*a > *b) - (*a < *b);
        }

        allocator     allocator_;
        comparator_fn comp_;
        int64_t       size_;
        list_node    *root_;
        list_node    *back_;

        list_map():
            comp_(&list_map::default_comparator), size_(0), root_(nullptr), back_(nullptr) {
        }

        list_map(const comparator_fn compare):
            comp_(compare), size_(0), root_(nullptr), back_(nullptr) {
        }

        list_map(const allocator &allocator_):
            allocator_(allocator_), comp_(&list_map::default_comparator),
            size_(0), root_(nullptr), back_(nullptr) {
        }

        list_map(const allocator &allocator_, const comparator_fn compare) :
            allocator_(allocator_), comp_(compare),
            size_(0), root_(nullptr), back_(nullptr) {
        }

        list_map(const list_map &other) {
            allocator_ = other.allocator_;
            comp_      = other.comp_;
            full_copy_(other.root_, &size_, &root_, &back_);
        }

        list_map(list_map &&other) noexcept {
            allocator_  = other.allocator_;
            comp_       = other.comp_;
            root_       = other.root_;
            back_       = other.back_;
            size_       = other.size_;
            other.root_ = nullptr;
            other.back_ = nullptr;
            other.size_ = 0;
        }

        list_map &operator = (const list_map &other) {
            if (this == &other)
                return *this;

            clean();

            allocator_ = other.allocator_;
            comp_      = other.comp_;
            full_copy_(other.root_, &size_, &root_, &back_);

            return *this;
        }

        list_map &operator = (list_map &&other) noexcept {
            if (this == &other)
                return *this;

            clean();

            allocator_  = other.allocator_;
            comp_       = other.comp_;
            root_       = other.root_;
            back_       = other.back_;
            size_       = other.size_;
            other.root_ = nullptr;
            other.back_ = nullptr;
            other.size_ = 0;

            return *this;
        }

        ~list_map() {
            for (list_node *head = root_; head != nullptr;) {
                list_node *next = head->next;
                release_node_(head);
                head = next;
            }
        }

        void clean() {
            for (list_node *head = root_; head != nullptr;) {
                list_node *next = head->next;
                release_node_(head);
                head = next;
            }
            root_ = nullptr;
            back_ = nullptr;
            size_ = 0;
        }

        bool empty() const {
            return size_ == 0;
        }

        long size() const {
            return size_;
        }

        void put(const entry &entry) {
            put(entry.key, entry.val);
        }

        void put(const key_t &key, const val_t &val) {
            if (root_ == nullptr) {
                root_       = instance_node_();
                root_->next = nullptr;
                root_->prev = nullptr;
                root_->key  = key;
                root_->val  = val;
                back_       = root_;
                size_       = 1;
                return;
            }

            ++size_;

            list_node *node = instance_node_();
            if (node == root_)
                abort_("");

            back_->next = node;
            node->prev  = back_;
            node->next  = nullptr;
            node->key   = key;
            node->val   = val;

            back_ = node;
        }

        void put_sorted(const key_t &key, const val_t &val) {
            if (root_ == nullptr) {
                root_       = instance_node_();
                root_->next = nullptr;
                root_->prev = nullptr;
                root_->key  = key;
                root_->val  = val;
                back_       = root_;
                size_       = 1;
                return;
            }

            if (comp_(&key, &(root_->key)) < 0) {
                put_before(root_, key, val);
                return;
            }

            if (comp_(&key, &(back_->key)) >= 0) {
                put_after(back_, key, val);
                return;
            }

            for (list_node *head = root_; head != nullptr; head = head->next) {
                if (comp_(&key, &(head->key)) < 0) {
                    put_before(head, key, val);
                    return;
                }
            }
        }

        bool contains(const key_t &key) const {
            for (list_node *head = root_; head != nullptr; head = head->next) {
                if (head->key == key)
                    return true;
            }
            return false;
        }

        val_t &operator [] (const key_t &key) {
            if (root_ == nullptr) {
                root_       = instance_node_();
                root_->next = nullptr;
                root_->prev = nullptr;
                root_->key  = key;
                root_->val  = val_t();
                back_       = root_;
                size_       = 1;
                return root_->val;
            }

            for (list_node *head = root_; head != nullptr; head = head->next) {
                if (head->key == key)
                    return head->val;
            }

            list_node *node = instance_node_();
            if (node == root_)
                abort_("");

            ++size_;

            back_->next = node;
            node->prev  = back_;
            node->next  = nullptr;
            node->val   = val_t();
            node->key   = key;

            back_ = node;

            return node->val;
        }

        const val_t &operator [] (const key_t &key) const {
            return at(key);
        }

        val_t &at(const key_t &key) const {
            for (list_node *head = root_; head != nullptr; head = head->next) {
                if (head->key == key)
                    return head->val;
            }
            abort_("Element is not present in map: " << key);
        }

        list_node *get(const key_t &key) const {
            for (list_node *head = root_; head != nullptr; head = head->next) {
                if (head->key == key)
                    return head;
            }
            return nullptr;
        }

        list_node *front() const {
            return root_;
        }

        list_node *back() const {
            return back_;
        }

        void remove_head() {
            remove_node(root_);
        }

        void remove_back() {
            remove_node(back_);
        }

        void remove_node(const key_t &key) {
            for (list_node *head = root_; head != nullptr; head = head->next) {
                if (head->key == key) {
                    remove_node(head);
                    return;
                }
            }
        }

        void remove_node(list_node *node) {
            if (node == nullptr)
                return;

            list_node *prev = node->prev;
            list_node *next = node->next;

            if (prev != nullptr)
                prev->next = next;
            else
                root_ = next;

            if (next != nullptr)
                next->prev = prev;
            else
                back_ = prev;

            --size_;
            release_node_(node);
        }

        void put_front(const key_t &key, const val_t &val) {
            if (root_ == nullptr) {
                put(key, val);
                return;
            }
            put_before(root_, key, val);
        }

        void put_back(const key_t &key, const val_t &val) {
            if (back_ == nullptr) {
                put(key, val);
                return;
            }
            put_after(back_, key, val);
        }

        void put_after(const key_t &after, const key_t &key, const val_t &val) {
            for (list_node *head = root_; head != nullptr; head = head->next) {
                if (head->key == after) {
                    put_after(head, key, val);
                    return;
                }
            }
        }

        void put_before(const key_t &before, const key_t &key, const val_t &val) {
            for (list_node *head = root_; head != nullptr; head = head->next) {
                if (head->key == before) {
                    put_before(head, key, val);
                    return;
                }
            }
        }

        void put_after(list_node *node, const key_t &key, const val_t &val) {
            if (node == nullptr)
                return;
            ++size_;

            list_node *new_node = instance_node_();
            if (new_node == root_)
                abort_("");

            list_node *next = node->next;
            node->next      = new_node;

            if (next != nullptr)
                next->prev = new_node;

            new_node->prev  = node;
            new_node->next  = next;
            new_node->key   = key;
            new_node->val   = val;

            if (new_node->prev == back_)
                back_ = new_node;
        }

        void put_before(list_node *node, const key_t &key, const val_t &val) {
            if (node == nullptr)
                return;
            ++size_;

            list_node *new_node = instance_node_();
            if (new_node == root_)
                abort_("");

            list_node *prev = node->prev;
            node->prev      = new_node;

            if (prev != nullptr)
                prev->next = new_node;

            new_node->prev  = prev;
            new_node->next  = node;
            new_node->key   = key;
            new_node->val   = val;

            if (new_node->next == root_)
                root_ = new_node;
        }

        list_node *instance_node_() {
            return new (malloc_<list_node>(this, allocator_, 1)) list_node;
        }

        void release_node_(list_node *node) {
            if (node == nullptr)
                return;
            node->~list_node();
            free_(this, allocator_, node);
        }

        list_node *copy_node_(const list_node *node) {
            if (node == nullptr)
                return nullptr;
            list_node *copy = instance_node_();
            copy->prev      = node->prev;
            copy->next      = node->next;
            copy->key       = node->key;
            copy->val       = node->val;
            return copy;
        }

        void full_copy_(const list_node *node, int64_t *size, list_node **root, list_node **back) {
            if (node == nullptr) {
                *size = 0;
                *root = nullptr;
                *back = nullptr;
                return;
            }

            *size = 1;
            *root = copy_node_(node);
            for (list_node *head = *root; head != nullptr; head = head->next) {
                if (list_node *next = head->next; next != nullptr) {
                    list_node *copy = copy_node_(next);
                    copy->prev      = head;
                    head->next      = copy;
                    *back           = copy;
                    (*size)++;
                }
            }
        }

        struct iterator {
            list_node *head_;

            iterator(list_node *node) :
                head_(node) {
            }

            list_node &operator * () const {
                return head_;
            }

            iterator &operator ++ () {
                head_ = head_->next;
                return *this;
            }

            iterator &operator -- () {
                head_ = head_->prev;
                return *this;
            }

            bool operator != (const iterator &other) const {
                return head_ != other.head_;
            }

            bool operator == (const iterator &other) const {
                return head_ == other.head_;
            }
        };

        iterator begin() const {
            return iterator(root_);
        }

        iterator end() const {
            return iterator(back_);
        }

        template <typename T>
        T *malloc_(void *, allocator &allocator, const std::size_t size_n) {
            return static_cast<T *>(allocator.malloc(&allocator, size_n * sizeof(T), alignof(T)));
        }

        static void free_(void *, allocator &allocator, void *ptr) {
            if (allocator.free != nullptr && ptr != nullptr)
                allocator.free(&allocator, ptr);
        }
    };

}

#endif //EX_LIMBO_DATA_LIST_MAP_H
