//
// Created by henryco on 12/03/25.
//

#ifndef EX_LIMBO_DATA_BUFFER_H
#define EX_LIMBO_DATA_BUFFER_H

#include "alloc/datalloc.h"
#include <cstring>

namespace ex::data {

    template<typename element_t>
    struct buffer {

        allocator allocator_;

        element_t *data = nullptr;
        long       size = 0;

        buffer() {
            add_watcher_(allocator_, this, &watch_allocator);
        }

        buffer(const allocator &allocator_) : allocator_(allocator_) {
            add_watcher_(allocator_, this, &watch_allocator);
        }

        buffer(const long &size) : size(size) {
            add_watcher_(allocator_, this, &watch_allocator);
            this->data = malloc_<element_t>(this, allocator_, size);
        }

        buffer(const long &size, const allocator &allocator_) : allocator_(allocator_), size(size) {
            add_watcher_(allocator_, this, &watch_allocator);
            this->data = malloc_<element_t>(this, allocator_, size);
        }

        ~buffer() {
            del_watcher_(allocator_, this, &watch_allocator);
            free_(this, allocator_, data);
            this->data = nullptr;
            this->size = 0;
        }

        void allocate(const long &size) {
            if (size <= this->size)
                return;

            if (this->size <= 0 || this->data == nullptr) {
                this->data = malloc_<element_t>(this, allocator_, size);
                this->size = size;
            }

            else {
                this->data = realloc_<element_t>(this, allocator_, this->data, size);
                this->size = size;
            }
        }

        buffer(const buffer &other) : allocator_(other.allocator_), size(other.size) {
            add_watcher_(allocator_, this, &watch_allocator);
            if (other.size <= 0) {
                data = nullptr;
                return;
            }

            data = malloc_<element_t>(this, other.allocator_, other.size);
            memcpy(data, other.data, other.size * sizeof(element_t));
        }

        buffer(buffer &&other) noexcept : allocator_(other.allocator_), size(other.size) {
            del_watcher_(other.allocator_, &other, &watch_allocator);
            add_watcher_(allocator_, this, &watch_allocator);
            this->data = other.data;
            other.data = nullptr;
        }

        buffer &operator = (const buffer &other) {
            if (this == &other)
                return *this;

            del_watcher_(allocator_, this, &watch_allocator);
            free_(this, allocator_, data);
            data = nullptr;

            allocator_ = other.allocator_;
            size       = other.size;

            if (other.size <= 0) {
                data = nullptr;
                return *this;
            }

            add_watcher_(allocator_, this, &watch_allocator);

            data = malloc_<element_t>(this, other.allocator_, other.size);
            memcpy(data, other.data, other.size * sizeof(element_t));

            return *this;
        }

        buffer &operator = (buffer &&other) noexcept {
            if (this == &other)
                return *this;

            del_watcher_(other.allocator_, &other, &watch_allocator);
            del_watcher_(allocator_, this, &watch_allocator);

            free_(this, allocator_, data);
            data = nullptr;

            this->allocator_ = other.allocator_;
            this->size       = other.size;
            this->data       = other.data;
            other.data       = nullptr;

            add_watcher_(allocator_, this, &watch_allocator);
            return *this;
        }

        static void watch_allocator(void *instance, const long int offset) {
            auto this_ = static_cast<buffer *>(instance);
            this_->data = reinterpret_cast<element_t *>(reinterpret_cast<unsigned char *>(this_->data) + offset);
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

#endif //EX_LIMBO_DATA_BUFFER_H
