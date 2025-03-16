//
// Created by henryco on 12/03/25.
//

#ifndef EX_LIMBO_DATA_RB_MAP_H
#define EX_LIMBO_DATA_RB_MAP_H

#include "datalloc.h"

namespace ex::data {

    /**
     * Red-Black Tree implementation of Map
     */
    template<typename key_t, typename val_t>
    struct rb_map {

        struct entry {
            key_t key;
            val_t val;
        };

        using idx_t = u_int32_t;
        using own_t = char;
        using col_t = bool;
#define BLACK_        false
#define RED_          true

#define HAS_NIL_ (0 << 0)
#define HAS_LNS_ (1 << 0)
#define HAS_RNS_ (1 << 1)

        struct rb_node {
            idx_t    idx;
            val_t    val;
            key_t    key;
            col_t    col;
            own_t    own;
        };

#ifdef UNIT_TEST
        using metrics_t = int;
        inline static metrics_t metrics_n_ = 0;
#endif

        using comparator_fn = int32_t (*)(const key_t *a, const key_t *b);
        static int32_t default_comparator(const key_t *a, const key_t *b) {
            return (*a > *b) - (*a < *b);
        }

        allocator allocator_;

        comparator_fn comp_;
        int64_t       size_;
        rb_node      *data_;

        rb_map():
            comp_(&rb_map::default_comparator), size_(0), data_(nullptr) {
        }

        rb_map(const comparator_fn compare):
            comp_(compare), size_(0), data_(nullptr) {
            add_watcher_(allocator_, this, &watch_allocator);
        }

        rb_map(const allocator &allocator_):
            allocator_(allocator_), comp_(&rb_map::default_comparator),
            size_(0), data_(nullptr) {
            add_watcher_(allocator_, this, &watch_allocator);
        }

        rb_map(const allocator &allocator_, const comparator_fn compare):
            allocator_(allocator_), comp_(compare), size_(0), data_(nullptr) {
            add_watcher_(allocator_, this, &watch_allocator);
        }

        rb_map(const rb_map &other):
            allocator_(other.allocator_), comp_(other.comp_), size_(other.size_) {
            add_watcher_(allocator_, this, &watch_allocator);

            if (other.size_ > 0 && other.data_ != nullptr) {
                data_ = malloc_<rb_node>(this, other.allocator_, other.size_);
                ::memcpy(data_, other.data_, other.size_ * sizeof(rb_node));
            }
        }

        rb_map(rb_map &&other) noexcept:
            allocator_(other.allocator_), comp_(other.comp_),
            size_(other.size_), data_(other.data_) {
            other.data_ = nullptr;
            other.size_ = 0;
            del_watcher_(other.allocator_, this, &watch_allocator);
            add_watcher_(allocator_, this, &watch_allocator);
        }

        rb_map &operator = (const rb_map &other) {
            if (this == &other)
                return *this;
            clean();
            del_watcher_(allocator_, this, &watch_allocator);
            allocator_ = other.allocator_;
            comp_      = other.comp_;
            size_      = other.size_;
            add_watcher_(allocator_, this, &watch_allocator);

            if (other.size_ > 0 && other.data_ != nullptr) {
                data_ = malloc_<rb_node>(this, other.allocator_, other.size_);
                ::memcpy(data_, other.data_, other.size_ * sizeof(rb_node));
            }

            return *this;
        }

        rb_map &operator = (rb_map &&other) noexcept {
            if (this == &other)
                return *this;
            clean();
            del_watcher_(other.allocator_, this, &watch_allocator);
            del_watcher_(allocator_, this, &watch_allocator);
            allocator_ = other.allocator_;
            comp_      = other.comp_;
            size_      = other.size_;
            data_      = other.data_;
            add_watcher_(allocator_, this, &watch_allocator);
            other.data_ = nullptr;
            other.size_ = 0;
            return *this;
        }

        ~rb_map() {
            del_watcher_(allocator_, this, &watch_allocator);
            free_(this, allocator_, data_);
            data_ = nullptr;
        }

        void clean() {
            free_(this, allocator_, data_);
            data_ = nullptr;
            size_ = 0;
        }

        bool empty() const {
            return size_ <= 0;
        }

        int64_t size() const {
            return size_;
        }

        rb_node *next(const key_t &key) const {
            int direction;
#ifdef UNIT_TEST
            rb_node *node = descend_(data_, key, &direction, &metrics_n_);
#else
            rb_node *node = descend_(data_, key, &direction);
#endif
            if (node == nullptr || direction != 0 || comp_(&key, &(node->key)) != 0)
                ::abort();
            if (node->rns != nullptr) {
                if (rb_node *min_ = minimum_(node->rns))
                    return min_;
                ::abort();
            }
            for (rb_node *head = node;;) {
                rb_node *parent = head->par;
                if (parent == nullptr)
                    return nullptr;
                if (parent->lns == head)
                    return parent;
                head = parent;
            }
        }

        rb_node *prev(const key_t &key) const {
            int direction;
#ifdef UNIT_TEST
            rb_node *node = descend_(data_, key, &direction, &metrics_n_);
#else
            rb_node *node = descend_(data_, key, &direction);
#endif
            if (node == nullptr || direction != 0 || comp_(&key, &(node->key)) != 0)
                ::abort();
            if (node->lns != nullptr) {
                if (rb_node *max_ = maximum_(node->lns))
                    return max_;
                ::abort();
            }
            for (rb_node *head = node;;) {
                rb_node *parent = head->par;
                if (parent == nullptr)
                    return nullptr;
                if (parent->rns == head)
                    return parent;
                head = parent;
            }
        }

        bool contains(const key_t &key) const {
            int direction;
#ifdef UNIT_TEST
            rb_node *node = descend_(data_, key, &direction, &metrics_n_);
#else
            rb_node *node = descend_(data_, key, &direction);
#endif
            if (node == nullptr)
                return false;

            if (direction != 0 || comp_(&key, &(node->key)) != 0)
                return false;

            return true;
        }

        val_t &at(const key_t &key) const {
            int direction;

#ifdef UNIT_TEST
            rb_node *node = descend_(data_, key, &direction, &metrics_n_);
#else
            rb_node *node = descend_(data_, key, &direction);
#endif

            if (node == nullptr)
                ::abort(); // all or nothing, avoid exceptions

            if (direction == 0 && comp_(&key, &(node->key)) == 0)
                return node->val;

            ::abort(); // all or nothing, avoid exceptions
        }

        val_t &operator [] (const key_t &key) {

            if (data_ == nullptr) {
                data_             = create_root_();
                const rb_node tmp = { 0, val_t(), key, BLACK_, HAS_NIL_ };
                ::memcpy(data_, &tmp, sizeof(rb_node));
                ++size_;
                return data_->val;
            }

            int direction;

#ifdef UNIT_TEST
            rb_node *base = descend_(data_, key, &direction, &metrics_n_);
#else
            rb_node *base = descend_(data_, key, &direction);
#endif

            if (direction == 0 && base != nullptr && comp_(&key, &(base->key)) == 0)
                return base->val;

            ++size_;

            const idx_t   c_idx = child_idx_(base, (direction < 0) ? HAS_LNS_ : HAS_RNS_);
            const rb_node tmp_  = { c_idx, val_t(), key, RED_, HAS_NIL_ };
            rb_node      *node  = create_node_(c_idx);
            ::memcpy(node, &tmp_, sizeof(rb_node));

            if (direction < 0) set_lns_(&(base->own));
            else               set_rns_(&(base->own));

            fix_insert_if_needed_(node);

            data_->col = BLACK_; // just in case
            return node->val;
        }

        const val_t &operator [] (const key_t &key) const {
            return at(key);
        }

        void put(const entry &entry) {
            put(entry.key, entry.val);
        }

        void put(const key_t &key, const val_t &val) {

            if (data_ == nullptr) {
                data_             = create_root_();
                const rb_node tmp = { 0, val_t(), key, BLACK_, HAS_NIL_ };
                ::memcpy(data_, &tmp, sizeof(rb_node));
                ++size_;
                return;
            }

            int direction;

#ifdef UNIT_TEST
            rb_node *base = descend_(data_, key, &direction, &metrics_n_);
#else
            rb_node *base = descend_(data_, key, &direction);
#endif

            if (direction == 0 && comp_(&key, &(base->key)) == 0) {
                base->val = val;
                return;
            }

            ++size_;

            const idx_t   c_idx = child_idx_(base, (direction < 0) ? HAS_LNS_ : HAS_RNS_);
            const rb_node tmp_  = { c_idx, val_t(), key, RED_, HAS_NIL_ };
            rb_node      *node  = create_node_(c_idx);
            ::memcpy(node, &tmp_, sizeof(rb_node));

            if (direction < 0) set_lns_(&(base->own));
            else               set_rns_(&(base->own));

            // if root_ is nullptr we have much bigger problem
            data_->col = BLACK_; // just in case
        }

        void remove(const key_t &key) {
            int direction;

#ifdef UNIT_TEST
            rb_node *node = descend_(data_, key, &direction, &metrics_n_);
#else
            rb_node *node = descend_(data_ key, &direction);
#endif

            if (node == nullptr)
                return;

            if (direction != 0 || comp_(&key, &(node->key)) != 0)
                return;

            remove_node_(node);
        }

        void remove_node_(rb_node *node) { // TODO
            if (--size_ == 0) {
                clean();
                return;
            }

            if (node->lns == nullptr && node->rns == nullptr) {
                if (node == data_) {
                    // node is a root, and node has no any children
                    clean();
                    return;
                }

                rb_node *sibling = get_sibling_(node);
                replace_with_child_(node, nullptr);
                fix_delete_if_needed_(node, nullptr, sibling);
                free_node_(node);

                return;
            }

            if (node->lns != nullptr && node->rns != nullptr) {
                // left and right
                rb_node *largest = get_largest_(node->lns);
                rb_node *child   = largest->lns;

                node->key = largest->key;
                node->val = largest->val;

                rb_node *sibling = get_sibling_(largest);
                replace_with_child_(largest, child);
                fix_delete_if_needed_(largest, child, sibling);
                free_node_(largest);

                return;
            }

            if (node->lns != nullptr && node->rns == nullptr) {
                // left
                rb_node *child = node->lns;

                if (node == data_) {
                    root_ = child;
                    root_->par = nullptr;
                    root_->col = BLACK_;
                    free_node_(node);
                    return;
                }

                rb_node *sibling = get_sibling_(node);
                replace_with_child_(node, child);
                fix_delete_if_needed_(node, child, sibling);
                free_node_(node);

                return;
            }

            if (node->rns != nullptr && node->lns == nullptr) {
                // right
                rb_node *child = node->rns;

                if (node == data_) {
                    root_ = child;
                    root_->par = nullptr;
                    root_->col = BLACK_;
                    free_node_(node);
                    return;
                }

                rb_node *sibling = get_sibling_(node);
                replace_with_child_(node, child);
                fix_delete_if_needed_(node, child, sibling);
                free_node_(node);

                return;
            }

            ::abort();
        }

        void fix_delete_if_needed_(rb_node *node, rb_node *child, rb_node *sibling) {
            // case 1: deleted was red
            if (node == nullptr || node->col == RED_)
                return;

            // case 2: deleted was black with a red child
            if (child != nullptr && child->col == RED_) {
                child->col = BLACK_;
                return;
            }

            fix_double_black_(child, sibling, node->par);
            if (data_ != nullptr)
                data_->col = BLACK_; // just in case
        }

        void fix_double_black_(rb_node *current, rb_node *sibling, rb_node *parent) {
            if (parent == nullptr)
                return;

            // case 2: deleted was black with black sibling (NULL is implicitly black)
            if (sibling == nullptr) {
                if (parent == data_)
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

//                parent = current->par; // parent does not change after rotation

                if (current != nullptr) sibling = get_sibling_(current);
                else                    sibling = (parent->lns == current) ? parent->rns : parent->lns;

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

                if (parent == data_ || parent->col == RED_) {
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
                if (node != data_)
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
                swap_colors(node, grand_parent);
                rotate_right_(parent);
                rotate_left_(grand_parent);
            }

            // Case 3b: Right-Left (RL) violation
            else if (node == parent->rns && parent == grand_parent->lns) {
                swap_colors(node, grand_parent);
                rotate_left_(parent);
                rotate_right_(grand_parent);
            }

            if (data_ != nullptr)
                data_->col = BLACK_; // just in case
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

        void rotate_right_(rb_node *node) { // TODO
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

        void rotate_left_(rb_node *node) { // TODO
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

        void replace_with_child_(rb_node *node, rb_node *child) { // TODO
            if (node->par == nullptr)
                root_ = child;
            else if (node == node->par->lns)
                node->par->lns = child;
            else
                node->par->rns = child;

            if (child != nullptr)
                child->par = node->par;
        }

        rb_node *create_root_() {
            if (data_!= nullptr || size_ > 0)
                ::abort(); /// wtf
            data_ = malloc_<rb_node>(this, allocator_, 1);
            size_ = 1;
            return &(data_[index]);
        }

        rb_node *create_node_(const idx_t index) {
            if (index >= size_) {
                size_ = (size_ * 2) + (index - size_);
                data_ = realloc_<rb_node>(this, allocator_, data_, size_);
            }
            return &(data_[index]);
        }

        static idx_t child_idx_(const rb_node *parent, const own_t side) {
            if (side != HAS_LNS_ && side != HAS_RNS_)
                ::abort(); /// wtf
            return (side == HAS_LNS_)
                ? idx_lns_(parent->idx)
                : idx_rns_(parent->idx);
        }

        static rb_node *get_largest_(const rb_node *node) {
            if (node == nullptr)
                return nullptr;

            rb_node *rns = rns_(node);
            if (rns == nullptr)
                return node;
            return get_largest_(rns);
        }

        static rb_node *get_sibling_(const rb_node *node) {
            if (node == nullptr)
                ::abort();

            const rb_node *parent = par_(node);
            if (parent == nullptr)
                return nullptr;

            rb_node *left = lns_(parent);
            return left == node ? rns_(parent) : left;
        }

        static void swap_colors(rb_node *a, rb_node *b) {
            const col_t tmp = a->col;
            a->col = b->col;
            b->col = tmp;
        }

        static rb_node *minimum_(rb_node *const node) {
            rb_node *head = node;

            while (head != nullptr) {
                rb_node *left = lns_(head);
                if (left == nullptr)
                    return head;
                head = left;
            }

            return head;
        }

        static rb_node *maximum_(rb_node *const node) {
            rb_node *head = node;

            while (head != nullptr) {
                rb_node *right = rns_(head);
                if (right == nullptr)
                    return head;
                head = right;
            }

            return head;
        }

        // DSF Post-Order iterator
        struct iterator { // TODO FIXME
            entry    entr_;
            rb_node *node_;
            rb_node *head_;
            rb_node *prev_;
            long     idx__;
            bool     down_;

            iterator(rb_node *node):
                node_(node) {
                head_ = node;
                prev_ = node;
                down_ = true;
                idx__ = -1;
                entr_ = {};
                spin_();
            }

            entry &operator * () {
                return entr_;
            }

            void spin_() {
                while (head_ != nullptr) {
                    if (down_ && head_->lns != nullptr) {
                        prev_ = head_;
                        head_ = head_->lns;
                        continue;
                    }

                    if (down_ && head_->rns != nullptr) {
                        prev_ = head_;
                        head_ = head_->rns;
                        continue;
                    }

                    if (down_ || prev_ == head_->rns) {
                        prev_ = head_;
                        head_ = head_->par;
                        down_ = false;

                        if (prev_ != nullptr)
                            entr_ = { prev_->key, prev_->val };
                        idx__++;
                        return;
                    }

                    if (head_->rns != nullptr) {
                        prev_ = head_;
                        head_ = head_->rns;
                        down_ = true;
                        continue;
                    }

                    prev_ = head_;
                    head_ = head_->par;
                    down_ = false;

                    if (prev_ != nullptr)
                        entr_ = { prev_->key, prev_->val };
                    idx__++;
                    return;
                }

                idx__ = -1;
            }

            iterator &operator ++ () {
                spin_();
                return *this;
            }

            bool operator != (const iterator &other) const {
                return idx__ != other.idx__;
            }
        };

        iterator begin() const {
            return iterator(data_);
        }

        iterator end() const {
            return iterator(nullptr);
        }

        static void watch_allocator(void *instance, const long int) {
            auto that_ = static_cast<rb_map *>(instance);
            ::abort(); // reallocation prohibited
        }

        template <typename T>
        T *malloc_(void *, const allocator &allocator, const std::size_t size_n) {
            if (size_n <= 0)
                ::abort();
            return static_cast<T *>(allocator.malloc(size_n * sizeof(T), alignof(T)));
        }

        template <typename T>
        T *realloc_(void *, const allocator &allocator, T *const ptr, const std::size_t size_n) {
            return static_cast<T *>(allocator.realloc(ptr, size_n * sizeof(T)));
        }

        static void free_(void *, const allocator &allocator, void *ptr) {
            if (allocator.free != nullptr && ptr != nullptr)
                allocator.free(ptr);
        }

        static void add_watcher_(const allocator &allocator, void *watcher, const watch_fn_t fn) {
            if (allocator.add_watcher != nullptr)
                allocator.add_watcher(watcher, fn);
        }

        static void del_watcher_(const allocator &allocator, void *watcher, const watch_fn_t fn) {
            if (allocator.del_watcher != nullptr)
                allocator.del_watcher(watcher, fn);
        }

        static bool has_par_(const idx_t idx) {
            return idx > 0;
        }

        static bool has_lns_(const own_t own) {
            return own & HAS_LNS_;
        }

        static bool has_rns_(const own_t own) {
            return own & HAS_RNS_;
        }

        static void set_lns_(own_t *own) {
            (*own) |= HAS_LNS_;
        }

        static void set_rns_(own_t *own) {
            (*own) |= HAS_RNS_;
        }

        static void del_lns_(own_t *own) {
            (*own) &= ~HAS_LNS_;
        }

        static void del_rns_(own_t *own) {
            (*own) &= ~HAS_RNS_;
        }

        static idx_t idx_par_(const idx_t idx) {
            return (idx <= 0) ? 0 : ((idx - 1) / 2);
        }

        static idx_t idx_lns_(const idx_t idx) {
            return (idx * 2) + 1;
        }

        static idx_t idx_rns_(const idx_t idx) {
            return (idx * 2) + 2;
        }

        static bool has_par_(const rb_node *node) {
            return has_par_(node->idx);
        }

        static bool has_lns_(const rb_node *node) {
            return has_lns_(node->own);
        }

        static bool has_rns_(const rb_node *node) {
            return has_rns_(node->own);
        }

        rb_node *par_(const rb_node *node) const {
            if (node == nullptr || !has_par_(node->idx))
                return nullptr;
            return par_(node->idx);
        }

        rb_node *lns_(const rb_node *node) const {
            if (node == nullptr || !has_lns_(node->own))
                return nullptr;
            return lns_(node->idx);
        }

        rb_node *rns_(const rb_node *node) const {
            if (node == nullptr || !has_rns_(node->own))
                return nullptr;
            return rns_(node->idx);
        }

        rb_node *par_(const idx_t index) const {
            return &(data_[idx_par_(index)]);
        }

        rb_node *lns_(const idx_t index) const {
            return &(data_[idx_lns_(index)]);
        }

        rb_node *rns_(const idx_t index) const {
            return &(data_[idx_rns_(index)]);
        }
    };

}

#endif //EX_LIMBO_DATA_RB_MAP_H
