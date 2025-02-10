//
// Created by xd on 10/02/25.
//

#include "../../../headers/logic/io/byte_serializer.h"
#include "../../../extra/bytebox/byte_box.h"
#include "../../../headers/state.h"

namespace ex::io::bytes {

    byte_array ByteSerializer::serialize(const void *in) const {
        const auto state = static_cast<const ex::State *>(in);

        // todo
        return {};
    }

    void *ByteSerializer::deserialize(const byte_array &in) const {
        // todo
        return nullptr;
    }

} // namespace ex::io::bytes
