//
// Created by xd on 10/02/25.
//

#include "../../../extra/bytebox/byte_box.h"
#include "../../headers/io/byte_serializer.h"
#include "../../headers/logic/logic_mappers.h"
#include "../../headers/logic/logic_globals.h"
#include "../../headers/logic/logic_state.h"
#include "../../headers/state.h"

#include <ex_version.h>

#include <cstring>
#include <iostream>

namespace ex::io::err {
    void abort(const char *msg) {
        std::cerr << msg << std::endl;
        std::abort();
    }
}

namespace ex::io::alc {
    static unsigned char *create_byte_arr_(const ::size_t size) {
        return static_cast<unsigned char *>(::malloc(size));
    }

    static void release_byte_arr_(void *ptr) {
        ::free(ptr);
    }
}

namespace ex::io::bytes {

    static constexpr std::uint64_t MAGIC_NUMBER = 0x65786D6F74696F6E;

    #define TYPE_NODE_BGN 1
    #define TYPE_NODE_ARR 2
    #define TYPE_NODE_NAM 3
    #define TYPE_NODE_OBJ 4
    #define TYPE_NODE_DAT 5
    #define TYPE_NODE_NOD 6

    namespace box = limbo::bytebox;
    namespace out = box::out;
    namespace in  = box::in;

    struct alignas(4) project_version {
        std::uint8_t major = 0;
        std::uint8_t minor = 0;
        std::uint8_t patch = 0;
        std::uint8_t _____ = 0;
    };

    struct alignas(16) project_metadata {
        std::uint64_t   magic_number;
        project_version version;
    };

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
            ex::io::err::abort("Fatal Error (3): tree_node_1 is NULL");
            return;
        }

        to->id_node    = from->id_node;
        to->children_n = from->children_n;
        to->data_size  = from->data_size;
        to->id_local   = from->id_local;
        to->id_link    = from->id_link;
        to->index      = from->index;
        to->state      = static_cast<node_state>(from->flags);
        to->children   = new TreeNode *[to->children_n];

        if (to->state >>= node_state::is_link)
            return;

        if (to->state >>= node_state::is_group)
            return;

        if (!ex::logic::node_mappers().contains(to->id_node))
            return;

        const auto mappers = ex::logic::node_mappers()[to->id_node];
        const auto instance = mappers.instance(to);
        ex::logic::state::store_node(to->id_local, instance);
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
                this->data = static_cast<unsigned char *>(::malloc(size));
                this->size = size;
            } else {
                this->data = static_cast<unsigned char *>(::realloc(this->data, size));
                this->size = size;
            }
        }

        ~byte_buffer() {
            if (this->data != nullptr)
                ::free(this->data);
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

            if (const auto s = dynamic_cast<serializable *>(ex::logic::state::get_node(node->id_local)))  {
                if (const auto serialized = s->serialize(); !serialized.empty()) {
                    // write base node data
                    out::object(buffer, TYPE_NODE_DAT, serialized.size, serialized.data);
                    garbage_collector.tmp_2.push(serialized);
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
//        std::cout << "in: " << head->name << std::endl;
        bool prev_arr_end = false;

        std::int64_t i = in::offset(buffer);
        TreeNode    *node = nullptr;

        while ((i = in::next(buffer, data + i)) != -1) {

//            std::cout << "\n-> i: " << i << std::endl;

            if (in::begin_array(buffer)) {
                if (in::array_type(buffer) == TYPE_NODE_BGN) {

//                    std::cout << ">> TYPE_NODE_BGN\n";
//                    std::cout << "   head: " << head->name << std::endl;

                    node = TreeNode::create();
                }

                else if (in::array_type(buffer) == TYPE_NODE_ARR) {

//                    std::cout << ">> TYPE_NODE_ARR\n";
//                    std::cout << "   head: " << head->name << std::endl;
//                    std::cout << "   node: " << node->name << std::endl;

                    read_tree(buffer, data, bb, node);
                    i = in::offset(buffer);

//                    std::cout << "-- TYPE_NODE_ARR\n";
//                    std::cout << "   head: " << head->name << std::endl;
//                    std::cout << "   node: " << node->name << std::endl;
                }

                else if (in::array_type(buffer) == TYPE_ARRAY) {

//                    std::cout << ">> TYPE_ARRAY\n";
//                    std::cout << "   head: " << head->name << std::endl;

                    // very beginning
                }
            }

            if (const auto obj_size_b = in::next_object(buffer)) {
                if (node == nullptr) {
                    ex::io::err::abort("Fatal Error (1): TreeNode is NULL");
                    return;
                }

                if (in::item_type(buffer) == TYPE_NODE_NAM) {

//                    std::cout << "** TYPE_NODE_NAM\n";
//                    std::cout << "   head: " << head->name << std::endl;

                    bb.allocate(obj_size_b);
                    ::memcpy(bb.data, data + i, obj_size_b);

                    const auto new_name = alc::create_byte_arr_(obj_size_b);
                    ::memcpy(new_name, bb.data, obj_size_b);
                    node->name = reinterpret_cast<const char *>(new_name);

//                    std::cout << "   node: " << node->name << std::endl;

                }

                else if (in::item_type(buffer) == TYPE_NODE_OBJ) {

//                    std::cout << "** TYPE_NODE_OBJ\n";
//                    std::cout << "   head: " << head->name << std::endl;
//                    std::cout << "   node: " << node->name << std::endl;

                    to_node(reinterpret_cast<const tree_node_1 *>(data + i), node);
                    ex::state::uuid::set_last_id(node->id_local);
                }

                else if (in::item_type(buffer) == TYPE_NODE_DAT) {

//                    std::cout << "** TYPE_NODE_DAT\n";
//                    std::cout << "   head: " << head->name << std::endl;
//                    std::cout << "   node: " << node->name << std::endl;

                    if (node->state >>= node_state::is_link)
                        continue;

                    const auto base_node = ex::logic::state::get_node(node->id_local);
                    if (base_node == nullptr)
                        continue;

                    if (const auto s = dynamic_cast<serializable *>(base_node)) {
                        s->deserialize(byte_array::from(data + i, obj_size_b));
                    }
                }
            }

            if (in::end_array(buffer)) {
                if (in::array_type(buffer) == TYPE_NODE_BGN) {

//                    std::cout << "<< TYPE_NODE_BGN\n";
//                    std::cout << "   head: " << head->name << std::endl;

                    if (prev_arr_end)
                        return;

                    if (node == nullptr) {
                        ex::io::err::abort("Fatal Error (2): TreeNode is NULL");
                        return;
                    }

//                    std::cout << "   node: " << node->name << std::endl;

                    head->children[node->index] = node;
                    node->parent                = head;
                    node = nullptr;
                }

                else if (in::array_type(buffer) == TYPE_NODE_ARR) {

//                    std::cout << "<< TYPE_NODE_ARR\n";
//                    std::cout << "   head: " << head->name << std::endl;

                    prev_arr_end = true;
                }

                else if (in::array_type(buffer) == TYPE_ARRAY) {

//                    std::cout << "<< TYPE_ARRAY\n";
//                    std::cout << "   head: " << head->name << std::endl;

                    i = (-1);
                }
            }
        }
    }

    byte_array ByteSerializer::serialize(const void *in) const {
        const auto state = static_cast<const ex::State *>(in);
        const auto root  = state->logic.work_tree;

        auto buff_out = out::create_buffer(10);

        gc garbage_collector;
        write_tree(buff_out, root, garbage_collector);

        const std::size_t size      = out::buffer_size_b(buff_out);
        const std::size_t real_size = sizeof(project_metadata) + size;

        const auto data = alc::create_byte_arr_(real_size);

        constexpr project_metadata metadata = {
            .magic_number = MAGIC_NUMBER,
            .version = {
                .major = PROJECT_VERSION_MAJOR,
                .minor = PROJECT_VERSION_MINOR,
                .patch = PROJECT_VERSION_PATCH
            }
        };

        ::memcpy(data, &metadata, sizeof(project_metadata));
        out::read_buffer_data(buff_out, data + sizeof(project_metadata));

        return byte_array::from(data, real_size, &alc::release_byte_arr_);
    }

    void *ByteSerializer::deserialize(const byte_array &data) const {
        const auto state = new ex::State;

        if (data.size < sizeof(project_metadata)) {
            state->error.code = 1;
            state->error.msg  = "Invalid file format (not enough space for header)";
            return state;
        }

        project_metadata metadata;
        ::memcpy(&metadata, data.data, sizeof(project_metadata));

        logic::globals::logger()->debug("File version: {}.{}.{}", metadata.version.major, metadata.version.minor,
                                            metadata.version.patch);

        if (metadata.magic_number != MAGIC_NUMBER) {
            state->error.code = 1;
            state->error.msg  = "File format not supported, magic number missmatch";
            return state;
        }

        if (metadata.version.major > PROJECT_VERSION_MAJOR) {
            state->error.code = 1;
            state->error.msg  = "File format version not supported";
            return state;
        }

        in::buffer  buffer = in::create_buffer(64, in::mode::OFFSET);
        byte_buffer bb;
        bb.allocate(64);

        TreeNode base   = TreeNode::dummy();
        base.name       = "base";
        base.id_local   = 141006542;
        base.children   = new TreeNode *[1];
        base.children_n = 1;

        state::uuid::reset_last_id();

        const auto t0 = std::chrono::high_resolution_clock::now();

        read_tree(buffer, data >> sizeof(project_metadata), bb, &base);

        const auto t1 = std::chrono::high_resolution_clock::now();

        state->logic.work_tree         = base.children[0];
        state->logic.work_tree->parent = nullptr;

        base.children[0] = nullptr;
        base.children_n  = 0;

        logic::globals::logger()->debug("deserialization time: {} ns", (t1 - t0).count());

        return state;
    }

}
