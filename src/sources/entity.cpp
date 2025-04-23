//
// Created by xd on 4/21/25.
//

#include "../../extra/data/struct/array.h"
#include "../headers/entity.h"

#include <cstdlib>

#ifndef ex_e_malloc_fn_
#define ex_e_malloc_fn_(size_b) (::malloc(size_b))
#endif

#ifndef ex_e_realloc_fn_
#define ex_e_realloc_fn_(ptr, size_b) (::realloc(ptr, size_b))
#endif

#ifndef ex_e_free_fn_
#define ex_e_free_fn_(ptr) (::free(ptr))
#endif

namespace ex {

    namespace internal_state {

        void *malloc__(const ::size_t size) {
            void *ptr = ex_e_malloc_fn_(size);
            if (ptr == nullptr)
                abort_("Cannot allocate enough memory");
            return ptr;
        }

        void *realloc__(void *ptr, const ::size_t size) {
            void *ptr_2 = ex_e_realloc_fn_(ptr, size);
            if (ptr_2 == nullptr)
                abort_("Cannot allocate enough memory");
            return ptr_2;
        }

        void free__(void *ptr) {
            if (ptr == nullptr)
                return;
            ex_e_free_fn_(ptr);
        }

        struct ArrayInt {
            id_type_32 *data = nullptr;
            id_type_32  size = 0;
            id_type_32  cap  = 0;

            static void deep_copy_array(ArrayInt *to, const ArrayInt *from) {
                if (to == nullptr || from == nullptr)
                    return;

                if (from->data == nullptr) {
                    *to = *from;
                    return;
                }

                id_type_32 *data = static_cast<id_type_32 *>(malloc__(from->size * sizeof(id_type_32)));
                ::memcpy(data, from->data, from->size * sizeof(id_type_32));

                to->size = from->size;
                to->cap  = from->cap;
                to->data = data;
            }

            static void free(ArrayInt *ptr) {
                if (ptr == nullptr)
                    return;

                if (ptr->data != nullptr)
                    free__(ptr->data);

                ptr->data = nullptr;
                ptr->size = 0;
                ptr->cap  = 0;
            }
        };

        struct TreeNodeCT {
            TreeNode node;
            ArrayInt children;
        };

        struct EntityState {
            data::array<TreeNodeCT> entities;
            data::array<id_type_32> free_stack;
            bool                    dirty = true;

            void reset() {
                for (int i = 0; i < entities.size; ++i)
                    ArrayInt::free(&entities[i].children);
                free_stack.clean();
                entities.clean();
                dirty = true;
            }
        };

        static EntityState state;

        static data::array<TreeNodeCT> &entities   = state.entities;
        static data::array<id_type_32> &free_stack = state.free_stack;
        static bool                    &dirty      = state.dirty;

        bool check_contains(const id_type_32 node_id) {
            return !(node_id > internal_state::entities.size || node_id == ID32_UNSET);
        }
    }

    struct entity::_snapshot {
        internal_state::EntityState state;
    };

    TreeNode *entity::create() {
        using container_t     = internal_state::TreeNodeCT;
        internal_state::dirty = true;

        if (!internal_state::free_stack.empty()) {
            const id_type_32 new_id = internal_state::free_stack.back();
            internal_state::free_stack.pop();
            container_t &ct = internal_state::entities[new_id];
            ct.node.state     = node_state::blank;
            ct.node.name      = nullptr;
            ct.node.id_node   = new_id;
            ct.node.id_type   = ID64_UNSET;
            ct.node.id_link   = ID32_UNSET;
            ct.node.id_parent = ID32_UNSET;
            return &ct.node;
        }

        const id_type_32 new_id = internal_state::entities.push();
        container_t     &ct     = internal_state::entities[new_id];
        ct.node.state     = node_state::blank;
        ct.node.name      = nullptr;
        ct.node.id_node   = new_id;
        ct.node.id_type   = ID64_UNSET;
        ct.node.id_link   = ID32_UNSET;
        ct.node.id_parent = ID32_UNSET;
        return &ct.node;
    }

    TreeNode *entity::create(const id_type_32 new_id) {
        using container_t     = internal_state::TreeNodeCT;
        internal_state::dirty = true;

        const int idx_1 = internal_state::entities.size;

        internal_state::entities.insert(new_id);
        container_t &ct = internal_state::entities[new_id];
        ct.node.state     = node_state::blank;
        ct.node.name      = nullptr;
        ct.node.id_node   = new_id;
        ct.node.id_type   = ID64_UNSET;
        ct.node.id_link   = ID32_UNSET;
        ct.node.id_parent = ID32_UNSET;

        const int idx_2 = internal_state::entities.size;

        for (int i = idx_1; i < idx_2 - 1; ++i)
            internal_state::free_stack.sort_insert_once(i);

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
        return { .data = ct.children.data, .size = ct.children.size };
    }

    int entity::index_of(const id_type_32 node_id) {
        if (!internal_state::check_contains(node_id))
            return -1;

        using container_t     = internal_state::TreeNodeCT;
        const container_t &tc = internal_state::entities[node_id];

        if (!internal_state::check_contains(tc.node.id_parent))
            return -1;

        const container_t &pc = internal_state::entities[tc.node.id_parent];

        if (pc.children.data == nullptr)
            return -1; // probably should abort

        for (int i = 0; i < pc.children.size; ++i) {
            if (pc.children.data[i] == node_id)
                return i;
        }

        return -1; // probably should abort
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

    int entity::add_child(const id_type_32 id_parent, const id_type_32 id_child, const int index) {
        internal_state::dirty = true;

        if (!internal_state::check_contains(id_parent))
            return -1;

        if (!internal_state::check_contains(id_child))
            return -1;

        using container_t = internal_state::TreeNodeCT;
        container_t &pc   = internal_state::entities[id_parent];
        container_t &tc   = internal_state::entities[id_child];

        if (tc.node.id_parent == id_parent)
            return -1;

        if (tc.node.id_parent != ID32_UNSET)
            del_child(tc.node.id_parent, id_child);

        if (pc.children.data == nullptr) {
            pc.children.data = static_cast<id_type_32 *>(internal_state::malloc__(sizeof(id_type_32) * 8));
            pc.children.size = 0;
            pc.children.cap  = 8;
        }

        if (pc.children.size >= pc.children.cap) {
            const id_type_32 new_size = pc.children.size * 2;
            pc.children.cap           = new_size;
            pc.children.data          = static_cast<id_type_32 *>(
                    internal_state::realloc__(pc.children.data, sizeof(id_type_32) * new_size));
        }

        const int idx = index < 0 ? pc.children.size : index;
        ::memmove(pc.children.data + idx + 1,
            pc.children.data + idx, (pc.children.size - idx) * sizeof(id_type_32));

        tc.node.id_parent     = id_parent;
        pc.children.data[idx] = id_child;
        pc.children.size++;

        return idx;
    }

    void entity::del_child(const id_type_32 id_parent, const id_type_32 id_child) {
        internal_state::dirty = true;

        if (!internal_state::check_contains(id_parent))
            return;

        if (!internal_state::check_contains(id_child))
            return;

        using container_t = internal_state::TreeNodeCT;
        container_t &pc   = internal_state::entities[id_parent];
        container_t &tc   = internal_state::entities[id_child];

        if (tc.node.id_parent != id_parent)
            return;

        tc.node.id_parent = ID32_UNSET;

        if (pc.node.state >>= node_state::deleted)
            return;

        if (pc.children.size == 0 || pc.children.data == nullptr)
            return;

        for (int i = 0; i < pc.children.size; ++i) {
            if (pc.children.data[i] == id_child) {
                ::memmove(pc.children.data + i, pc.children.data + i + 1,
                          (pc.children.size - i - 1) * sizeof(id_type_32));
                pc.children.size--;
                return;
            }
        }
    }

    void entity::remove(const id_type_32 node_id) {
        if (!internal_state::check_contains(node_id))
            return;

        using container_t = internal_state::TreeNodeCT;
        container_t &ct   = internal_state::entities[node_id];

        TreeNode &node = ct.node;
        if (node.state >>= node_state::deleted)
            return;

        if (node.id_parent != ID32_UNSET)
            del_child(node.id_parent, node_id);

        node.name      = nullptr; // potential mem leak (not big tho)
        node.state     = node_state::deleted;
        node.id_type   = ID64_UNSET;
        node.id_node   = ID32_UNSET;
        node.id_link   = ID32_UNSET;
        node.id_parent = ID32_UNSET;

        for (int i = 0; i < ct.children.size; ++i)
            remove(ct.children.data[i]);

        internal_state::ArrayInt::free(&ct.children);

        internal_state::free_stack.sort_insert_once(node_id);
        internal_state::dirty = true;
    }

    void entity::set_dirty(const bool dirty) {
        internal_state::dirty = dirty;
    }

    bool entity::is_dirty() {
        return internal_state::dirty;
    }

    void entity::reset_entities() {
        internal_state::state.reset();
    }

    entity::snapshot entity::store_entities() {
        snapshot s = static_cast<_snapshot *> (internal_state::malloc__(sizeof(_snapshot)));

        using arr_t_1 = data::array<internal_state::TreeNodeCT>;
        using arr_t_2 = data::array<id_type_32>;

        arr_t_1::deep_copy_set(&s->state.entities, &internal_state::state.entities);
        arr_t_2::deep_copy_set(&s->state.free_stack, &internal_state::state.free_stack);
        s->state.dirty = internal_state::state.dirty;

        for (int i = 0; i < s->state.entities.size; ++i) {
            auto *ptr = &s->state.entities[i].children;
            internal_state::ArrayInt::deep_copy_array(ptr, ptr);
        }

        return s;
    }

    void entity::restore_entities(const entity::snapshot s) {
        internal_state::state.reset();

        internal_state::state = s->state;
        internal_state::dirty = true;

        for (int i = 0; i < s->state.entities.size; ++i) {
            auto       *ptr_to   = &internal_state::state.entities[i].children;
            const auto *ptr_from = &s->state.entities[i].children;
            internal_state::ArrayInt::deep_copy_array(ptr_to, ptr_from);
        }
    }

    void entity::remove_snapshot(const entity::snapshot s) {
        if (s) s->state.reset();
        internal_state::free__(s);
    }

}
