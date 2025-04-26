//
// Created by xd on 4/21/25.
//

#ifndef EX_MOTION_ENTITY_H
#define EX_MOTION_ENTITY_H

namespace ex {

    // --- sanity check ---
    static_assert(sizeof(unsigned long int) == 8);
    static_assert(sizeof(unsigned short) == 2);
    static_assert(sizeof(unsigned char) == 1);
    static_assert(sizeof(unsigned int) == 4);
    static_assert(sizeof(void *) == 8);
    // --- sanity check ---

    using id_type_64 = unsigned long int; // 8
    using id_type_32 = unsigned int;      // 4
    using id_type_16 = unsigned short;    // 2

    #define ID64_UNSET ((id_type_64) -1)
    #define ID32_UNSET ((id_type_32) -1)
    #define ID16_UNSET ((id_type_16) -1)

    enum class node_state : id_type_16 {
        blank     = 0,
        deleted   = 1 << 1,
        output    = 1 << 2,
        group     = 1 << 3,
        link      = 1 << 4,
        dimmed    = 1 << 5,
        warning   = 1 << 6,
        selected  = 1 << 7,
        disabled  = 1 << 8,
        connected = 1 << 9,
        section   = 1 << 10, // TODO
    };

    // 32 bytes
    struct TreeNode {
        const char *name      = nullptr;
        id_type_64  id_type   = ID64_UNSET;
        id_type_32  id_node   = ID32_UNSET;
        id_type_32  id_link   = ID32_UNSET;
        id_type_32  id_parent = ID32_UNSET;
        node_state  state     = node_state::blank;
    };

}

namespace ex::entity {

    using snapshot = struct _snapshot *;

    struct IdArray {
        const id_type_32 *data;
        const id_type_32  size;
    };

    TreeNode *create();
    TreeNode *create(id_type_32 id_node);
    TreeNode *find(id_type_32 id_node);
    TreeNode *parent(id_type_32 id_node);
    TreeNode *root(id_type_32 id_node);
    IdArray   children(id_type_32 id_node);
    bool      is_within(id_type_32 id_node, id_type_32 id_within);
    int       index_of(id_type_32 id_node);
    int       add_child(id_type_32 id_parent, id_type_32 id_child, int index = -1);
    void      del_child(id_type_32 id_parent, id_type_32 id_child);
    void      remove(id_type_32 id_node);
    void      rename(id_type_32 id_node, const char *name);

    void set_dirty(bool dirty);
    bool is_dirty();
    void reset_entities();

    snapshot store_entities();
    void     restore_entities(snapshot);
    void     remove_snapshot(snapshot);

}

/** OR */
constexpr ex::node_state operator | (const ex::node_state &lhs, const ex::node_state &rhs) {
    return static_cast<ex::node_state>(static_cast<ex::id_type_16>(lhs) | static_cast<ex::id_type_16>(rhs));
}

/** AND */
constexpr ex::node_state operator & (const ex::node_state &lhs, const ex::node_state &rhs) {
    return static_cast<ex::node_state>(static_cast<ex::id_type_16>(lhs) & static_cast<ex::id_type_16>(rhs));
}

/** Assign XOR */
constexpr ex::node_state operator ^= (ex::node_state &lhs, const ex::node_state &rhs) {
    return lhs = static_cast<ex::node_state>(static_cast<ex::id_type_16>(lhs) ^ static_cast<ex::id_type_16>(rhs));
}

/** Assign OR */
constexpr ex::node_state operator |= (ex::node_state &lhs, const ex::node_state &rhs) {
    return lhs = static_cast<ex::node_state>(static_cast<ex::id_type_16>(lhs) | static_cast<ex::id_type_16>(rhs));
}

/** Assign OR */
constexpr ex::node_state operator += (ex::node_state &lhs, const ex::node_state &rhs) {
    return lhs = static_cast<ex::node_state>(static_cast<ex::id_type_16>(lhs) | static_cast<ex::id_type_16>(rhs));
}

constexpr ex::node_state operator -= (ex::node_state &lhs, const ex::node_state &rhs) {
    return lhs = static_cast<ex::node_state>((~(static_cast<ex::id_type_16>(rhs))) & static_cast<ex::id_type_16>(lhs));
}

constexpr ex::node_state operator && (const ex::node_state &lhs, const bool &rhs) {
    return static_cast<ex::node_state>(rhs ? static_cast<ex::id_type_16>(lhs) : 0);
}

constexpr bool operator >>= (const ex::node_state &lhs, const ex::node_state &rhs) {
    return (static_cast<ex::id_type_16>(lhs) & static_cast<ex::id_type_16>(rhs)) != 0;
}

#endif //EX_MOTION_ENTITY_H
