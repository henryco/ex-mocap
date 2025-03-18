//
// Created by henryco on 01/03/25.
//

#ifndef EX_DATALLOC_H
#define EX_DATALLOC_H

#include <cstdlib>

namespace ex::data {

#define KB * 1024
#define MB * 1024 KB
#define GB * 1024 MB
#define TB * 1024 GB

    using malloc_fn_t  = void *(*) (std::size_t size_new_b, std::size_t alignment_b);
    using realloc_fn_t = void *(*) (void *ptr, std::size_t size_new_b);
    using watch_fn_t   = void  (*) (void *instance, long int ptr_offset);
    using reg_fn_t     = void  (*) (void *ptr, watch_fn_t watch_fn);
    using free_fn_t    = void  (*) (void *ptr);
    using size_fn_t    = long  (*) ();
    using watch_id_t   = void *;

    inline void *default_malloc_(const std::size_t size_new_b, std::size_t) {
        void *mem = ::malloc(size_new_b);
        if (mem == nullptr)
            ::abort();
        return mem;
    }

    inline void *default_realloc_(void *ptr, const std::size_t size_new_b) {
        void *mem = ::realloc(ptr, size_new_b);
        if (mem == nullptr)
            ::abort();
        return mem;
    }

    inline void default_free_(void *ptr) {
        ::free(ptr);
    }

    inline void default_reg_(void *, watch_fn_t) {
        /*NOP*/
    }

    inline long default_size_() {
        return 0;
    }

    struct allocator {
        malloc_fn_t  malloc      = &default_malloc_;
        realloc_fn_t realloc     = &default_realloc_;
        free_fn_t    free        = &default_free_;
        reg_fn_t     add_watcher = &default_reg_;
        reg_fn_t     del_watcher = &default_reg_;
        size_fn_t    size_total  = &default_size_;
        size_fn_t    size_used   = &default_size_;
    };

}

#endif //EX_DATALLOC_H
