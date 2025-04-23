//
// Created by xd on 31/01/25.
//

#include "../../../extra/data/data_utils.h"
#include "../../../headers/utils/err_utils.h"
#include "../../../headers/imgui/tree/gui_tree_ops.h"
#include "../../../headers/imgui/gui_globals.h"
#include "../../../headers/imgui/gui_mappers.h"
#include "../../../headers/imgui/strings/gui_strings.h"
#include "../../../headers/logic/logic_globals.h"

namespace ex::gui::tree {

    namespace i_ {
        const auto &mappers_ = ex::gui::node_mappers();
    }

    struct Snapshot {
        entity::snapshot snap   = nullptr;
        ex::id_type_32   root   = ID32_UNSET;
        ex::id_type_32   active = ID32_UNSET;

        void rollback() {
            auto state = ex::logic::globals::state();
            rollback(&state);
        }

        void rollback(State *s) const {
            ex::entity::restore_entities(snap);
            s->logic.root_node   = root;
            s->logic.active_node = active;
        }

        static Snapshot *create() {
            return create(ex::logic::globals::state());
        }

        static Snapshot *create(State &state) {
            Snapshot *s = static_cast<Snapshot *>(::malloc(sizeof(Snapshot)));
            *s = {
                .snap = ex::entity::store_entities(),
                .root = state.logic.root_node,
                .active = state.logic.active_node
            };
            return s;
        }

        static void free(Snapshot *s) {
            if (s == nullptr)
                return;
            if (s->snap != nullptr)
                ex::entity::remove_snapshot(s->snap);
            ::free(s);
        }
    };

    selection_node *bubble_sort_selection(selection_node *selection) {
        if (selection == nullptr)
            return nullptr;

        const selection_node *node = selection->root;
        selection_node       *root = nullptr;

        while (node != nullptr) {
            auto curr = root;

            if (curr == nullptr) {
                root           = new selection_node;
                root->selected = node->selected;
                root->prev     = nullptr;
                root->next     = nullptr;
                root->root     = root;

                node = node->next;
                continue;
            }

            const int node_idx = ex::entity::index_of(node->selected->id_node);

            while (node_idx > ex::entity::index_of(curr->selected->id_node) && curr->next != nullptr) {
                if (ex::entity::index_of(curr->next->selected->id_node) > node_idx)
                    break;
                curr = curr->next;
            }

            const auto new_node = new selection_node;
            new_node->selected  = node->selected;

            if (ex::entity::index_of(curr->selected->id_node) < ex::entity::index_of(node->selected->id_node)) {
                const auto next = curr->next;
                new_node->root  = root->root;
                new_node->prev  = curr;
                new_node->next  = next;
                curr->next      = new_node;

                if (next != nullptr)
                    next->prev = new_node;

                node = node->next;
                continue;
            }

            if (curr->prev == nullptr) {
                new_node->root = new_node;
                new_node->prev = nullptr;
                new_node->next = curr;
                curr->prev     = new_node;
                root           = new_node;

                node = node->next;
                continue;
            }

            const auto prev = curr->prev;
            new_node->root  = root->root;
            new_node->next  = curr;
            new_node->prev  = prev;
            curr->prev      = new_node;

            node = node->next;
        }

        return root;
    }

    void copy_selected(selection_node *selection) {

        auto head_h = selection->root;
        while (head_h != nullptr) {
            if (head_h->selected->id_parent != selection->root->selected->id_parent)
                return;
            head_h = head_h->next;
        }

        const auto copy = selection_node::copy(selection->root);
        TreeState::reset(tree::state());

        if (copy == nullptr)
            return;

        node_buffer::create(&tree::state().buffer, copy->root, false, false);
        selection_node::free(copy);
    }

    void copy_node(TreeNode *node) {
        if (node->state >>= node_state::link)
            return;
        TreeState::reset(tree::state());
        node_buffer::create(&tree::state().buffer, node, false, false);
    }

    void ref_node(TreeNode *node) {
        if (node->state >>= node_state::link)
            return;
        TreeState::reset(tree::state());
        node_buffer::create(&tree::state().buffer, node, false, true);
    }

    void add_node(TreeNode *add) {
        State &state = ex::logic::globals::state();

        TreeState::reset(tree::state());
        Snapshot *snapshot = Snapshot::create(state);

        TreeNode *created  = ex::entity::create();
        created->id_type   = add->id_type;
        created->id_link   = add->id_link;
        created->state     = add->state;
        created->name      = add->name;

        TreeNode copy           = *created;
        state.logic.active_node = created->id_node;
        ex::entity::add_child(state.logic.root_node, created->id_node);

        undo_history().push(strings::gui_tree_ops_history_add,
        new std::function([snapshot] (State *s) {
            TreeState::reset(tree::state());
            snapshot->rollback(s);
        }),
        new std::function([copy] (State *s) {
            TreeState::reset(tree::state());
            TreeNode *new_node   = ex::entity::create(copy.id_node);
            s->logic.active_node = new_node->id_node;
            *new_node            = copy;
            ex::entity::add_child(s->logic.root_node, new_node->id_node);
        }),
        new std::function([snapshot] {
            Snapshot::free(snapshot);
        }));
    }

    void paste_node() {
//        const auto &tree_state = tree::state();
//
//        if (tree_state.buffer == nullptr) {
//            TreeState::reset(tree::state());
//            return;
//        }
//
//        const auto selection = bubble_sort_selection(tree_state.buffer->selection->root);
//        const bool move      = tree_state.buffer->move;
//        const bool ref       = tree_state.buffer->ref;
//
//        State &state = ex::logic::globals::state();
//
//        TreeState::reset(tree::state());
//        const auto snapshot = Snapshot::create(state);
//
//        selection_node *pasted = nullptr;
//
//        auto head = selection;
//        while (head != nullptr) {
//            TreeNode *deep_copy = ref ? TreeNode::link(head->selected) : TreeNode::deep_copy(head->selected, !move);
//            const auto copy = TreeNode::shallow_copy(head->selected);
//
//            if (!move && !ref) {
//                deep_copy->name = (new std::string(deep_copy->name))->append(" (copy)").c_str();
//            }
//
//            head->selected->state -= node_state::moved;
//            head->selected->state |= (node_state::moved && move);
//
//            head->selected = copy;
//            head           = head->next;
//
//            if (pasted == nullptr) {
//                pasted = selection_node::from_node(TreeNode::deep_copy(deep_copy));
//            } else {
//                pasted->next = selection_node::from_node(TreeNode::deep_copy(deep_copy));
//                pasted->next->prev = pasted;
//                pasted->next->root = pasted->root;
//                pasted             = pasted->next;
//            }
//
//            deep_copy->state |= node_state::created;
//            state.logic.work_tree->add_node(deep_copy);
//        }
//
//        undo_history().push(strings::gui_tree_ops_history_paste,
//        new std::function([snapshot] (ex::State *s) {
//            TreeState::reset(tree::state());
//            snapshot->rollback(s);
//        }),
//        new std::function([move, selection, pasted] (ex::State *s) {
//            TreeState::reset(tree::state());
//
//            auto root = selection;
//            while (root != nullptr) {
//                const auto f_node = s->logic.work_tree->find_node(root->selected->id_local);
//                if (f_node == nullptr)
//                    abort__("Cannot find original node");
//                f_node->state -= node_state::moved;
//                f_node->state |= (node_state::moved && move);
//                root = root->next;
//            }
//
//            root = pasted->root;
//            while (root != nullptr) {
//                const auto dcp = TreeNode::deep_copy(root->selected);
//                dcp->state |= node_state::created;
//                s->logic.work_tree->add_node(dcp);
//                root = root->next;
//            }
//        }),
//        new std::function([snapshot, selection, pasted] {
//            Snapshot::free(snapshot);
//            selection_node::free(selection);
//            selection_node::free(pasted);
//        }));
    }

    void add_into_node(TreeNode *node, TreeNode *add) {
        State &state = ex::logic::globals::state();

        TreeState::reset(tree::state());
        const auto snapshot = Snapshot::create(state);

        TreeNode *created  = ex::entity::create();
        created->id_type   = add->id_type;
        created->id_link   = add->id_link;
        created->state     = add->state;
        created->name      = add->name;

        TreeNode copy           = *created;
        state.logic.active_node = created->id_node;

        entity::IdArray children_1 = ex::entity::children(node->id_node);
        if (children_1.data == nullptr || children_1.size <= 0) {
            TreeNode *group = ex::entity::create();
            group->state |= node_state::group;
            ex::entity::add_child(node->id_node, group->id_node);
        }

        entity::IdArray  children_2 = ex::entity::children(node->id_node);
        TreeNode        *parent     = ex::entity::find(children_2.data[0]);
        const id_type_32 group_id   = parent->id_node;
        const id_type_32 parent_id  = node->id_node;
        ex::entity::add_child(group_id, created->id_node);

        undo_history().push(strings::gui_tree_ops_history_add_into,
        new std::function([snapshot] (ex::State *s) {
            TreeState::reset(tree::state());
            snapshot->rollback(s);
        }),
        new std::function([parent_id, group_id, copy] (ex::State *s) {
            TreeState::reset(tree::state());
            const TreeNode *p_node = ex::entity::find(parent_id);
            if (p_node == nullptr)
                abort__("Cannot find original node");

            entity::IdArray children_3 = ex::entity::children(parent_id);
            if (children_3.data == nullptr || children_3.size <= 0) {
                TreeNode *group = ex::entity::create(group_id);
                group->state |= node_state::group;
                ex::entity::add_child(parent_id, group->id_node);
            }

            entity::IdArray children_4 = ex::entity::children(parent_id);
            TreeNode       *parent_dst = ex::entity::find(children_4.data[0]);
            TreeNode       *new_node   = ex::entity::create(copy.id_node);
            s->logic.active_node       = new_node->id_node;
            *new_node                  = copy;

            ex::entity::add_child(parent_dst->id_node, new_node->id_node);
        }),
        new std::function([snapshot] {
            Snapshot::free(snapshot);
        }));
    }

    void add_after_node(TreeNode *node, TreeNode *add) {
        State &state = ex::logic::globals::state();

        TreeState::reset(tree::state());
        const auto snapshot = Snapshot::create(state);

        TreeNode *created  = ex::entity::create();
        created->id_type   = add->id_type;
        created->id_link   = add->id_link;
        created->state     = add->state;
        created->name      = add->name;

        TreeNode copy           = *created;
        state.logic.active_node = created->id_node;

        const int        index     = ex::entity::index_of(node->id_node);
        const id_type_32 parent_id = node->id_parent;

        ex::entity::add_child(parent_id, created->id_node, index + 1);

        undo_history().push(strings::gui_tree_ops_history_add_after,
        new std::function([snapshot] (ex::State *s) {
            TreeState::reset(tree::state());
            snapshot->rollback(s);
        }),
        new std::function([parent_id, index, copy] (ex::State *s) {
            TreeState::reset(tree::state());

            TreeNode       *new_node = ex::entity::create(copy.id_node);
            s->logic.active_node     = new_node->id_node;
            *new_node                = copy;

            ex::entity::add_child(parent_id, new_node->id_node, index + 1);
        }),
        new std::function([snapshot] {
            Snapshot::free(snapshot);
        }));
    }

    void cut_selected(selection_node *selection) {

        auto head_h = selection->root;
        while (head_h != nullptr) {
            if (head_h->selected->id_parent != selection->root->selected->id_parent)
                return;
            head_h = head_h->next;
        }

        const auto copy = selection_node::copy(selection->root);
        TreeState::reset(tree::state());

        if (copy == nullptr)
            return;

        node_buffer::create(&tree::state().buffer, copy->root, true, false);
        auto head = copy->root;
        while (head != nullptr) {
            head->selected->state |= node_state::dimmed;
            head = head->next;
        }
        selection_node::free(copy);
    }

    void cut_node(TreeNode *node) {
        if (node->state >>= node_state::link)
            return;
        TreeState::reset(tree::state());
        node_buffer::create(&tree::state().buffer, node, true, false);
        node->state |= node_state::dimmed;
    }

    void rename_node(TreeNode *node, const char *name) {
        const TreeNode before = *node;

        const auto id_local = node->id_node;
        const auto og_name = node->name;
        const auto new_name = static_cast<char *>(::malloc(sizeof(char) * 64));
        ::memset(new_name, 0, 64);
        ::memcpy(new_name, name, std::strlen(name));
        node->name = new_name;

        TreeState::reset(tree::state());

        const auto &upd_fn = i_::mappers_[node->id_type].update_tree_node;
        if (upd_fn != nullptr)
            upd_fn(&before, node);

        undo_history().push(strings::gui_tree_ops_history_node_renamed,
        new std::function([id_local, og_name, upd_fn] (State *) {
            TreeState::reset(tree::state());
            const auto f_node = ex::entity::find(id_local);
            if (f_node == nullptr)
                abort__("Cannot find original node");

            const TreeNode before_ = *f_node;
            f_node->name = og_name;

            if (upd_fn != nullptr)
                upd_fn(&before_, f_node);
        }),
        new std::function([id_local, new_name, upd_fn] (State *) {
            TreeState::reset(tree::state());
            const auto f_node = ex::entity::find(id_local);
            if (f_node == nullptr)
                abort__("Cannot find original node");

            const TreeNode before_ = *f_node;
            f_node->name = new_name;

            if (upd_fn != nullptr)
                upd_fn(&before_, f_node);
        }));
    }

    void duplicate_node(TreeNode *node) {
//        if (node->state >>= node_state::link)
//            return;
//        TreeState::reset(tree::state());
//
//        State &state = ex::logic::globals::state();
//
//        const auto snapshot = Snapshot::create(state);
//
//        TreeNode *copy = TreeNode::deep_copy(node, true);
//        copy->name     = (new std::string(node->name))->append(" (copy)").c_str();
//        copy->parent   = nullptr;
//        copy->index    = 0;
//
//        const auto parent_id   = node->parent->id_local;
//        const auto child_index = node->index + 1;
//
//        TreeNode *snap = TreeNode::deep_copy(copy);
//        node->parent->insert_node(child_index, copy);
//
//        state.logic.active_node = copy;
//
//        undo_history().push(strings::gui_tree_ops_history_node_duplicated,
//        new std::function([snapshot] (ex::State *s) -> void {
//            TreeState::reset(tree::state());
//            snapshot->rollback(s);
//        }),
//
//        new std::function([parent_id, child_index, snap] (ex::State *s) -> void {
//            TreeState::reset(tree::state());
//            if (const auto p_node = s->logic.work_tree->find_node(parent_id); p_node != nullptr) {
//                const auto new_copy = snap->deep_copy();
//                new_copy->state |= node_state::created;
//                p_node->insert_node(child_index, new_copy);
//                s->logic.active_node = new_copy;
//            } else
//                abort__("Cannot find original node");
//        }),
//
//        new std::function([snapshot, snap] {
//            Snapshot::free(snapshot);
//            if (snap != nullptr)
//                delete snap;
//        }));
    }

    void paste_after_node(TreeNode *node) {
//        const auto &tree_state = tree::state();
//
//        if (node == nullptr || node->parent == nullptr || tree_state.buffer == nullptr) {
//            TreeState::reset(tree::state());
//            return;
//        }
//
//        if (node->state >>= node_state::link)
//            return;
//
//        const auto parent    = node->parent;
//        const auto parent_id = parent->id_local;
//        const auto index     = node->index;
//
//        const auto selection = bubble_sort_selection(tree_state.buffer->selection->root);
//        const bool move      = tree_state.buffer->move;
//        const bool ref       = tree_state.buffer->ref;
//
//        if (move) {
//            for (auto base = selection; base != nullptr; base = base->next) {
//                if (base->selected == node)
//                    return;
//            }
//        }
//
//        State &state = ex::logic::globals::state();
//
//        TreeState::reset(tree::state());
//        const auto snapshot = Snapshot::create(state);
//
//        selection_node *pasted = nullptr;
//
//        int c = index;
//        auto head = selection;
//        while (head != nullptr) {
//            TreeNode *deep_copy = ref ? TreeNode::link(head->selected) : TreeNode::deep_copy(head->selected, !move);
//            const auto copy = TreeNode::shallow_copy(head->selected);
//
//            if (!move && !ref) {
//                deep_copy->name = (new std::string(deep_copy->name))->append(" (copy)").c_str();
//            }
//
//            head->selected->state -= node_state::moved;
//            head->selected->state |= (node_state::moved && move);
//            head->selected = copy;
//            head           = head->next;
//
//            if (pasted == nullptr) {
//                pasted = selection_node::from_node(TreeNode::deep_copy(deep_copy));
//            } else {
//                pasted->next = selection_node::from_node(TreeNode::deep_copy(deep_copy));
//                pasted->next->prev = pasted;
//                pasted->next->root = pasted->root;
//                pasted             = pasted->next;
//            }
//
//            deep_copy->state |= node_state::created;
//            parent->insert_node(++c, deep_copy);
//        }
//
//        undo_history().push(strings::gui_tree_ops_history_paste_after,
//        new std::function([snapshot] (ex::State *s) {
//            TreeState::reset(tree::state());
//            snapshot->rollback(s);
//        }),
//        new std::function([parent_id, move, index, selection, pasted] (ex::State *s) {
//            TreeState::reset(tree::state());
//            const auto p_node = s->logic.work_tree->find_node(parent_id);
//            if (p_node == nullptr)
//                abort__("Cannot find original node");
//
//            auto root = selection;
//            while (root != nullptr) {
//                const auto f_node = s->logic.work_tree->find_node(root->selected->id_local);
//                if (f_node == nullptr)
//                    abort__("Cannot find original node");
//                f_node->state -= node_state::moved;
//                f_node->state |= (node_state::moved && move);
//                root = root->next;
//            }
//
//            int k = index;
//            root = pasted->root;
//            while (root != nullptr) {
//                const auto dcp = TreeNode::deep_copy(root->selected);
//                dcp->state |= node_state::created;
//                p_node->insert_node(++k, dcp);
//                root = root->next;
//            }
//        }),
//        new std::function([snapshot, selection, pasted] {
//            Snapshot::free(snapshot);
//            selection_node::free(selection);
//            selection_node::free(pasted);
//        }));
    }

    void paste_into_node(TreeNode *node) {
//        const auto &tree_state = tree::state();
//
//        if (node == nullptr || tree_state.buffer == nullptr) {
//            TreeState::reset(tree::state());
//            return;
//        }
//
//        if (node->state >>= node_state::link)
//            return;
//
//        const auto selection = bubble_sort_selection(tree_state.buffer->selection->root);
//        const bool move      = tree_state.buffer->move;
//        const bool ref       = tree_state.buffer->ref;
//
//        if (move) {
//            for (auto base = selection; base != nullptr; base = base->next) {
//                if (base->selected == node)
//                    return;
//            }
//        }
//
//        State &state = ex::logic::globals::state();
//
//        TreeState::reset(tree::state());
//        const auto snapshot = Snapshot::create(state);
//
//        if (node->children == nullptr || node->children_n <= 0) {
//            TreeNode *group = TreeNode::create();
//            group->state |= node_state::is_group;
//            group->state |= node_state::created;
//            node->add_node(group);
//        }
//
//        TreeNode       *parent = node->children[0];
//        selection_node *pasted = nullptr;
//
//        const auto parent_id = node->id_local;
//        const auto group_id  = parent->id_local;
//
//        auto head = selection;
//        while (head != nullptr) {
//            TreeNode *deep_copy = ref ? TreeNode::link(head->selected) : TreeNode::deep_copy(head->selected, !move);
//            const auto copy = TreeNode::shallow_copy(head->selected);
//
//            if (!move && !ref) {
//                deep_copy->name = (new std::string(deep_copy->name))->append(" (copy)").c_str();
//            }
//
//            head->selected->state -= node_state::moved;
//            head->selected->state |= (node_state::moved && move);
//
//            head->selected = copy;
//            head           = head->next;
//
//            if (pasted == nullptr) {
//                pasted = selection_node::from_node(TreeNode::deep_copy(deep_copy));
//            } else {
//                pasted->next = selection_node::from_node(TreeNode::deep_copy(deep_copy));
//                pasted->next->prev = pasted;
//                pasted->next->root = pasted->root;
//                pasted             = pasted->next;
//            }
//
//            deep_copy->state |= node_state::created;
//            parent->add_node(deep_copy);
//        }
//
//        undo_history().push(strings::gui_tree_ops_history_paste_into,
//        new std::function([snapshot] (ex::State *s) {
//            TreeState::reset(tree::state());
//            snapshot->rollback(s);
//        }),
//        new std::function([parent_id, group_id, move, selection, pasted] (ex::State *s) {
//            TreeState::reset(tree::state());
//            const auto p_node = s->logic.work_tree->find_node(parent_id);
//            if (p_node == nullptr)
//                abort__("Cannot find original node");
//
//            auto root = selection;
//            while (root != nullptr) {
//                const auto f_node = s->logic.work_tree->find_node(root->selected->id_local);
//                if (f_node == nullptr)
//                    abort__("Cannot find original node");
//                f_node->state -= node_state::moved;
//                f_node->state |= (node_state::moved && move);
//                root = root->next;
//            }
//
//            if (p_node->children_n <= 0 || p_node->children == nullptr) {
//                TreeNode *group = TreeNode::create();
//                group->id_local = group_id;
//                group->state |= node_state::is_group;
//                group->state |= node_state::created;
//                p_node->add_node(group);
//            }
//
//            TreeNode *group = p_node->children[0];
//            root            = pasted->root;
//            while (root != nullptr) {
//                const auto dcp = TreeNode::deep_copy(root->selected);
//                dcp->state |= node_state::created;
//                group->add_node(dcp);
//                root = root->next;
//            }
//        }),
//        new std::function([snapshot, selection, pasted] {
//            Snapshot::free(snapshot);
//            selection_node::free(selection);
//            selection_node::free(pasted);
//        }));
    }

    void move_after_node(TreeNode *node, TreeNode *move) {
//        if (node == nullptr || move == nullptr || node == move)
//            return;
//
//        State &state = ex::logic::globals::state();
//
//        TreeState::reset(tree::state());
//        const auto snapshot = Snapshot::create(state);
//
//        const auto parent    = node->parent;
//        const auto index     = node->index;
//        const auto parent_id = parent->id_local;
//
//        TreeNode *copy = TreeNode::deep_copy(move);
//        copy->state |= node_state::created;
//        move->state |= node_state::moved;
//
//        TreeNode *nco = TreeNode::deep_copy(copy);
//        parent->insert_node(index + 1, nco);
//        state.logic.active_node = nco;
//
//        undo_history().push(strings::gui_tree_ops_history_move_after,
//        new std::function([snapshot] (ex::State *s) {
//            TreeState::reset(tree::state());
//            snapshot->rollback(s);
//        }),
//        new std::function([parent_id, index, copy] (ex::State *s) {
//            TreeState::reset(tree::state());
//            const auto p_node = s->logic.work_tree->find_node(parent_id);
//            if (p_node == nullptr)
//                abort__("Cannot find original node");
//
//            const auto f_node = s->logic.work_tree->find_node(copy->id_local);
//            if (f_node == nullptr)
//                abort__("Cannot find original node");
//            f_node->state |= node_state::moved;
//
//            TreeNode *new_copy = TreeNode::deep_copy(copy);
//            new_copy->state |= node_state::created;
//
//            p_node->insert_node(index + 1, new_copy);
//            s->logic.active_node = new_copy;
//        }),
//        new std::function([snapshot, copy] {
//            Snapshot::free(snapshot);
//            if (copy != nullptr)
//                delete copy;
//        }));
    }

    void move_into_node(TreeNode *node, TreeNode *move) {
//        if (node == nullptr || move == nullptr || move == node)
//            return;
//
//        State &state = ex::logic::globals::state();
//
//        TreeState::reset(tree::state());
//        const auto snapshot = Snapshot::create(state);
//
//        if (node->children == nullptr || node->children_n <= 0) {
//            TreeNode *group = TreeNode::create();
//            group->state |= node_state::is_group;
//            group->state |= node_state::created;
//            node->add_node(group);
//        }
//
//        TreeNode  *parent    = node->children[0];
//        const auto parent_id = node->id_local;
//        const auto group_id  = parent->id_local;
//
//        TreeNode *copy = TreeNode::deep_copy(move);
//        copy->state |= node_state::created;
//        move->state |= node_state::moved;
//
//        TreeNode *nco = TreeNode::deep_copy(copy);
//        parent->add_node(nco);
//
//        state.logic.active_node = nco;
//
//        undo_history().push(strings::gui_tree_ops_history_move_into,
//        new std::function([snapshot] (ex::State *s) {
//            TreeState::reset(tree::state());
//            snapshot->rollback(s);
//        }),
//        new std::function([parent_id, group_id, copy] (ex::State *s) {
//            TreeState::reset(tree::state());
//            const auto p_node = s->logic.work_tree->find_node(parent_id);
//            if (p_node == nullptr)
//                abort__("Cannot find original node");
//
//            const auto f_node = s->logic.work_tree->find_node(copy->id_local);
//            if (f_node == nullptr)
//                abort__("Cannot find original node");
//            f_node->state |= node_state::moved;
//
//            if (p_node->children_n <= 0 || p_node->children == nullptr) {
//                TreeNode *group = TreeNode::create();
//                group->id_local = group_id;
//                group->state |= node_state::is_group;
//                group->state |= node_state::created;
//                p_node->add_node(group);
//            }
//
//            TreeNode *group    = p_node->children[0];
//            TreeNode *new_copy = TreeNode::deep_copy(copy);
//            new_copy->state |= node_state::created;
//            group->add_node(new_copy);
//            s->logic.active_node = new_copy;
//        }),
//        new std::function([snapshot, copy] {
//            Snapshot::free(snapshot);
//            if (copy != nullptr)
//                delete copy;
//        }));
    }

    void group_selected(selection_node *selection) {
//        const auto sorted    = bubble_sort_selection(selection->root);
//        const auto parent    = selection->root->selected->parent;
//        const auto parent_id = parent->id_local;
//
//        const auto &state = ex::logic::globals::state();
//
//        TreeState::reset(tree::state());
//        const auto snapshot = state.logic.work_tree->deep_copy();
//
//        TreeNode *new_group = TreeNode::create();
//        new_group->name     = "generic_group";
//        new_group->state |= node_state::is_group;
//        new_group->state |= node_state::created;
//
//        int             index = -1;
//        selection_node *head  = sorted;
//        while (head != nullptr) {
//            if (index == -1)
//                index = head->selected->index;
//            TreeNode *new_one = head->selected->deep_copy();
//            new_one->state |= node_state::created;
//
//            new_group->add_node(new_one);
//            head->selected->state |= node_state::moved;
//            head->selected        = TreeNode::shallow_copy(head->selected);
//            head                  = head->next;
//        }
//
//        if (index >= 0)
//            parent->insert_node(index, new_group->deep_copy());
//
//        undo_history().push(strings::gui_tree_ops_history_group,
//        new std::function([snapshot] (ex::State *s) {
//            TreeState::reset(tree::state());
//            if (s->logic.work_tree != nullptr && s->logic.work_tree != snapshot)
//                delete s->logic.work_tree;
//            s->logic.work_tree            = snapshot->deep_copy();
//            s->logic.work_tree->state -= node_state::connected;
//            s->logic.work_tree->state |= node_state::created;
//        }),
//
//        new std::function([new_group, parent_id, index, sorted] (ex::State *s) {
//            TreeState::reset(tree::state());
//            if (index < 0)
//                abort__("Cannot find original node, index < 0");
//
//            auto node = sorted;
//            while (node != nullptr) {
//                const auto child = s->logic.work_tree->find_node(node->selected->id_local);
//                if (child == nullptr)
//                    abort__("Cannot find original child node");
//                child->state |= node_state::moved;
//                node         = node->next;
//            }
//
//            const auto p_node = s->logic.work_tree->find_node(parent_id);
//            if (p_node == nullptr)
//                abort__("Cannot find original parent node");
//
//            p_node->insert_node(index, new_group->deep_copy());
//        }),
//
//        new std::function([new_group, snapshot, sorted] {
//            if (new_group != nullptr)
//                delete new_group;
//            if (snapshot != nullptr)
//                delete snapshot;
//            selection_node::free(sorted);
//        }));
    }

    void ungroup_selected(selection_node *selection) {
//        const auto sorted = bubble_sort_selection(selection->root);
//        const auto parent = selection->root->selected->parent;
//
//        const std::size_t parent_id = parent->parent->id_local;
//        const std::size_t index     = parent->index;
//
//        const auto &state = ex::logic::globals::state();
//
//        TreeState::reset(tree::state());
//        const auto snapshot = state.logic.work_tree->deep_copy();
//
//        TreeNode       *temp_group = TreeNode::create();
//        selection_node *head       = sorted;
//
//        while (head != nullptr) {
//            TreeNode *new_one = head->selected->deep_copy();
//            new_one->state |= node_state::created;
//
//            temp_group->add_node(new_one);
//            head->selected->state |= node_state::moved;
//            head->selected        = TreeNode::shallow_copy(head->selected);
//            head                  = head->next;
//        }
//
//        for (int c = 0, i = index; c < temp_group->children_n; c++) {
//            parent->parent->insert_node(++i, temp_group->children[c]->deep_copy());
//        }
//
//        undo_history().push(strings::gui_tree_ops_history_ungroup,
//        new std::function([snapshot] (ex::State *s) {
//            TreeState::reset(tree::state());
//            if (s->logic.work_tree != nullptr && s->logic.work_tree != snapshot)
//                delete s->logic.work_tree;
//            s->logic.work_tree            = snapshot->deep_copy();
//            s->logic.work_tree->state -= node_state::connected;
//            s->logic.work_tree->state |= node_state::created;
//        }),
//
//        new std::function([index, parent_id, sorted, temp_group] (ex::State *s) {
//            TreeState::reset(tree::state());
//            if (index < 0)
//                abort__("Cannot find original node, index < 0");
//
//            auto node = sorted;
//            while (node != nullptr) {
//                const auto child = s->logic.work_tree->find_node(node->selected->id_local);
//                if (child == nullptr)
//                    abort__("Cannot find original child node");
//                child->state |= node_state::moved;
//                node         = node->next;
//            }
//
//            const auto p_node = s->logic.work_tree->find_node(parent_id);
//            if (p_node == nullptr)
//                abort__("Cannot find original parent node");
//
//            for (int c = 0, i = index; c < temp_group->children_n; c++) {
//                const auto child = temp_group->children[c]->deep_copy();
//                child->state |= node_state::created;
//                p_node->insert_node(++i, child);
//            }
//
//        }),
//
//        new std::function([snapshot, temp_group, sorted] {
//            if (temp_group != nullptr)
//                delete temp_group;
//            if (snapshot != nullptr)
//                delete snapshot;
//            selection_node::free(sorted);
//        }));
    }

    void enable_node(TreeNode *node) {
        const TreeNode before = *node;

        const auto og_state = node->state;
        node->state -= node_state::disabled;

        const auto &upd_fn = i_::mappers_[node->id_type].update_tree_node;
        if (upd_fn != nullptr)
            upd_fn(&before, node);

        TreeState::reset(tree::state());
        ex::entity::set_dirty(true);

        undo_history().push(strings::gui_tree_ops_history_node_enabled,
        new std::function([id_l = node->id_node, og_state, upd_fn] (ex::State *) {
            const auto f_node = ex::entity::find(id_l);
            if (f_node == nullptr)
                abort__("Cannot find original node");

            const TreeNode before_ = *f_node;

            f_node->state -= node_state::disabled;
            f_node->state |= (og_state & node_state::disabled);

            if (upd_fn != nullptr)
                upd_fn(&before_, f_node);

            TreeState::reset(tree::state());
            ex::entity::set_dirty(true);
        }),
        new std::function([id_l = node->id_node, upd_fn] (ex::State *) {
            const auto f_node = ex::entity::find(id_l);
            if (f_node == nullptr)
                abort__("Cannot find original node");

            const TreeNode before_ = *f_node;

            f_node->state -= node_state::disabled;

            if (upd_fn != nullptr)
                upd_fn(&before_, f_node);

            TreeState::reset(tree::state());
            ex::entity::set_dirty(true);
        }));
    }

    void disable_node(TreeNode *node) {
        const TreeNode before = *node;

        const auto og_state = node->state;
        node->state |= node_state::disabled;

        if (const auto &upd_fn = i_::mappers_[node->id_type].update_tree_node)
            upd_fn(&before, node);

        TreeState::reset(tree::state());
        ex::entity::set_dirty(true);

        undo_history().push(strings::gui_tree_ops_history_node_disabled,
        new std::function([id_l = node->id_node, og_state] (ex::State *) {
            const auto f_node = ex::entity::find(id_l);
            if (f_node == nullptr)
                abort__("Cannot find original node");

            const TreeNode before_ = *f_node;

            f_node->state -= node_state::disabled;
            f_node->state |= (og_state & node_state::disabled);

            if (const auto &upd_fn = i_::mappers_[f_node->id_type].update_tree_node)
                upd_fn(&before_, f_node);

            TreeState::reset(tree::state());
            ex::entity::set_dirty(true);
        }),
        new std::function([id_l = node->id_node] (ex::State *) {

            const auto f_node = ex::entity::find(id_l);
            if (f_node == nullptr)
                abort__("Cannot find original node");

            const TreeNode before_ = *f_node;

            f_node->state |= node_state::disabled;

            if (const auto &upd_fn = i_::mappers_[f_node->id_type].update_tree_node)
                upd_fn(&before_, f_node);

            TreeState::reset(tree::state());
            ex::entity::set_dirty(true);
        }));
    }

    void enable_selected(selection_node *selection) {
        if (selection == nullptr)
            return;

        ex::data::array_map<std::size_t, node_state> og_map;
        auto head = selection->root;
        while (head != nullptr) {
            const TreeNode before_ = *head->selected;

            og_map.put(head->selected->id_node, head->selected->state);
            head->selected->state -= node_state::disabled;

            if (const auto &upd_fn = i_::mappers_[head->selected->id_type].update_tree_node)
                upd_fn(&before_, head->selected);
            head = head->next;
        }

        TreeState::reset(tree::state());
        ex::entity::set_dirty(true);

        undo_history().push(strings::gui_tree_ops_history_select_enabled,
        new std::function([og_state = og_map](ex::State *) {
            for (int j = 0; j < og_state.size; j++) {
                const auto &k = og_state.key_entries[j];
                const auto &v = og_state.val_entries[j];

                const auto f_node = ex::entity::find(k);
                if (f_node == nullptr)
                    abort__("Cannot find original node");

                const TreeNode before_ = *f_node;

                f_node->state -= node_state::disabled;
                f_node->state |= (v & node_state::disabled);

                if (const auto &upd_fn = i_::mappers_[f_node->id_type].update_tree_node)
                    upd_fn(&before_, f_node);
            }
            ex::entity::set_dirty(true);
            TreeState::reset(tree::state());
        }),
        new std::function([og_state = og_map](ex::State *) {
            for (int ik = 0; ik < og_state.size; ik++) {
                const auto &k = og_state.key_entries[ik];
                const auto f_node = ex::entity::find(k);
                if (f_node == nullptr)
                    abort__("Cannot find original node");

                const TreeNode before_ = *f_node;

                f_node->state -= node_state::disabled;

                if (const auto &upd_fn = i_::mappers_[f_node->id_type].update_tree_node)
                    upd_fn(&before_, f_node);
            }
            ex::entity::set_dirty(true);
            TreeState::reset(tree::state());
        }));
    }

    void disable_selected(selection_node *selection) {
        if (selection == nullptr)
            return;

        ex::data::array_map<std::size_t, node_state> og_map;
        auto head = selection->root;
        while (head != nullptr) {
            const TreeNode before_ = *head->selected;

            og_map.put(head->selected->id_node, head->selected->state);
            head->selected->state |= node_state::disabled;

            if (const auto &upd_fn = i_::mappers_[head->selected->id_type].update_tree_node)
                upd_fn(&before_, head->selected);

            head = head->next;
        }

        TreeState::reset(tree::state());
        ex::entity::set_dirty(true);

        undo_history().push(strings::gui_tree_ops_history_select_disabled,
        new std::function([og_state = og_map](ex::State *) {
            for (int j = 0; j < og_state.size; j++) {
                const auto &k = og_state.key_entries[j];
                const auto &v = og_state.val_entries[j];

                const auto f_node = ex::entity::find(k);
                if (f_node == nullptr)
                    abort__("Cannot find original node");

                const TreeNode before_ = *f_node;

                f_node->state -= node_state::disabled;
                f_node->state |= (v & node_state::disabled);

                if (const auto &upd_fn = i_::mappers_[f_node->id_type].update_tree_node)
                    upd_fn(&before_, f_node);
            }
            ex::entity::set_dirty(true);
            TreeState::reset(tree::state());
        }),
        new std::function([og_state = og_map](ex::State *) {
            for (int ik = 0; ik < og_state.size; ik++) {
                const auto &k = og_state.key_entries[ik];
                const auto f_node = ex::entity::find(k);
                if (f_node == nullptr)
                    abort__("Cannot find original node");

                const TreeNode before_ = *f_node;

                f_node->state |= node_state::disabled;

                if (const auto &upd_fn = i_::mappers_[f_node->id_type].update_tree_node)
                    upd_fn(&before_, f_node);
            }
            ex::entity::set_dirty(true);
            TreeState::reset(tree::state());
        }));
    }

    void delete_selected(selection_node *selection) {
        if (selection == nullptr)
            return;

        const auto copy_selection = selection_node::copy(selection->root);

        TreeState::reset(tree::state());
        ex::data::array<id_type_32> selections;

        auto head = copy_selection->root;
        while (head != nullptr) {
            head->selected->state -= node_state::selected;
            if (!(head->selected->state >>= node_state::group))
                selections.push(head->selected->id_node);
            head = head->next;
        }

        const auto snapshot = Snapshot::create();
        selection_node::free(copy_selection);

        for (int i = 0; i < selections.size; ++i)
            ex::entity::remove(selections[i]);

        undo_history().push(strings::gui_tree_ops_history_select_deleted,
        new std::function([snapshot] (ex::State *) {
            TreeState::reset(tree::state());
            snapshot->rollback();
        }),
        new std::function([selections] (ex::State *) {
            TreeState::reset(tree::state());
            for (int i = 0; i < selections.size; ++i)
                ex::entity::remove(selections[i]);
        }),
        new std::function([snapshot] {
            Snapshot::free(snapshot);
        }));
    }

    void delete_node(TreeNode *node) {
        if (node == nullptr)
            return;

        TreeState::reset(tree::state());

        node->state -= node_state::selected;
        const auto node_id  = node->id_node;
        const auto snapshot = Snapshot::create();

        ex::entity::remove(node_id);

        undo_history().push(strings::gui_tree_ops_history_node_deleted,
        new std::function([snapshot] (ex::State *s) {
            TreeState::reset(tree::state());
            snapshot->rollback(s);
        }),
        new std::function([node_id] (ex::State *) {
            TreeState::reset(tree::state());
            ex::entity::remove(node_id);
        }),
        new std::function([snapshot] {
            Snapshot::free(snapshot);
        }));
    }

}