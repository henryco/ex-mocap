//
// Created by xd on 10/02/25.
//

#ifndef BYTE_SERIALIZER_H
#define BYTE_SERIALIZER_H

#include "../../../extra/nodes/nodes.h"
#include <cstdint>

namespace ex::io::bytes {

    struct byte_array {
        const std::uint8_t *data;
    };

    struct ByteSerializer: limbo::nodes::Serializer<std::uint8_t *> {
        ~ByteSerializer() override = default;

        std::uint8_t *serialize(const void *) const override;

        void *deserialize(std::uint8_t *const &) const override;
    };

}

#endif //BYTE_SERIALIZER_H
