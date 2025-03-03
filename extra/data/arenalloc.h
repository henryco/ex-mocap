//
// Created by henryco on 01/03/25.
//

#ifndef EX_ARENALLOC_H
#define EX_ARENALLOC_H

#include "data_utils.h"

namespace ex::data {

    // ---- DEFAULT FALLBACK (malloc/realloc)
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
        T *request() {
            std::size_t padding = 0;

            if (offset % alignof(T) != 0)
                padding = alignof(T) - (offset % alignof(T));

            const std::size_t required = padding + sizeof(T);
            const long int    remains  = capacity - offset;

            if (required > remains)
                expand(capacity + required);

            offset += required;

            return reinterpret_cast<T *>(static_cast<unsigned char *>(data) + offset - sizeof(T));
        }

        void *request(const std::size_t size, const std::size_t alignment = 0) {
            const std::size_t align   = alignment != 0 ? alignment : size;
            std::size_t       padding = 0;

            if (offset % align != 0)
                padding = align - (offset % align);

            const std::size_t required = padding + size;
            const long int    remains  = capacity - offset;

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
    // ---- DEFAULT FALLBACK (malloc/realloc)

}

namespace ex::data::memory::arena {

    inline void abort(const char *msg) {
        std::cerr << msg << std::endl;
        ::abort();
    }

    using watch_id_t = void *;
    using block_id_t = void *;

    struct memory_block {
        u_int32_t size_b;
        u_int16_t alignment_b;
        u_int8_t  padding_b;
        u_int8_t  freed;
    };

#define DEFAULT_ARENA_SIZE_B 1048576
#define DEFAULT_META_STACK_SIZE_B 10

    inline static ex::data::arena                         arena_ = DEFAULT_ARENA_SIZE_B;
    inline static ex::data::map<watch_id_t, watch_fn_t>   watch_ = DEFAULT_META_STACK_SIZE_B;
    inline static ex::data::map<block_id_t, memory_block> block_ = DEFAULT_META_STACK_SIZE_B;

    inline void free(const block_id_t ptr) {

        const int block_curr = block_.index_at(ptr);
        const int block_prev = block_curr - 1;
        const int block_next = block_curr + 1;

        if (block_.val_entries[block_curr].freed > 0)
            abort("memory double free");

        int block_target = block_curr;

        // x p c n x
        // x p [c <- n] x
        // x p c _ x
        // x [p -> c] x
        // x _ c _ x

        // NEXT BLOCK
        if (block_curr < block_.size - 1) {
            if (const memory_block &next  = block_.at_index(block_next); next.freed > 0) {
                      memory_block &block = block_.at(ptr);

                const block_id_t ptr_should_next = static_cast<unsigned char *>(ptr) + block.size_b;
                const block_id_t ptr_next = block_.key_entries[block_next];

                if (ptr_should_next != ptr_next)
                    abort("memory fragmentation error: next block misalignment");

                block.size_b += (next.size_b + next.padding_b);
                block_.collapse_at_pos(block_next);
            }
        }

        // PREV BLOCK
        if (block_curr > 0) {
            if (const memory_block &prev  = block_.at_index(block_prev); prev.freed > 0) {
                      memory_block &block = block_.at(ptr);

                const block_id_t ptr_should_prev = static_cast<unsigned char *>(ptr) - (block.padding_b + prev.size_b);
                const block_id_t ptr_prev = block_.key_entries[block_prev];

                if (ptr_should_prev != ptr_prev)
                    abort("memory fragmentation error: prev block misalignment");

                block.size_b += (prev.size_b + block.padding_b);
                block.padding_b = prev.padding_b;

                block_.key_entries[block_curr] = block_.key_entries[block_prev];
                block_.collapse_at_pos(block_prev);
                block_target = block_prev;
            }
        }

        block_.key_entries[block_target] =
                static_cast<void *>(static_cast<unsigned char *>(block_.key_entries[block_target]) -
                                    block_.val_entries[block_target].padding_b);

        block_.val_entries[block_target].alignment_b = 0;
        block_.val_entries[block_target].padding_b   = 0;
        block_.val_entries[block_target].freed       = 1;
    }

    inline void shrink(memory_block &block, const std::size_t size_new_b) {
        // TODO

    }

    inline block_id_t reallocate_locally(const block_id_t ptr, const std::size_t size_new_b) {
        memory_block &curr_block = block_.at(ptr);

        if (curr_block.size_b > size_new_b) {
            shrink(curr_block, size_new_b);
            return ptr;
        }

        const int     curr_index = block_.index_at(ptr);
        const size_t  align      = curr_block.alignment_b;

        const int prev_index = curr_index - 1;
        const int next_index = curr_index + 1;

        const size_t curr_size = curr_block.size_b;
        const size_t req_size  = curr_size + size_new_b;

        // TODO

        return nullptr;
    }

    inline block_id_t allocate_locally(const std::size_t size_new_b, const std::size_t alignment_b) {
        const std::size_t align = alignment_b != 0 ? alignment_b : size_new_b;

        for (int i = 0; i < block_.size; ++i) {
            memory_block &block = block_.val_entries[i];
            if (block.freed == 0)
                continue;
            if (block.size_b >= size_new_b) {

                void *const data_ptr = block_.key_entries[i];

                const u_int64_t offset =
                        reinterpret_cast<u_int64_t>(data_ptr) - reinterpret_cast<u_int64_t>(arena_.data);

                int64_t padding = 0;
                if (offset % align != 0)
                    padding = align - (offset % align);

                const u_int32_t total_size = block.size_b;
                const int64_t   required   = padding + size_new_b;
                const int64_t   remain     = total_size - required;

                if (remain < 0)
                    continue;

                block.size_b      = static_cast<u_int32_t>(size_new_b);
                block.alignment_b = static_cast<u_int16_t>(align);
                block.padding_b   = static_cast<u_int8_t>(padding);
                block.freed       = static_cast<u_int8_t>(false);

                const block_id_t new_block_id = static_cast<unsigned char *>(data_ptr) + padding;
                block_.key_entries[i] = new_block_id;

                if (remain > 0) {
                    const auto ptr_remain = static_cast<void *>(static_cast<unsigned char *>(data_ptr) + required);
                    block_.key_entries.insert(i + 1, ptr_remain);
                    block_.val_entries.insert(i + 1, {
                        .size_b      = static_cast<u_int32_t>(remain),
                        .alignment_b = static_cast<u_int16_t>(0),
                        .padding_b   = static_cast<u_int8_t>(0),
                        .freed       = static_cast<u_int8_t>(true),
                    });
                }

                return new_block_id;
            }
        }

        return nullptr;
    }

    inline block_id_t allocate(const std::size_t size_new_b, const std::size_t alignment_b) {
        if (size_new_b == 0)
            return nullptr;

        if (const block_id_t locally_allocated = allocate_locally(size_new_b, alignment_b))
            return locally_allocated;

        const void *old_data_ptr = arena_.data;
        const long  old_offset   = arena_.offset;
        void       *allocated    = arena_.request(size_new_b, alignment_b);
        const long  new_offset   = arena_.offset;
        const void *new_data_ptr = arena_.data;

        // 0 1 2 3 4 5 6 7
        // x x _ x x x > x

        block_.collapse(allocated);
        block_.put(allocated, {
            .size_b      = static_cast<u_int32_t>(size_new_b),
            .alignment_b = static_cast<u_int16_t>(alignment_b),
            .padding_b   = static_cast<u_int8_t>((new_offset - old_offset - size_new_b)),
            .freed       = static_cast<u_int8_t>(false),
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

    inline block_id_t reallocate(const block_id_t ptr, const std::size_t size_new_b) {
        if (size_new_b == 0) {
            free(ptr);
            return nullptr;
        }

        if (const block_id_t locally_allocated = reallocate_locally(ptr, size_new_b)) {
            // TODO
            return locally_allocated;
        }

        memory_block &old_block  = block_.at(ptr);

        if (old_block.size_b > size_new_b) {
            shrink(old_block, size_new_b);
            return ptr;
        }

        const void *old_data_ptr = arena_.data;
        void       *allocated    = nullptr;

        if (const block_id_t last_block_ptr = block_.key_entries[block_.size - 1]; last_block_ptr != ptr) {

            const long old_offset = arena_.offset;
            allocated             = arena_.request(size_new_b, old_block.alignment_b);
            const long new_offset = arena_.offset;

            const u_int16_t old_alignment_b = old_block.alignment_b;
            memcpy(allocated, ptr, old_block.size_b);
            free(ptr);

            block_.put(allocated, {
                .size_b      = static_cast<u_int32_t>(size_new_b),
                .alignment_b = old_alignment_b,
                .padding_b   = static_cast<u_int8_t>((new_offset - old_offset - size_new_b)),
                .freed       = static_cast<u_int8_t>(false),
            });

        } else {
            const size_t req_size = size_new_b - old_block.size_b;

            const long old_offset = arena_.offset;
            allocated             = arena_.request(req_size, old_block.alignment_b);
            const long new_offset = arena_.offset;

            if ((new_offset - old_offset - req_size) != 0)
                abort("reallocation padding mismatch (SHOULD: 0)"); // WTF, padding MUST be 0

            old_block.size_b = size_new_b;
        }

        const void *new_data_ptr = arena_.data;

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

    inline void release(const int size_new_b = DEFAULT_ARENA_SIZE_B) {
        arena_.clean();
        block_.clean();
        if (size_new_b >= 0)
            arena_.reserve(size_new_b);
    }

    inline void add_watcher(const watch_id_t instance, const watch_fn_t fn) {
        watch_.put(instance, fn);
    }

    inline void del_watcher(const watch_id_t instance, watch_fn_t) {
        watch_.remove(instance);
    }

    inline long size() {
        return arena_.offset;
    }

    inline void nop(void *, void *, std::size_t) {
        /*NOP*/
    }

    inline static ex::data::allocator allocator = {
        &allocate, &reallocate, &free, &add_watcher, &del_watcher, &size
    };

}

#endif //EX_ARENALLOC_H
