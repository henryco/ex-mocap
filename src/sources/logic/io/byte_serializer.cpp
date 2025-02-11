//
// Created by xd on 10/02/25.
//

#include "../../../extra/bytebox/byte_box.h"
#include <cstring>

#include "../../../headers/logic/io/byte_serializer.h"
#include "../../../headers/logic/logic_globals.h"
#include "../../../headers/state.h"

const std::uint8_t &ex::io::byte_array::operator [] (const std::size_t &index) const {
    return data[index];
}

const std::uint8_t *ex::io::byte_array::operator + (const std::size_t &index) const {
    if (index >= size) {
        std::cerr << "Fatal Error: Attempt to read byte array data out of bound" << std::endl;
        std::abort();
    }
    return data + index;
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

void ex::io::byte_array::free(byte_array &array) {
    limbo::bytebox::out::free_buffer_data(array.data);
    array.data = nullptr;
}

namespace ex::io::bytes {

    #define TYPE_NODE_BGN 1
    #define TYPE_NODE_ARR 2
    #define TYPE_NODE_NAM 3
    #define TYPE_NODE_OBJ 4
    #define TYPE_NODE_DAT 5
    #define TYPE_NODE_NOD 6

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
        std::uint16_t flags      = 0;
        std::uint16_t _____      = 0;
    };

    std::uint16_t flags_node(const TreeNode *node) {
        node_state state = node->state;
        state -= node_state::created;   // ignore created
        state -= node_state::dimmed;    // ignore dimmed
        state -= node_state::selected;  // ignore selected
        state -= node_state::connected; // ignore connected
        return static_cast<uint16_t>(state);
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

    void to_node(const tree_node_1 *from, TreeNode *to) {
        if (from == nullptr) {
            std::cerr << "Fatal Error: tree_node_1 is NULL" << std::endl;
            std::abort();
        }
        to->id_node    = from->id_node;
        to->children_n = from->children_n;
        to->data_size  = from->data_size;
        to->id_local   = from->id_local;
        to->id_link    = from->id_link;
        to->index      = from->index;
        to->state      = static_cast<node_state>(from->flags);
        to->children   = new TreeNode *[to->children_n];
    }

    struct gc {
        box::array<tree_node_1 *> tmp_1 = (10);
        box::array<byte_array>    tmp_2 = (10);

        ~gc() {
            for (std::uint32_t i = 0; i < tmp_1.size; i++)
                delete tmp_1[i];

            for (std::uint32_t i = 0; i < tmp_2.size; i++)
                byte_array::free(&(tmp_2[i]));
        }
    };

    struct byte_buffer {
        unsigned char *data = nullptr;
        std::uint64_t  size = 0;

        void allocate(const std::uint64_t &size) {
            if (size <= this->size)
                return;
            if (this->size <= 0) {
                this->data = static_cast<unsigned char *>(malloc(size));
                this->size = size;
            } else {
                this->data = static_cast<unsigned char *>(realloc(this->data, size));
                this->size = size;
            }
        }

        ~byte_buffer() {
            if (this->data != nullptr)
                std::free(this->data);
            this->data = nullptr;
            this->size = 0;
        }
    };

    /*
     * 0:  [header(16): b(4), n(4), t(8)]      main container header
     *     -----------------------------------
     * 16: - [header(16): b(4), n(4), t(8)]    [TYPE_NODE_BGN]  node structure header
     *
     * 32: - [header(16): b(4), n(4), t(8)]    [TYPE_NODE_NAM]  node name header
     * 48: - [string(N):               ...]                     node name content
     *
     *     - [header(16): b(4), n(4), t(8)]    [TYPE_NODE_OBJ]  node structure header
     *     - [tree_node_1(16):         ...]                     node structure content
     *
     *     - [header(16): b(4), n(4), t(8)]    [TYPE_NODE_DAT]  (optional) node data header
     *     - [data(N):                 ...]                     (optional) node data content
     *
     *     - [header(16): b(4), n(4), t(8)]    [TYPE_NODE_ARR]  (optional) children array header
     *       - ...                                              (optional) children array content
     *       - ...
     *     -----------------------------------
     *     - [header(16): b(4), n(4), t(8)]    [TYPE_NODE_BGN]  node structure header
     *     - ...
     *     - ...
     */

    void write_tree(out::buffer &buffer, const TreeNode *node, gc &garbage_collector) {
        if (node == nullptr)
            return;

        const auto serial = from_node(node);
        garbage_collector.tmp_1.push(serial);

        // begin tree node structure
        out::begin_array(buffer, TYPE_NODE_BGN);
        {
            // write name
            out::object(buffer, TYPE_NODE_NAM, std::strlen(node->name) + 1, node->name);

            // write tree node data
            out::object(buffer, TYPE_NODE_OBJ, sizeof(tree_node_1), serial);

            if (ex::logic::globals::node_store().contains(node->id_local)) {
                if (const auto serializable = dynamic_cast<limbo::nodes::Serialization<ByteSerializer> *>(
                        ex::logic::globals::node_store().at(node->id_local));
                serializable != nullptr)
                {
                    if (const auto serialized = serializable->serialize(); !serialized.empty()) {
                        // write base node data
                        out::object(buffer, TYPE_NODE_DAT, serialized.size, serialized.data);
                        garbage_collector.tmp_2.push(serialized);
                    }
                }
            }

            if (node->children_n <= 0 || node->children == nullptr) {
                out::end_array(buffer);
                return;
            }

            out::begin_array(buffer, TYPE_NODE_ARR);
            {
                for (auto i = 0; i < node->children_n; i++)
                    write_tree(buffer, node->children[i], garbage_collector);
            }
            out::end_array(buffer);

        }
        out::end_array(buffer);
    }

    void read_tree(in::buffer &buffer, const byte_array &data, byte_buffer &bb, TreeNode *head) {
        TreeNode *node = nullptr;

        std::int64_t i = in::offset(buffer);
        while ((i = in::next(buffer, data + i)) != -1) {

            if (in::begin_array(buffer)) {

                if (in::array_type(buffer) == TYPE_NODE_BGN) {
                    node = TreeNode::create();
                }

                else if (in::array_type(buffer) == TYPE_NODE_ARR) {
                    read_tree(buffer, data, bb, node);
                    i = in::offset(buffer);
                }

                else if (in::array_type(buffer) == TYPE_ARRAY)
                    ; // very beginning
            }

            else if (const auto obj_size_b = in::next_object(buffer)) {

                if (node == nullptr) {
                    std::cerr << "Fatal Error: TreeNode is NULL" << std::endl;
                    std::abort();
                }

                bb.allocate(obj_size_b);
                std::memcpy(bb.data, data + i, obj_size_b);

                if (in::item_type(buffer) == TYPE_NODE_NAM) {
                    const auto new_name = new unsigned char[obj_size_b];
                    std::memcpy(new_name, bb.data, obj_size_b);
                    node->name = reinterpret_cast<const char *>(new_name);
                }

                else if (in::item_type(buffer) == TYPE_NODE_OBJ) {
                    to_node(reinterpret_cast<const tree_node_1 *>(bb.data), node);
                    ex::state::uuid::reset_last_id(node->id_local);
                }

                else if (in::item_type(buffer) == TYPE_NODE_DAT) {
                    // TODO node data
                }
            }

            else if (in::end_array(buffer)) {

                if (in::array_type(buffer) == TYPE_NODE_BGN) {
                    if (node == nullptr) {
                        std::cerr << "Fatal Error: TreeNode is NULL" << std::endl;
                        std::abort();
                    }

                    head->children[node->index] = node;
                    node->parent                = head;
                    node = nullptr;
                }

                else if (in::array_type(buffer) == TYPE_NODE_ARR)
                    return; // back to parent

                else if (in::array_type(buffer) == TYPE_ARRAY)
                    ; // should be very end
            }

        }
    }

    byte_array ByteSerializer::serialize(const void *in) const {
        const auto state = static_cast<const ex::State *>(in);
        const auto root  = state->work_tree;

        auto buff_out = out::create_buffer(10);

        gc garbage_collector;
        write_tree(buff_out, root, garbage_collector);

        std::size_t size = 0;
        const auto data = out::read_buffer_data(buff_out, &size);
        return byte_array::from(data, size);
    }

    void *ByteSerializer::deserialize(const byte_array &data) const {
        const auto state = new ex::State;

        in::buffer  buffer = in::create_buffer(10, in::mode::OFFSET);
        byte_buffer bb;

        TreeNode  base = TreeNode::dummy();
        base.children   = new TreeNode *[1];
        base.children_n = 1;

        read_tree(buffer, data, bb, &base);
        state->work_tree = base.children[0];

        base.children[0] = nullptr;
        base.children_n  = 0;

        return state;
    }

} // namespace ex::io::bytes
