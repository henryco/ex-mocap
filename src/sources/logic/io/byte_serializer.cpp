//
// Created by xd on 10/02/25.
//

#include "../../../headers/logic/io/byte_serializer.h"
#include "../../../extra/bytebox/byte_box.h"
#include "../../../headers/state.h"
#include <cstring>

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

    #define TYPE_NODE_ARR 1
    #define TYPE_NODE_NAM 2
    #define TYPE_NODE_OBJ 3
    #define TYPE_NODE_DAT 4
    #define TYPE_NODE_NOD 5

    namespace box = limbo::bytebox;
    namespace out = box::out;
    namespace in  = box::in;

    struct alignas(32) tree_node_1 {
        std::uint64_t id_node    = 0;
        std::uint32_t children_n = 0;
        std::uint32_t data_size  = 0;
        std::uint32_t id_local   = 0;
        std::uint32_t id_link    = 0;
        std::uint32_t index      = 0;
        std::uint32_t flags      = 0;
    };

    std::uint32_t flags_node(const TreeNode *node) {
        std::uint32_t flags = 0;

        flags |= (node->is_group << 1);
        flags |= (node->is_link << 2);
        flags |= (node->disabled << 3);
        flags |= (0 << 4);  // ignore updated
        flags |= (node->deleted << 5);
        flags |= (node->moved << 6);
        flags |= (node->dimmed << 7);
        flags |= (node->warning << 8);
        flags |= (1 << 9);  // always created
        flags |= (0 << 10); // ignore selected
        flags |= (0 << 11); // ignore connected

        return flags;
    }

    tree_node_1 *from_node(const TreeNode *node) {
        return new tree_node_1(
            node->id_node,
            node->children_n,
            node->data_size,
            node->id_local,
            node->id_link,
            node->index,
            flags_node(node)
        );
    }

    void write_tree(out::buffer &buffer, const TreeNode *node, box::array<tree_node_1 *> &tmp) {
        if (node == nullptr)
            return;

        const auto serial = from_node(node);
        tmp.push(serial);

        out::object(buffer, TYPE_NODE_NAM, std::strlen(node->name) + 1, node->name);
        out::object(buffer, TYPE_NODE_OBJ, sizeof(tree_node_1), serial);
        out::object(buffer, TYPE_NODE_DAT, node->data_size, node->data);

        if (node->children_n <= 0 || node->children == nullptr)
            return;

        out::begin_array(buffer, TYPE_NODE_ARR);
        for (auto i = 0; i < node->children_n; i++)
            write_tree(buffer, node->children[i], tmp);
        out::end_array(buffer);
    }

    byte_array ByteSerializer::serialize(const void *in) const {
        const auto state = static_cast<const ex::State *>(in);
        const auto root  = state->work_tree;

        auto buff_out = out::create_buffer(10);

        box::array<tree_node_1 *> tmp(10);
        write_tree(buff_out, root, tmp);

        std::size_t size = 0;
        const auto data = out::read_buffer_data(buff_out, &size);
        return byte_array::from(data, size);
    }

    void *ByteSerializer::deserialize(const byte_array &in) const {
        // todo
        return nullptr;
    }

} // namespace ex::io::bytes
