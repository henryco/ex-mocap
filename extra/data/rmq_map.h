//
// Created by xd on 3/12/25.
//

#ifndef EX_LIMBO_DATA_RMQ_MAP_H
#define EX_LIMBO_DATA_RMQ_MAP_H

#include "datalloc.h"
#include <new>

namespace ex::data {

    /**
     * Range-Min/Max-Query Tree based on top of a Red-Black Tree implementation of Map
     */
    template<typename key_t, typename val_t, typename rmq_t = key_t>
    struct rmq_map {

        struct entry {
            key_t key;
            val_t val;
            rmq_t rmq;
        };

        struct it_entry {
            key_t key;
            val_t val;
            rmq_t rmq;
            rmq_t min;
            rmq_t max;
        };

        using col_t = bool;
#define BLACK_        false
#define RED_          true

        struct rmq_node {
            rmq_node *par;
            rmq_node *lns;
            rmq_node *rns;
            rmq_t     min;
            rmq_t     max;
            rmq_t     rmq;
            val_t     val;
            key_t     key;
            col_t     col;
        };

#ifdef UNIT_TEST
        using metrics_t = int;
        inline static metrics_t metrics_n_ = 0;
#endif

        using comparator_fn = int32_t (*)(const key_t *a, const key_t *b);
        using min_max_eq_fn = int16_t (*)(const rmq_t *a, const rmq_t *b);

        static int32_t default_comparator(const key_t *a, const key_t *b) {
            return (*a > *b) - (*a < *b);
        }

        static int16_t default_min_max_eq(const rmq_t *a, const rmq_t *b) {
            return (*a > *b) - (*a < *b);
        }

        allocator allocator_;

        comparator_fn comp_;
        min_max_eq_fn mmeq_;
        int64_t       size_;
        rmq_node     *root_;

        rmq_map():
            comp_(&rmq_map::default_comparator),
            mmeq_(&rmq_map::default_min_max_eq),
            size_(0), root_(nullptr) {
        }

        rmq_map(const comparator_fn compare_key):
            comp_(compare_key),
            mmeq_(&rmq_map::default_min_max_eq),
            size_(0), root_(nullptr) {
            add_watcher_(allocator_, this, &watch_allocator);
        }

        rmq_map(const min_max_eq_fn compare_range):
            comp_(&rmq_map::default_min_max_eq),
            mmeq_(compare_range),
            size_(0), root_(nullptr) {
            add_watcher_(allocator_, this, &watch_allocator);
        }

        rmq_map(const comparator_fn compare_key, const min_max_eq_fn compare_range):
            comp_(compare_key),
            mmeq_(compare_range),
            size_(0), root_(nullptr) {
            add_watcher_(allocator_, this, &watch_allocator);
        }

        rmq_map(const allocator &allocator_):
            allocator_(allocator_),
            comp_(&rmq_map::default_comparator),
            mmeq_(&rmq_map::default_min_max_eq),
            size_(0), root_(nullptr) {
            add_watcher_(allocator_, this, &watch_allocator);
        }

        rmq_map(const allocator &allocator_, const comparator_fn compare_key):
            allocator_(allocator_),
            comp_(compare_key),
            mmeq_(&rmq_map::default_min_max_eq),
            size_(0), root_(nullptr) {
            add_watcher_(allocator_, this, &watch_allocator);
        }

        rmq_map(const allocator &allocator_, const min_max_eq_fn compare_range):
            allocator_(allocator_),
            comp_(&rmq_map::default_comparator),
            mmeq_(compare_range),
            size_(0), root_(nullptr) {
            add_watcher_(allocator_, this, &watch_allocator);
        }

        rmq_map(const allocator &allocator_, const comparator_fn compare_key, const min_max_eq_fn compare_range):
            allocator_(allocator_),
            comp_(compare_key),
            mmeq_(compare_range),
            size_(0), root_(nullptr) {
            add_watcher_(allocator_, this, &watch_allocator);
        }

        rmq_map(const rmq_map &other):
            allocator_(other.allocator_),
            comp_(other.comp_),
            mmeq_(other.mmeq_),
            size_(other.size_) {
            root_ = other.iterative_copy_(other.root_);
            add_watcher_(allocator_, this, &watch_allocator);
        }

        rmq_map(rmq_map &&other) noexcept:
            allocator_(other.allocator_),
            comp_(other.comp_),
            mmeq_(other.mmeq_),
            size_(other.size_),
            root_(other.root_) {
            other.root_ = nullptr;
            other.size_ = 0;
            del_watcher_(other.allocator_, this, &watch_allocator);
            add_watcher_(allocator_, this, &watch_allocator);
        }

        rmq_map &operator = (const rmq_map &other) {
            if (this == &other)
                return *this;
            clean();
            del_watcher_(allocator_, this, &watch_allocator);
            root_      = other.iterative_copy_(other.root_);
            allocator_ = other.allocator_;
            comp_      = other.comp_;
            mmeq_      = other.mmeq_;
            size_      = other.size_;
            add_watcher_(allocator_, this, &watch_allocator);
            return *this;
        }

        rmq_map &operator = (rmq_map &&other) noexcept {
            if (this == &other)
                return *this;
            clean();
            del_watcher_(other.allocator_, this, &watch_allocator);
            del_watcher_(allocator_, this, &watch_allocator);
            allocator_ = other.allocator_;
            comp_      = other.comp_;
            mmeq_      = other.mmeq_;
            size_      = other.size_;
            root_      = other.root_;
            add_watcher_(allocator_, this, &watch_allocator);
            other.root_ = nullptr;
            other.size_ = 0;
            return *this;
        }

        ~rmq_map() {
            del_watcher_(allocator_, this, &watch_allocator);
            iterative_free_(root_);
            root_ = nullptr;
        }

        void clean() {
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

        rmq_node *next(const key_t &key) const {
            int direction;
#ifdef UNIT_TEST
            rmq_node *node = descend_(root_, key, &direction, &metrics_n_);
#else
            rmq_node *node = descend_(root_, key, &direction);
#endif
            if (node == nullptr || direction != 0 || comp_(&key, &(node->key)) != 0)
                ::abort();
            if (node->rns != nullptr) {
                if (rmq_node *min_ = minimum_(node->rns))
                    return min_;
                ::abort();
            }
            for (rmq_node *head = node;;) {
                rmq_node *parent = head->par;
                if (parent == nullptr)
                    return nullptr;
                if (parent->lns == head)
                    return parent;
                head = parent;
            }
        }

        rmq_node *prev(const key_t &key) const {
            int direction;
#ifdef UNIT_TEST
            rmq_node *node = descend_(root_, key, &direction, &metrics_n_);
#else
            rmq_node *node = descend_(root_, key, &direction);
#endif
            if (node == nullptr || direction != 0 || comp_(&key, &(node->key)) != 0)
                ::abort();
            if (node->lns != nullptr) {
                if (rmq_node *max_ = maximum_(node->lns))
                    return max_;
                ::abort();
            }
            for (rmq_node *head = node;;) {
                rmq_node *parent = head->par;
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
            rmq_node *node = descend_(root_, key, &direction, &metrics_n_);
#else
            rmq_node *node = descend_(root_, key, &direction);
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
            rmq_node *node = descend_(root_, key, &direction, &metrics_n_);
#else
            rmq_node *node = descend_(root_, key, &direction);
#endif

            if (node == nullptr)
                ::abort(); // all or nothing, avoid exceptions

            if (direction == 0 && comp_(&key, &(node->key)) == 0)
                return node->val;

            ::abort(); // all or nothing, avoid exceptions
        }

        const val_t &operator [] (const key_t &key) const {
            return at(key);
        }

        void put(const entry &entry) {
            put(entry.key, entry.val, entry.rmq);
        }

        void put(const key_t &key, const val_t &val) {
            put(key, val, key);
        }

        void put(const key_t &key, const val_t &val, const rmq_t &range) {

            if (root_ == nullptr) {
                root_      = instance_node_();
                root_->par = nullptr;
                root_->lns = nullptr;
                root_->rns = nullptr;
                root_->val = val;
                root_->key = key;
                root_->rmq = range;
                root_->min = range;
                root_->max = range;
                root_->col = BLACK_;
                ++size_;
                return;
            }

            int direction;

#ifdef UNIT_TEST
            rmq_node *base = descend_(root_, key, &direction, &metrics_n_);
#else
            rmq_node *base = descend_(root_, key, &direction);
#endif

            if (direction == 0 && comp_(&key, &(base->key)) == 0) {
                base->rmq = range;
                base->val = val;
                update_ranges_(base);
                return;
            }

            ++size_;

            rmq_node *node = instance_node_();
            node->par      = base;
            node->lns      = nullptr;
            node->rns      = nullptr;
            node->val      = val;
            node->key      = key;
            node->rmq      = range;
            node->min      = range;
            node->max      = range;
            node->col      = RED_;

            if (direction >= 0) base->rns = node;
            else                base->lns = node;

            update_ranges_(node);
            fix_insert_if_needed_(node);

            // if root_ is nullptr we have much bigger problem
            root_->col = BLACK_; // just in case
        }

        void remove(const key_t &key) {
            int direction;
#ifdef UNIT_TEST
            rmq_node *node = descend_(root_, key, &direction, &metrics_n_);
#else
            rmq_node *node = descend_(root_, key, &direction);
#endif
            if (node == nullptr)
                return;
            if (direction != 0 || comp_(&key, &(node->key)) != 0)
                return;
            remove_node_(node);
        }

        void range_set(const key_t &key, const rmq_t &range) {
            int direction;
#ifdef UNIT_TEST
            rmq_node *node = descend_(root_, key, &direction, &metrics_n_);
#else
            rmq_node *node = descend_(root_, key, &direction);
#endif
            if (node == nullptr)
                return;

            if (direction != 0 || comp_(&key, &(node->key)) != 0)
                return;

            node->rmq = range;
            update_ranges_(node);
        }

        // TODO TEST
        bool range_overlap(const rmq_t &min, const rmq_t &max) const {
            return overlap_(root_, min, max);
        }

        // TODO TEST
        bool range_overlap(const rmq_t &val) const {
            return overlap_(root_, val, val);
        }

        // TODO TEST
        rmq_node *range_fit(const rmq_t &min, const rmq_t &max) const {
            if (!overlap_(root_, min, max))
                return nullptr;

            for (rmq_node *head = root_; head != nullptr;) {
                if (mmeq_(&min, &(head->rmq) <= 0) &&
                    mmeq_(&max, &(head->rmq) >= 0))
                    return head;

                if (overlap_(head->lns, min, max)) {
                    head = head->lns;
                    continue;
                }

                if (overlap_(head->rns, min, max)) {
                    head = head->rns;
                    continue;
                }

                return nullptr;
            }

            return nullptr;
        }

        rmq_node *range_min() const {
            if (root_ == nullptr)
                return nullptr;
            return range_fit(root_->min, root_->min);
        }

        rmq_node *range_max() const {
            if (root_ == nullptr)
                return nullptr;
            return range_fit(root_->max, root_->max);
        }

        rmq_node *range_minimize(const rmq_t &min) const {
            if (root_ == nullptr)
                return nullptr;
            // -----------x---------<-| |-----------------
            // -----------x-----<-| |---------------------
            // minimizing min/max value

            // -------[---x------]------------------------
            // ---------[-x--------]----------------------
            // -----------x-[----]------------------------
            // d: min_(min,max)
            // r: min_(min_(d_left, d_right), val)

            for (rmq_node *head = root_; head != nullptr;) {
                const rmq_t &n_min = head->min;
                const rmq_t &n_max = head->max;
                const rmq_t &n_val = head->rmq;


                return head;
            }

            return nullptr;
        }

        rmq_node *range_maximize(const rmq_t &max) const {
            if (root_ == nullptr)
                return nullptr;
            // ----------| |->----------x----------------
            // ---------------| |->-----x----------------
            // maximizing min value

            // -----------------[-------x---]------------
            // ----------------[--------x-]--------------
            // ------------------[----]-x----------------
            // d: max_(min,max)
            // r: max_(max_(d_left, d_right), val)

            for (rmq_node *head = root_; head != nullptr;) {
                const rmq_t &n_min = head->min;
                const rmq_t &n_max = head->max;
                const rmq_t &n_val = head->rmq;

                // TODO
                return head;
            }

            return nullptr;
        }

        rmq_t fit_d_min_(const rmq_t &l, const rmq_t &r, const rmq_t &min) const {
            if (mmeq_(&l, &min) < 0)
                return r;
            if (mmeq_(&l, &r) <= 0)
                return l;
            return r;
        }

        rmq_t fit_d_max_(const rmq_t &l, const rmq_t &r, const rmq_t &max) const {
            if (mmeq_(&r, &max) > 0)
                return l;
            if (mmeq_(&r, &l) >= 0)
                return r;
            return l;
        }

        bool overlap_(const rmq_node *node, const rmq_t &min, const rmq_t &max) const {
            if (node == nullptr)
                return false;
            // ------------[-x-x------x-x-]---------- NODE
            // ------[-----x---]--------------------------
            // -----------------------[---x-----]---------
            // --------------[----------]-----------------
            // --------[---x--------------x---]-----------
            return (mmeq_(&max, &(node->min)) >= 0 &&
                    mmeq_(&min, &(node->max)) <= 0);
        }

        void update_ranges_(rmq_node *const node, const rmq_node *stop = nullptr) {
            rmq_node *head = node;
            while (head != nullptr && head != stop) {

                int min_l = head->rmq, max_l = head->rmq;
                int min_r = head->rmq, max_r = head->rmq;

                if (head->lns != nullptr) {
                    min_l = head->lns->min;
                    max_l = head->lns->max;
                }

                if (head->rns != nullptr) {
                    min_r = head->rns->min;
                    max_r = head->rns->max;
                }

                head->min = min_val_(head->rmq, min_val_(min_l, min_r));
                head->max = max_val_(head->rmq, max_val_(max_l, max_r));

                head = head->par;
            }
        }

        void remove_node_(rmq_node *node) {
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

                rmq_node *sibling = get_sibling_(node);
                replace_with_child_(node, nullptr);
                fix_delete_if_needed_(node, nullptr, sibling);
                free_node_(node);

                return;
            }

            if (node->lns != nullptr && node->rns != nullptr) {
                // left and right
                rmq_node *largest = get_largest_(node->lns);
                rmq_node *child   = largest->lns;

                node->key = largest->key;
                node->val = largest->val;

                rmq_node *sibling = get_sibling_(largest);
                replace_with_child_(largest, child);
                fix_delete_if_needed_(largest, child, sibling);
                free_node_(largest);

                return;
            }

            if (node->lns != nullptr && node->rns == nullptr) {
                // left
                rmq_node *child = node->lns;

                if (node == root_) {
                    root_ = child;
                    root_->par = nullptr;
                    root_->col = BLACK_;
                    free_node_(node);
                    return;
                }

                rmq_node *sibling = get_sibling_(node);
                replace_with_child_(node, child);
                fix_delete_if_needed_(node, child, sibling);
                free_node_(node);

                return;
            }

            if (node->rns != nullptr && node->lns == nullptr) {
                // right
                rmq_node *child = node->rns;

                if (node == root_) {
                    root_ = child;
                    root_->par = nullptr;
                    root_->col = BLACK_;
                    free_node_(node);
                    return;
                }

                rmq_node *sibling = get_sibling_(node);
                replace_with_child_(node, child);
                fix_delete_if_needed_(node, child, sibling);
                free_node_(node);

                return;
            }

            ::abort();
        }

        void fix_delete_if_needed_(rmq_node *node, rmq_node *child, rmq_node *sibling) {
            // case 1: deleted was red
            if (node == nullptr || node->col == RED_)
                return;

            // case 2: deleted was black with a red child
            if (child != nullptr && child->col == RED_) {
                child->col = BLACK_;
                return;
            }

            fix_double_black_(child, sibling, node->par);
            if (root_ != nullptr)
                root_->col = BLACK_; // just in case
        }

        void fix_double_black_(rmq_node *current, rmq_node *sibling, rmq_node *parent) {
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

                rmq_node *near = is_left ? sibling->lns : sibling->rns;
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

        void fix_insert_if_needed_(rmq_node *node) {
            rmq_node *parent = node->par;

            if (parent == nullptr) {
                if (node != root_)
                    ::abort();

                // just in case, root is always black
                node->col = BLACK_;
                return;
            }

            if (parent->col == BLACK_)
                return;

            rmq_node *grand_parent = parent->par;
            if (grand_parent == nullptr) {
                // just in case, root is always black
                parent->col = BLACK_;
            }

            rmq_node *uncle = get_sibling_(parent);

            // Case 1: Parent and uncle are both red -> Recolor and recurse up
            if (uncle != nullptr && uncle->col == RED_) {
                parent->col = BLACK_;
                uncle->col  = BLACK_;

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

            if (root_ != nullptr)
                root_->col = BLACK_; // just in case
        }

        rmq_node *descend_(rmq_node *node, const key_t &key, int *direction, int *n = nullptr) const {
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

        void rotate_right_(rmq_node *node) {
            rmq_node *left = node->lns;
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

            update_ranges_(node->par->lns, node->par);
            update_ranges_(node->par->rns, node->par);
            update_ranges_(node->par, node->par->par);
        }

        void rotate_left_(rmq_node *node) {
            rmq_node *right = node->rns;
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

            update_ranges_(node->par->lns, node->par);
            update_ranges_(node->par->rns, node->par);
            update_ranges_(node->par, node->par->par);
        }

        void replace_with_child_(rmq_node *node, rmq_node *child) {
            if (node->par == nullptr)
                root_ = child;
            else if (node == node->par->lns)
                node->par->lns = child;
            else
                node->par->rns = child;

            if (child != nullptr) {
                child->par = node->par;
                update_ranges_(child);
            } else {
                update_ranges_(node->par);
            }
        }

        rmq_node *instance_node_() {
            return new (malloc_<rmq_node>(this, allocator_, 1)) rmq_node;
        }

        rmq_node *copy_node_(const rmq_node *node) const {
            if (node == nullptr)
                return nullptr;
            rmq_node *copy = instance_node_();
            *copy = *node;
            return copy;
        }

        void iterative_copy_(rmq_node *const node) {
            rmq_node *copy = copy_node_(node);
            rmq_node *head = node;
            rmq_node *prev = node;
            bool     down = true;

            while (head != nullptr) {
                if (down && head->lns != nullptr) {
                    prev = head;
                    head = head->lns;

                    *copy->lns = copy_node_(head);
                    copy = copy->lns;
                    continue;
                }

                if (down && head->rns != nullptr) {
                    prev = head;
                    head = head->rns;

                    *copy->rns = copy_node_(head);
                    copy = copy->rns;
                    continue;
                }

                if (down || prev == head->rns) {
                    prev = head;
                    head = head->par;
                    copy = copy->par;
                    down = false;
                    continue;
                }

                if (head->rns != nullptr) {
                    prev = head;
                    head = head->rns;

                    *copy->rns = copy_node_(head);
                    copy = copy->rns;
                    down = true;
                    continue;
                }

                prev = head;
                head = head->par;
                copy = copy->par;
                down = false;
            }
        }


        void free_node_(rmq_node *node) {
            if (node == nullptr)
                return;
            if (node->par != nullptr) {
                if (node->par->lns == node)
                    node->par->lns = nullptr;
                if (node->par->rns == node)
                    node->par->rns = nullptr;
            }
            release_node_(node);
        }

        void release_node_(rmq_node *node) {
            if (node == nullptr)
                return;
            node->~rmq_node();
            free_(this, allocator_, node);
        }

        void iterative_free_(rmq_node *const node) {
            rmq_node *head = node;
            rmq_node *prev = node;
            bool     down = true;

            while (head != nullptr) {
                if (down && head->lns != nullptr) {
                    prev = head;
                    head = head->lns;
                    continue;
                }

                if (down && head->rns != nullptr) {
                    prev = head;
                    head = head->rns;
                    continue;
                }

                if (down || prev == head->rns) {
                    prev = head;
                    head = head->par;
                    down = false;
                    release_node_(prev);
                    continue;
                }

                if (head->rns != nullptr) {
                    prev = head;
                    head = head->rns;
                    down = true;
                    continue;
                }

                prev = head;
                head = head->par;
                down = false;
                release_node_(prev);
            }
        }

        static rmq_node *get_largest_(rmq_node *node) {
            if (node == nullptr)
                return nullptr;
            if (node->rns == nullptr)
                return node;
            return get_largest_(node->rns);
        }

        static rmq_node *get_sibling_(const rmq_node *node) {
            if (node == nullptr)
                ::abort();

            const rmq_node *parent = node->par;
            if (parent == nullptr)
                return nullptr;
            return parent->lns == node ? parent->rns : parent->lns;
        }

        static void swap_colors(rmq_node *a, rmq_node *b) {
            const col_t tmp = a->col;
            a->col = b->col;
            b->col = tmp;
        }

        static rmq_node *minimum_(rmq_node *const node) {
            rmq_node *head = node;
            while (head != nullptr && head->lns != nullptr)
                head = head->lns;
            return head;
        }

        static rmq_node *maximum_(rmq_node *const node) {
            rmq_node *head = node;
            while (head != nullptr && head->rns != nullptr)
                head = head->rns;
            return head;
        }

        // DSF Post-Order iterator
        struct iterator {
            it_entry  entr_;
            rmq_node *node_;
            rmq_node *head_;
            rmq_node *prev_;
            long      idx__;
            bool      down_;

            iterator(rmq_node *node):
                node_(node) {
                head_ = node;
                prev_ = node;
                down_ = true;
                idx__ = -1;
                entr_ = {};
                spin_();
            }

            it_entry &operator * () {
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
                            entr_ = { prev_->key, prev_->val, prev_->rmq, prev_->min, prev_->max };
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
                        entr_ = { prev_->key, prev_->val, prev_->rmq, prev_->min, prev_->max };
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
            return iterator(root_);
        }

        iterator end() const {
            return iterator(nullptr);
        }

        static void watch_allocator(void *instance, const long int) {
            auto that_ = static_cast<rmq_map *>(instance);
            ::abort(); // reallocation prohibited
        }

        template <typename T>
        T *malloc_(void *, const allocator &allocator, const std::size_t size_n) {
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

        template <typename T>
        static T min_val_(const T &a, const T &b) {
            return (a < b) ? a : b;
        }

        template <typename T>
        static T max_val_(const T &a, const T &b) {
            return (a > b) ? a : b;
        }
    };

}

#endif //EX_LIMBO_DATA_RMQ_MAP_H
