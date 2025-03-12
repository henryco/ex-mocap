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

    };

}

#endif //EX_LIMBO_DATA_RMQ_MAP_H
