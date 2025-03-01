//
// Created by henryco on 01/03/25.
//

#ifndef EX_DATALLOC_H
#define EX_DATALLOC_H

#include <cstdlib>
#include <cstring>

namespace ex::data {

    using malloc_fn_t  = void *(*) (void *instance, std::size_t size_new_b, std::size_t alignment_b);
    using realloc_fn_t = void *(*) (void *instance, std::size_t size_old_b, std::size_t size_new_b, std::size_t alignment_b, void *ptr);
    using watch_fn_t   = void  (*) (void *instance, long int ptr_offset);
    using free_fn_t    = void  (*) (void *instance, void *ptr);
    using reg_fn_t     = void  (*) (void *ptr, watch_fn_t watch_fn);
    using size_fn_t    = long  (*) ();

    inline void *default_malloc_(void *instance, std::size_t size_new_b, std::size_t alignment_b);
    inline void *default_realloc_(void *instance, std::size_t size_old_b, std::size_t size_new_b, std::size_t alignment_b, void *ptr);
    inline void default_free_(void *instance, void *ptr);
    inline void default_reg_(void *ptr, watch_fn_t watch_fn);
    inline long default_size_();

    struct allocator {
        malloc_fn_t  malloc      = &default_malloc_;
        realloc_fn_t realloc     = &default_realloc_;
        free_fn_t    free        = &default_free_;
        reg_fn_t     add_watcher = &default_reg_;
        reg_fn_t     del_watcher = &default_reg_;
        size_fn_t    size        = &default_size_;
    };

    void *default_malloc_(void *, const std::size_t size_new_b, std::size_t) {
        return ::malloc(size_new_b);
    }

    void *default_realloc_(void *, std::size_t, const std::size_t size_new_b, std::size_t, void *ptr) {
        return ::realloc(ptr, size_new_b);
    }

    void default_free_(void *, void *ptr) {
        ::free(ptr);
    }

    inline void default_reg_(void *, watch_fn_t) {
    }

    inline long default_size_() {
        return 0;
    }

}

#endif //EX_DATALLOC_H
