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
    template<typename key_t, typename val_t, typename rmq_t = val_t>
    struct rmq_map {

        struct entry {
            key_t key;
            val_t val;
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
            val_t     val;
            key_t     key;
            col_t     col;
        };

        using comparator_fn = int8_t (*)(const key_t *a, const key_t *b);
        using min_max_eq_fn = int8_t (*)(const rmq_t *a, const rmq_t *b);

        static int8_t default_min_max_eq(const rmq_t *a, const rmq_t *b) {
            return (*a > *b) - (*a < *b);
        }

        static int8_t default_comparator(const key_t *a, const key_t *b) {
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
    };

}

#endif //EX_LIMBO_DATA_RMQ_MAP_H
