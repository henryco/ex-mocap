//
// Created by xd on 4/11/25.
//

#ifndef EX_LIMBO_DATA_BYTE_ARRAY_H
#define EX_LIMBO_DATA_BYTE_ARRAY_H

#include <iostream>
#include <cstdlib>

#define abort_(msg) do { std::cerr << msg << std::endl; ::abort(); } while(false)

namespace ex::data {

    /**
     * Designed for manual memory management, RAII GTFO
     */
    struct byte_array {
        using free_fn = void (*)(void *);

        free_fn     del_;
        ::u_int8_t *data;
        ::u_int64_t size;

        ::u_int8_t &operator [] (const ::size_t &) const;
        ::u_int8_t *operator +  (const ::size_t &) const;
        byte_array  operator >> (const ::size_t &) const;
        byte_array  operator << (const ::size_t &) const;
        bool        empty() const;

        static byte_array from(::u_int8_t *, const ::size_t &, free_fn = nullptr);
        static void       free(byte_array *);
        static void       free(byte_array &);
    };

    inline ::u_int8_t &byte_array::operator [] (const ::size_t &index) const {
        if (index >= size)
            abort_("index out of bounds");
        return data[index];
    }

    inline ::u_int8_t *byte_array::operator + (const ::size_t &index) const {
        return data + index;
    }

    inline byte_array byte_array::operator >> (const ::size_t &offset) const {
        return byte_array::from(data + offset, size - offset, del_);
    }

    inline byte_array byte_array::operator << (const ::size_t &offset) const {
        return byte_array::from(data - offset, size + offset, del_);
    }

    inline bool byte_array::empty() const {
        return data != nullptr && size != 0;
    }

    inline byte_array byte_array::from(::u_int8_t *data, const ::size_t &size, const free_fn free_) {
        return byte_array(free_, data, static_cast<::u_int64_t>(size));
    }

    inline void byte_array::free(byte_array *array) {
        if (array == nullptr || array->del_ == nullptr) {
            array->data = nullptr;
            array->size = 0;
            return;
        }
        array->del_(array->data);
        array->data = nullptr;
        array->size = 0;
    }

    inline void byte_array::free(byte_array &array) {
        if (array.del_ == nullptr) {
            array.data = nullptr;
            array.size = 0;
            return;
        }
        array.del_(array.data);
        array.data = nullptr;
        array.size = 0;
    }
}

namespace ex::io {
    // backward compatibility naming
    using byte_array = ex::data::byte_array;
}

#endif //EX_LIMBO_DATA_BYTE_ARRAY_H
