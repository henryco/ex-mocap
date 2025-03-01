//
// Created by henryco on 01/03/25.
//

#ifndef EX_ARENALLOC_H
#define EX_ARENALLOC_H

#include "data_utils.h"

namespace ex::data::memory::arena {

#define DEFAULT_ARENA_SIZE_B 1048576

    inline static ex::data::arena arena_ = DEFAULT_ARENA_SIZE_B;
    inline static ex::data::map<void *, ex::data::watch_fn_t> watchers_ = 10;

    static void *allocate(void *instance, const std::size_t size_new_b, const std::size_t alignment_b) {

        const void *old_data_ptr = arena_.data;
        void *const allocated    = arena_.allocate(size_new_b, alignment_b);
        const void *new_data_ptr = arena_.data;

        if (old_data_ptr == new_data_ptr)
            return allocated;

        const long int offset = reinterpret_cast<unsigned long int>(new_data_ptr) -
                                reinterpret_cast<unsigned long int>(old_data_ptr);

        if (offset == 0)
            return allocated;

        for (int i = 0; i < watchers_.size; ++i) {
            void *that = watchers_.key_entries[i];
            if (that == instance)
                continue;
            if (const auto fn = watchers_.val_entries[i])
                fn(that, offset);
        }

        return allocated;
    }

    static void *reallocate(void *instance, const std::size_t size_old_b, const std::size_t size_new_b,
                            const std::size_t alignment_b, void *ptr) {
        void *const new_data = allocate(instance, size_new_b, alignment_b);
        memcpy(new_data, ptr, size_old_b);
        return new_data;
    }

    static void free(const int size_new_b = DEFAULT_ARENA_SIZE_B) {
        arena_.clean();
        if (size_new_b >= 0)
            arena_.reserve(size_new_b);
    }

    static void add_watcher(void *instance, const ex::data::watch_fn_t fn) {
        watchers_.put(instance, fn);
    }

    static void del_watcher(void *instance, ex::data::watch_fn_t) {
        watchers_.remove(instance);
    }

    static long size() {
        return arena_.offset;
    }

    inline static ex::data::allocator allocator = { &allocate, &reallocate, nullptr, &add_watcher, &del_watcher, &size };
}

#endif //EX_ARENALLOC_H
