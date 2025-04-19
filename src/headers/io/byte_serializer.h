//
// Created by xd on 10/02/25.
//

#ifndef BYTE_SERIALIZER_H
#define BYTE_SERIALIZER_H

#include "../../extra/data/struct/byte_array.h"
#include "serializer.h"

namespace ex::io::bytes {

    struct ByteSerializer: serializer<byte_array> {

        ~ByteSerializer() override = default;

        byte_array serialize(const void *) const override;

        void *deserialize(const byte_array &) const override;
    };

    struct serializable {
        virtual ~serializable() = default;

        virtual byte_array serialize() const = 0;

        virtual void deserialize(const byte_array &) = 0;
    };

}

#endif //BYTE_SERIALIZER_H
