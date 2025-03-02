//
// Created by henryco on 01/03/25.
//

#ifndef EX_ARENALLOC_H
#define EX_ARENALLOC_H

#include "data_utils.h"

namespace ex::data {

    struct arena {

        void       *data     = nullptr;
        std::size_t offset   = 0;
        std::size_t capacity = 0;

        arena() = default;

        ~arena() {
            if (data == nullptr)
                return;
            ::free(data);
        }

        arena(const std::size_t &reserve) {
            expand(reserve);
        }

        void reserve(const std::size_t n) {
            if (n <= capacity)
                return;

            capacity = n;

            if (data == nullptr)
                data = ::malloc(capacity);
            else
                data = ::realloc(data, capacity);
        }

        void expand(const std::size_t n) {
            reserve(capacity + n);
        }

        void clean() {
            offset   = 0;
            capacity = 0;
            if (data == nullptr)
                return;
            ::free(data);
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
                ::free(this->data);

            this->data     = other.data;
            this->offset   = other.offset;
            this->capacity = other.capacity;

            other.data = nullptr;
            return *this;
        }
    };

}

namespace ex::data::memory::arena {

    using watch_id_t = void *;
    using block_id_t = void *;

    struct memory_block {
        u_int32_t size;
        u_int16_t alignment;
        u_int8_t  padding;
        u_int8_t  freed;
    };

#define DEFAULT_ARENA_SIZE_B 1048576
#define DEFAULT_META_STACK_SIZE_B 10

    inline static ex::data::arena                         arena_ = DEFAULT_ARENA_SIZE_B;
    inline static ex::data::map<watch_id_t, watch_fn_t>   watch_ = DEFAULT_META_STACK_SIZE_B;
    inline static ex::data::map<block_id_t, memory_block> block_ = DEFAULT_META_STACK_SIZE_B;

    inline static long free_offset = 0;
    inline static long alignment   = 1;

    static void calc_alignment(const long align_b) {
        const long max = alignment > align_b ? alignment : align_b;
        const long min = alignment > align_b ? align_b : alignment;
        alignment      = (max % min == 0) ? max : (max * min);
    }

    static void free(const block_id_t ptr) {
        memory_block &block = block_.at(ptr);
        block.freed = true;

        // TODO
    }

    static block_id_t allocate(const std::size_t size_new_b, const std::size_t alignment_b) {
        if (size_new_b == 0 || alignment_b == 0)
            abort();

        calc_alignment(alignment_b);

        const void *old_data_ptr = arena_.data;
        const long  old_offset   = arena_.offset;
        void       *allocated    = arena_.allocate(size_new_b, alignment_b);
        const long  new_offset   = arena_.offset;
        const void *new_data_ptr = arena_.data;

        block_.remove(allocated);
        block_.put(allocated, {
            .size      = static_cast<u_int32_t>(size_new_b),
            .alignment = static_cast<u_int16_t>(alignment_b),
            .padding   = static_cast<u_int8_t>((new_offset - old_offset - size_new_b)),
            .freed     = static_cast<u_int8_t>(false),
        });

        if (old_data_ptr == new_data_ptr)
            return allocated;

        const long int offset = reinterpret_cast<unsigned long int>(new_data_ptr) -
                                reinterpret_cast<unsigned long int>(old_data_ptr);

        if (offset == 0)
            return allocated;

        for (int i = 0; i < watch_.size; ++i) {
            if (const auto fn = watch_.val_entries[i])
                fn(watch_.key_entries[i], offset);
        }

        return allocated;
    }

    static block_id_t reallocate(const block_id_t ptr, const std::size_t size_new_b) {
        if (size_new_b == 0)
            abort();

        memory_block &old_block = block_.at(ptr);

        const void *old_data_ptr = arena_.data;
        const long  old_offset   = arena_.offset;
        void       *allocated    = arena_.allocate(size_new_b, old_block.alignment);
        const long  new_offset   = arena_.offset;
        const void *new_data_ptr = arena_.data;

        const block_id_t last_block_ptr = block_.key_entries[block_.size - 1];
        if (last_block_ptr != ptr) {
            free_offset = 0;

            memcpy(allocated, ptr, old_block.size);
            free(last_block_ptr);

            allocated = reinterpret_cast<block_id_t>(reinterpret_cast<unsigned long int>(allocated) + free_offset);

            block_.put(allocated, {
                .size      = static_cast<u_int32_t>(size_new_b),
                .alignment = old_block.alignment,
                .padding   = static_cast<u_int8_t>((new_offset - old_offset - size_new_b)),
                .freed     = static_cast<u_int8_t>(false),
            });

            free_offset = 0;
        } else {
            if ((new_offset - old_offset - size_new_b) != 0)
                abort(); // WTF, padding MUST be 0

            old_block.size += size_new_b;
        }

        if (old_data_ptr == new_data_ptr)
            return allocated;

        const long int offset = reinterpret_cast<unsigned long int>(new_data_ptr) -
                                reinterpret_cast<unsigned long int>(old_data_ptr);

        if (offset == 0)
            return allocated;

        for (int i = 0; i < watch_.size; ++i) {
            if (const auto fn = watch_.val_entries[i])
                fn(watch_.key_entries[i], offset);
        }

        return allocated;
    }

    static void release(const int size_new_b = DEFAULT_ARENA_SIZE_B) {
        alignment = 1;
        arena_.clean();
        block_.clean();
        if (size_new_b >= 0)
            arena_.reserve(size_new_b);
    }

    static void add_watcher(const watch_id_t instance, const watch_fn_t fn) {
        watch_.put(instance, fn);
    }

    static void del_watcher(const watch_id_t instance, watch_fn_t) {
        watch_.remove(instance);
    }

    static long size() {
        return arena_.offset;
    }

    static void nop(void *, void *, std::size_t) {
        /*NOP*/
    }

    inline static ex::data::allocator allocator = {
        &allocate, &reallocate, &free, &add_watcher, &del_watcher, &size
    };

}

#endif //EX_ARENALLOC_H
