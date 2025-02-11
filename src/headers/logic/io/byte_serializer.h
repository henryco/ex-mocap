//
// Created by xd on 10/02/25.
//

#ifndef BYTE_SERIALIZER_H
#define BYTE_SERIALIZER_H

#include "../../../extra/nodes/nodes.h"
#include <cstdint>

namespace ex::io {
    struct byte_array {
        const std::uint8_t *data;
        std::uint64_t       size;
        const std::uint8_t &operator[](const std::size_t &index) const;
        bool                empty() const;
        static byte_array   from(const std::uint8_t *data, const std::size_t &size);
        static void         free(byte_array *array);
        static void         free(byte_array &array);
    };
}

namespace ex::io::bytes {

    struct ByteSerializer: limbo::nodes::Serializer<byte_array> {
        ~ByteSerializer() override = default;

        byte_array serialize(const void *) const override;

        void *deserialize(const byte_array &) const override;
    };

}

#endif //BYTE_SERIALIZER_H
