//
// Created by xd on 10/02/25.
//

#include "../../../headers/logic/io/byte_serializer.h"
#include "../../../extra/bytebox/byte_box.h"
#include "../../../headers/state.h"

namespace ex::io::bytes {

    std::uint8_t *ByteSerializer::serialize(const void *in) const {
        const auto state = static_cast<const ex::State *>(in);

        // todo
        return nullptr;
    }

    void *ByteSerializer::deserialize(std::uint8_t *const &in) const {
        // todo
        return nullptr;
    }

} // namespace ex::io::bytes
