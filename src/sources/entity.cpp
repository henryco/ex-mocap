//
// Created by xd on 4/21/25.
//

#include "../../extra/data/struct/array.h"
#include "../headers/entity.h"

#include <cstdlib>

namespace ex {

    namespace internal_state {
        struct TreeNodeCT {
            TreeNode    node;
            id_type_32 *children_d = nullptr;
            id_type_32  children_n = 0;
        };

        static data::array<TreeNodeCT> entities   = 1;
        static data::array<id_type_32> free_stack = 1;
        static bool                    dirty      = true;

        bool check_contains(const id_type_32 node_id) {
            return node_id > internal_state::entities.size || node_id == ID32_UNSET;
        }
    }

    TreeNode *entity::create() {
        using container_t     = internal_state::TreeNodeCT;
        internal_state::dirty = true;

        if (!internal_state::free_stack.empty()) {
            const id_type_32 new_id = internal_state::free_stack.back();
            internal_state::free_stack.pop();
            container_t &ct = internal_state::entities[new_id];
            ct.node.state   = node_state::blank;
            ct.node.id_node = new_id;
            return &ct.node;
        }
        const id_type_32 new_id = internal_state::entities.push();
        container_t     &ct     = internal_state::entities[new_id];
        ct.node.state           = node_state::blank;
        ct.node.id_node         = new_id;
        return &ct.node;
    }

    TreeNode *entity::create(const id_type_32 new_id) {
        using container_t     = internal_state::TreeNodeCT;
        internal_state::dirty = true;

        internal_state::entities.insert(new_id);
        container_t &ct = internal_state::entities[new_id];
        ct.node.state   = node_state::blank;
        ct.node.id_node = new_id;
        return &ct.node;
    }

    TreeNode *entity::find(const id_type_32 node_id) {
        if (!internal_state::check_contains(node_id))
            return nullptr;
        return &internal_state::entities[node_id].node;
    }

    entity::IdArray entity::children(const id_type_32 node_id) {
        if (!internal_state::check_contains(node_id))
            return { nullptr, 0 };
        using container_t     = internal_state::TreeNodeCT;
        const container_t &ct = internal_state::entities[node_id];
        return { .data = ct.children_d, .size = ct.children_n };
    }

    TreeNode *entity::parent(const id_type_32 node_id) {
        if (const TreeNode *node = find(node_id))
            find(node->id_parent);
        return nullptr;
    }

    TreeNode *entity::root(const id_type_32 node_id) {
        TreeNode *head = find(node_id);
        if (head == nullptr)
            return nullptr;
        if (!internal_state::check_contains(head->id_parent))
            return nullptr;

        while (true) {
            if (TreeNode *parent = find(head->id_parent)) {
                head = parent;
                continue;
            }
            return head;
        }
    }

    int entity::add_child(const id_type_32 id_parent, const id_type_32 id_child) {
        internal_state::dirty = true;

        return 0; // TODO
    }

    void entity::del_child(const id_type_32 id_parent, const id_type_32 id_child) {
        internal_state::dirty = true;

        // TODO
    }

    void entity::remove(const id_type_32 id_node) {
        TreeNode *node = find(id_node);
        if (node == nullptr)
            return;

        node->name      = nullptr; // potential mem leak (not big tho)
        node->state     = node_state::deleted;
        node->id_type   = ID64_UNSET;
        node->id_node   = ID32_UNSET;
        node->id_link   = ID32_UNSET;
        node->id_parent = ID32_UNSET;

        // TODO UPDATE PARENT/CHILDREN

        internal_state::free_stack.sort_insert_once(id_node);
        internal_state::dirty = true;
    }

    void entity::set_dirty(const bool dirty) {
        internal_state::dirty = dirty;
    }

    bool entity::is_dirty() {
        return internal_state::dirty;
    }

}
