//
// Created by xd on 10/02/25.
//

#include "../../../headers/logic/io/byte_serializer.h"
#include "../../../extra/bytebox/byte_box.h"
#include "../../../headers/state.h"

const std::uint8_t &ex::io::byte_array::operator[](const size_t &index) const {
    return data[index];
}

bool ex::io::byte_array::empty() const {
    return data != nullptr && size != 0;
}

ex::io::byte_array ex::io::byte_array::from(const std::uint8_t *data, const size_t &size) {
    return ex::io::byte_array(data, static_cast<std::uint64_t>(size));
}

void ex::io::byte_array::free(byte_array *array) {
    if (array == nullptr)
        return;
    limbo::bytebox::out::free_buffer_data(array->data);
    array->data = nullptr;
}

namespace ex::io::bytes {

    namespace box = limbo::bytebox;
    namespace out = box::out;
    namespace in  = box::in;

    void write_tree(out::buffer &buffer, const TreeNode *node) {
        // TODO
    }

    byte_array ByteSerializer::serialize(const void *in) const {
        const auto state = static_cast<const ex::State *>(in);
        const auto root  = state->work_tree;

        auto buff_out = out::create_buffer(10);

        write_tree(buff_out, root);

        std::size_t size = 0;
        const auto data = out::read_buffer_data(buff_out, &size);
        return byte_array::from(data, size);
    }

    void *ByteSerializer::deserialize(const byte_array &in) const {
        // todo
        return nullptr;
    }

} // namespace ex::io::bytes
