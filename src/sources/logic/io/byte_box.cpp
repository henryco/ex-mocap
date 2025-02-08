//
// Created by xd on 07/02/25.
//

#include "../../../headers/logic/io/byte_box.h"
#include <cstring>

namespace ex::io::box {

    inline element element_array() {
        return { .header = { .size_b = sizeof(header), .size_n = 0, .type = TYPE_ARRAY }, .data = nullptr };
    }

    inline element& array_head(buffer &buff) {
        return buff.items[buff.stack[buff.stack.size - 1]];
    }

    buffer create_buffer() {
        buffer buff;
        buff.items.push(element_array());
        buff.stack.push(0);
        return buff;
    }

    void begin_array(buffer &buff) {
        buff.stack.push(buff.items.size);
        buff.items.push(element_array());
    }

    void end_array(buffer &buff) {
        const auto &head = array_head(buff);
        buff.stack.pop();

        auto &curr = array_head(buff);
        curr.header.size_b += head.header.size_b;
        curr.header.size_n += head.header.size_n;
    }

    void object(buffer &buff, const std::uint64_t type, const std::uint32_t size, const void *data) {
        const std::uint32_t b_size = static_cast<std::uint32_t>(sizeof(header)) + size;

        buff.items.push({ .header = { .size_b = b_size, .size_n = 1, .type = type }, .data = data });

        auto &head = array_head(buff);
        head.header.size_b += b_size;
        head.header.size_n += 1;
    }

    const unsigned char *read_buffer_data(buffer &buff, std::size_t *size) {
        const auto &head = array_head(buff);

        *size = head.header.size_b;

        const auto data = new unsigned char[*size];
        for (std::size_t i = 0, k = 0; i < buff.items.size; ++i) {
            const auto &item = buff.items[i];
            std::memcpy(data + k, &item.header, sizeof(header));
            std::memcpy(data + k + sizeof(header), &item.data, item.header.size_b - sizeof(header));
            k += item.header.size_b;
        }

        return data;
    }

    void free_buffer_data(const unsigned char *data) {
        if (data != nullptr)
            return;
        delete[] data;
    }

}