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
        stack<element> stack_(1);
        stack_.push(element_array());
        buff.stack.push(stack_);
        return buff;
    }

    void begin_array(buffer &buff) {
        stack<element> stack_(1);
        stack_.push(element_array());
        buff.stack.push(stack_);
    }

    void end_array(buffer &buff) {
        // TODO
    }

    void object(buffer &buff, std::uint64_t type, std::uint32_t size, const void *data) {
        // TODO
    }

    const unsigned char *read_buffer(buffer &buff, std::size_t &size) {
        return nullptr;
    }

}