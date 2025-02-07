//
// Created by xd on 07/02/25.
//

#include "../../../headers/logic/io/byte_box.h"

namespace ex::io::box {

    element element_array() {
        element e;
        e.header = { .size_b = sizeof(header), .size_n = 0, .type = TYPE_ARRAY };
        e.data   = nullptr;
        return e;
    }

    buffer create_buffer() {
        buffer buff;
        buff.items.push(element_array());
        return buff;
    }

    void begin_array(buffer &buff) {
        buffer _buff;
        _buff.items.push(element_array());
        // TODO FIXME
        buff.stack.push(_buff);
    }

    void end_array(buffer &buff) {
        // TODO
    }

    void object(buffer &buff, const std::uint64_t type, const std::uint32_t size, const void *data) {
        const std::uint32_t b_size = static_cast<std::uint32_t>(sizeof(header)) + size;

        auto &[items, stack] = buff.stack.empty() ? buff : buff.stack[buff.stack.size - 1];
        buff.items.push({ .header = { .size_b = b_size, .size_n = 1, .type = type }, .data = data });

        items[0].header.size_b += b_size;
        items[0].header.size_n += 1;
    }

    const unsigned char *read_buffer(buffer &buff, const std::size_t &size) {
        // TODO
        return nullptr;
    }

}