//
// Created by xd on 10/02/25.
//

#ifndef BYTE_SERIALIZER_H
#define BYTE_SERIALIZER_H

#include "../../../extra/nodes/nodes.h"

namespace ex::io::bytes {

    struct ByteSerializer: limbo::nodes::Serializer<unsigned char *> {
        ~ByteSerializer() override = default;

        unsigned char *serialize(const void *) const override;

        void *deserialize(unsigned char *const &) const override;
    };

}

#endif //BYTE_SERIALIZER_H
