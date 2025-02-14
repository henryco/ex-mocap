//
// Created by xd on 31/01/25.
//

#include "../../../headers/imgui/tree/gui_tree_ops.h"
#include "../../../headers/imgui/gui_globals.h"
#include "../../../headers/imgui/gui_strings.h"
#include "../../../headers/logic/logic_globals.h"
#include <ranges>
#include <map>

namespace ex::gui::tree {

    struct Snapshot {
        TreeNode *work_tree   = nullptr;
        TreeNode *active_node = nullptr;

        void rollback(State *s) const {
            if (s->work_tree != nullptr && s->work_tree != this->work_tree)
                delete s->work_tree;
            s->work_tree            = this->work_tree->deep_copy();
            s->work_tree->state -= node_state::connected;
            s->work_tree->state |= node_state::created;

            if (active_node == nullptr)
                s->active_node = nullptr;
            else
                s->active_node = s->work_tree->find_node(active_node->id_local);
        }

        static Snapshot *create(State &state) {
            const auto snapshot = state.work_tree->deep_copy();
            const auto active_copy = TreeNode::shallow_copy(state.active_node);
            return new Snapshot(snapshot, active_copy);
        }

        static void free(Snapshot *snapshot) {
            if (snapshot == nullptr)
                return;
            if (snapshot->work_tree != nullptr)
                delete snapshot->work_tree;
            if (snapshot->active_node != nullptr)
                delete snapshot->active_node;
            delete snapshot;
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

            while (node->selected->index > curr->selected->index && curr->next != nullptr) {
                if (curr->next->selected->index > node->selected->index)
                    break;
                curr = curr->next;
            }

            const auto new_node = new selection_node;
            new_node->selected  = node->selected;

            if (curr->selected->index < node->selected->index) {
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
            if (head_h->selected->parent != selection->root->selected->parent)
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
        if (node->state >>= node_state::is_link)
            return;
        TreeState::reset(tree::state());
        node_buffer::create(&tree::state().buffer, node, false, false);
    }

    void ref_node(TreeNode *node) {
        if (node->state >>= node_state::is_link)
            return;
        TreeState::reset(tree::state());
        node_buffer::create(&tree::state().buffer, node, false, true);
    }

    void add_node(TreeNode *add) {
        State &state = ex::logic::globals::state();

        TreeState::reset(tree::state());
        const auto snapshot = Snapshot::create(state);

        TreeNode *copy = TreeNode::deep_copy(add);
        copy->state |= node_state::created;
        add->state |= node_state::created;

        state.work_tree->add_node(add);
        state.active_node = add;

        undo_history().push(strings::gui_tree_ops_history_add,
        new std::function([snapshot](State *s) {
            TreeState::reset(tree::state());
            snapshot->rollback(s);
        }),
        new std::function([copy](State *s) {
            TreeState::reset(tree::state());
            TreeNode *new_copy = TreeNode::deep_copy(copy);
            new_copy->state |= node_state::created;
            s->work_tree->add_node(new_copy);
            s->active_node = new_copy;
        }),
        new std::function([snapshot, copy] {
            Snapshot::free(snapshot);
            if (copy != nullptr)
                delete copy;
        }));
    }

    void add_into_node(TreeNode *node, TreeNode *add) {
        State &state = ex::logic::globals::state();

        TreeState::reset(tree::state());
        const auto snapshot = Snapshot::create(state);

        TreeNode *copy = TreeNode::deep_copy(add);
        copy->state |= node_state::created;
        add->state |= node_state::created;

        if (node->children == nullptr || node->children_n <= 0) {
            TreeNode *group = TreeNode::create();
            group->state |= node_state::is_group;
            group->state |= node_state::created;
            node->add_node(group);
        }

        TreeNode  *parent    = node->children[0];
        const auto parent_id = node->id_local;
        const auto group_id  = parent->id_local;

        parent->add_node(add);
        state.active_node = add;

        undo_history().push(strings::gui_tree_ops_history_add_into,
        new std::function([snapshot] (ex::State *s) {
            TreeState::reset(tree::state());
            snapshot->rollback(s);
        }),
        new std::function([parent_id, group_id, copy] (ex::State *s) {
            TreeState::reset(tree::state());
            const auto p_node = s->work_tree->find_node(parent_id);
            if (p_node == nullptr)
                throw std::runtime_error("Cannot find original node");

            if (p_node->children_n <= 0 || p_node->children == nullptr) {
                TreeNode *group = TreeNode::create();
                group->id_local = group_id;
                group->state |= node_state::is_group;
                group->state |= node_state::created;
                p_node->add_node(group);
            }

            TreeNode *group    = p_node->children[0];
            TreeNode *new_copy = TreeNode::deep_copy(copy);
            new_copy->state |= node_state::created;
            group->add_node(new_copy);
            s->active_node = new_copy;
        }),
        new std::function([snapshot, copy] {
            Snapshot::free(snapshot);
            if (copy != nullptr)
                delete copy;
        }));
    }

    void add_after_node(TreeNode *node, TreeNode *add) {
        State &state = ex::logic::globals::state();

        TreeState::reset(tree::state());
        const auto snapshot = Snapshot::create(state);

        TreeNode *copy = TreeNode::deep_copy(add);
        copy->state |= node_state::created;
        add->state |= node_state::created;

        const auto parent    = node->parent;
        const auto index     = node->index;
        const auto parent_id = parent->id_local;

        parent->insert_node(index + 1, add);
        state.active_node = add;

        undo_history().push(strings::gui_tree_ops_history_add_after,
        new std::function([snapshot] (ex::State *s) {
            TreeState::reset(tree::state());
            snapshot->rollback(s);
        }),
        new std::function([parent_id, index, copy] (ex::State *s) {
            TreeState::reset(tree::state());
            const auto p_node = s->work_tree->find_node(parent_id);
            if (p_node == nullptr)
                throw std::runtime_error("Cannot find original node");

            TreeNode *new_copy = TreeNode::deep_copy(copy);
            new_copy->state |= node_state::created;

            p_node->insert_node(index + 1, new_copy);
            s->active_node = new_copy;
        }),
        new std::function([snapshot, copy] {
            Snapshot::free(snapshot);
            if (copy != nullptr)
                delete copy;
        }));
    }

    void cut_selected(selection_node *selection) {

        auto head_h = selection->root;
        while (head_h != nullptr) {
            if (head_h->selected->parent != selection->root->selected->parent)
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
        if (node->state >>= node_state::is_link)
            return;
        TreeState::reset(tree::state());
        node_buffer::create(&tree::state().buffer, node, true, false);
        node->state |= node_state::dimmed;
    }

    void rename_node(TreeNode *node, const char *name) {
        const auto id_local = node->id_local;
        const auto og_name = node->name;
        const auto new_name = new char[64]{ "" };

        std::memcpy(new_name, name, std::strlen(name));
        node->name = new_name;

        TreeState::reset(tree::state());

        undo_history().push(strings::gui_tree_ops_history_node_renamed,
        new std::function([id_local, og_name] (State *s) {
            TreeState::reset(tree::state());
            const auto f_node = s->work_tree->find_node(id_local);
            if (f_node == nullptr)
                throw std::runtime_error("Cannot find original node");
            f_node->name = og_name;
        }),
        new std::function([id_local, new_name] (State *s) {
            TreeState::reset(tree::state());
            const auto f_node = s->work_tree->find_node(id_local);
            if (f_node == nullptr)
                throw std::runtime_error("Cannot find original node");
            f_node->name = new_name;
        }));
    }

    void duplicate_node(TreeNode *node) {
        if (node->state >>= node_state::is_link)
            return;
        TreeState::reset(tree::state());

        State &state = ex::logic::globals::state();

        const auto snapshot = Snapshot::create(state);

        TreeNode *copy = TreeNode::deep_copy(node, true);
        copy->name     = (new std::string(node->name))->append(" (copy)").c_str();
        copy->parent   = nullptr;
        copy->index    = 0;

        const auto parent_id   = node->parent->id_local;
        const auto child_index = node->index + 1;

        TreeNode *snap = TreeNode::deep_copy(copy);
        node->parent->insert_node(child_index, copy);

        state.active_node = copy;

        undo_history().push(strings::gui_tree_ops_history_node_duplicated,
        new std::function([snapshot] (ex::State *s) -> void {
            TreeState::reset(tree::state());
            snapshot->rollback(s);
        }),

        new std::function([parent_id, child_index, snap] (ex::State *s) -> void {
            TreeState::reset(tree::state());
            if (const auto p_node = s->work_tree->find_node(parent_id); p_node != nullptr) {
                const auto new_copy = snap->deep_copy();
                new_copy->state |= node_state::created;
                p_node->insert_node(child_index, new_copy);
                s->active_node = new_copy;
            } else
                throw std::runtime_error("Cannot find original node");
        }),

        new std::function([snapshot, snap] {
            Snapshot::free(snapshot);
            if (snap != nullptr)
                delete snap;
        }));
    }

    void paste_after_node(TreeNode *node) {
        const auto &tree_state = tree::state();

        if (node == nullptr || node->parent == nullptr || tree_state.buffer == nullptr) {
            TreeState::reset(tree::state());
            return;
        }

        if (node->state >>= node_state::is_link)
            return;

        const auto parent    = node->parent;
        const auto parent_id = parent->id_local;
        const auto index     = node->index;

        const auto selection = bubble_sort_selection(tree_state.buffer->selection->root);
        const bool move      = tree_state.buffer->move;
        const bool ref       = tree_state.buffer->ref;

        State &state = ex::logic::globals::state();

        TreeState::reset(tree::state());
        const auto snapshot = Snapshot::create(state);

        selection_node *pasted = nullptr;

        int c = index;
        auto head = selection;
        while (head != nullptr) {
            TreeNode *deep_copy = ref ? TreeNode::link(head->selected) : TreeNode::deep_copy(head->selected, !move);
            const auto copy = TreeNode::shallow_copy(head->selected);

            if (!move && !ref) {
                deep_copy->name = (new std::string(deep_copy->name))->append(" (copy)").c_str();
            }

            head->selected->state -= node_state::moved;
            head->selected->state |= (node_state::moved && move);
            head->selected = copy;
            head           = head->next;

            if (pasted == nullptr) {
                pasted = selection_node::from_node(TreeNode::deep_copy(deep_copy));
            } else {
                pasted->next = selection_node::from_node(TreeNode::deep_copy(deep_copy));
                pasted->next->prev = pasted;
                pasted->next->root = pasted->root;
                pasted             = pasted->next;
            }

            deep_copy->state |= node_state::created;
            parent->insert_node(++c, deep_copy);
        }

        undo_history().push(strings::gui_tree_ops_history_paste_after,
        new std::function([snapshot] (ex::State *s) {
            TreeState::reset(tree::state());
            snapshot->rollback(s);
        }),
        new std::function([parent_id, move, index, selection, pasted] (ex::State *s) {
            TreeState::reset(tree::state());
            const auto p_node = s->work_tree->find_node(parent_id);
            if (p_node == nullptr)
                throw std::runtime_error("Cannot find original node");

            auto root = selection;
            while (root != nullptr) {
                const auto f_node = s->work_tree->find_node(root->selected->id_local);
                if (f_node == nullptr)
                    throw std::runtime_error("Cannot find original node");
                f_node->state -= node_state::moved;
                f_node->state |= (node_state::moved && move);
                root = root->next;
            }

            int k = index;
            root = pasted->root;
            while (root != nullptr) {
                const auto dcp = TreeNode::deep_copy(root->selected);
                dcp->state |= node_state::created;
                p_node->insert_node(++k, dcp);
                root = root->next;
            }
        }),
        new std::function([snapshot, selection, pasted] {
            Snapshot::free(snapshot);
            selection_node::free(selection);
            selection_node::free(pasted);
        }));
    }

    void paste_into_node(TreeNode *node) {
        const auto &tree_state = tree::state();

        if (node == nullptr || tree_state.buffer == nullptr) {
            TreeState::reset(tree::state());
            return;
        }

        if (node->state >>= node_state::is_link)
            return;

        const auto selection = bubble_sort_selection(tree_state.buffer->selection->root);
        const bool move      = tree_state.buffer->move;
        const bool ref       = tree_state.buffer->ref;

        State &state = ex::logic::globals::state();

        TreeState::reset(tree::state());
        const auto snapshot = Snapshot::create(state);

        if (node->children == nullptr || node->children_n <= 0) {
            TreeNode *group = TreeNode::create();
            group->state |= node_state::is_group;
            group->state |= node_state::created;
            node->add_node(group);
        }

        TreeNode       *parent = node->children[0];
        selection_node *pasted = nullptr;

        const auto parent_id = node->id_local;
        const auto group_id  = parent->id_local;

        auto head = selection;
        while (head != nullptr) {
            TreeNode *deep_copy = ref ? TreeNode::link(head->selected) : TreeNode::deep_copy(head->selected, !move);
            const auto copy = TreeNode::shallow_copy(head->selected);

            if (!move && !ref) {
                deep_copy->name = (new std::string(deep_copy->name))->append(" (copy)").c_str();
            }

            head->selected->state -= node_state::moved;
            head->selected->state |= (node_state::moved && move);

            head->selected = copy;
            head           = head->next;

            if (pasted == nullptr) {
                pasted = selection_node::from_node(TreeNode::deep_copy(deep_copy));
            } else {
                pasted->next = selection_node::from_node(TreeNode::deep_copy(deep_copy));
                pasted->next->prev = pasted;
                pasted->next->root = pasted->root;
                pasted             = pasted->next;
            }

            deep_copy->state |= node_state::created;
            parent->add_node(deep_copy);
        }

        undo_history().push(strings::gui_tree_ops_history_paste_into,
        new std::function([snapshot] (ex::State *s) {
            TreeState::reset(tree::state());
            snapshot->rollback(s);
        }),
        new std::function([parent_id, group_id, move, selection, pasted] (ex::State *s) {
            TreeState::reset(tree::state());
            const auto p_node = s->work_tree->find_node(parent_id);
            if (p_node == nullptr)
                throw std::runtime_error("Cannot find original node");

            auto root = selection;
            while (root != nullptr) {
                const auto f_node = s->work_tree->find_node(root->selected->id_local);
                if (f_node == nullptr)
                    throw std::runtime_error("Cannot find original node");
                f_node->state -= node_state::moved;
                f_node->state |= (node_state::moved && move);
                root = root->next;
            }

            if (p_node->children_n <= 0 || p_node->children == nullptr) {
                TreeNode *group = TreeNode::create();
                group->id_local = group_id;
                group->state |= node_state::is_group;
                group->state |= node_state::created;
                p_node->add_node(group);
            }

            TreeNode *group = p_node->children[0];
            root            = pasted->root;
            while (root != nullptr) {
                const auto dcp = TreeNode::deep_copy(root->selected);
                dcp->state |= node_state::created;
                group->add_node(dcp);
                root = root->next;
            }
        }),
        new std::function([snapshot, selection, pasted] {
            Snapshot::free(snapshot);
            selection_node::free(selection);
            selection_node::free(pasted);
        }));
    }

    void move_after_node(TreeNode *node, TreeNode *move) {
        if (node == nullptr || move == nullptr)
            return;

        State &state = ex::logic::globals::state();

        TreeState::reset(tree::state());
        const auto snapshot = Snapshot::create(state);

        const auto parent    = node->parent;
        const auto index     = node->index;
        const auto parent_id = parent->id_local;

        TreeNode *copy = TreeNode::deep_copy(move);
        copy->state |= node_state::created;
        move->state |= node_state::moved;

        TreeNode *nco = TreeNode::deep_copy(copy);
        parent->insert_node(index + 1, nco);
        state.active_node = nco;

        undo_history().push(strings::gui_tree_ops_history_move_after,
        new std::function([snapshot] (ex::State *s) {
            TreeState::reset(tree::state());
            snapshot->rollback(s);
        }),
        new std::function([parent_id, index, copy] (ex::State *s) {
            TreeState::reset(tree::state());
            const auto p_node = s->work_tree->find_node(parent_id);
            if (p_node == nullptr)
                throw std::runtime_error("Cannot find original node");

            const auto f_node = s->work_tree->find_node(copy->id_local);
            if (f_node == nullptr)
                throw std::runtime_error("Cannot find original node");
            f_node->state |= node_state::moved;

            TreeNode *new_copy = TreeNode::deep_copy(copy);
            new_copy->state |= node_state::created;

            p_node->insert_node(index + 1, new_copy);
            s->active_node = new_copy;
        }),
        new std::function([snapshot, copy] {
            Snapshot::free(snapshot);
            if (copy != nullptr)
                delete copy;
        }));
    }

    void move_into_node(TreeNode *node, TreeNode *move) {
        if (node == nullptr || move == nullptr)
            return;

        State &state = ex::logic::globals::state();

        TreeState::reset(tree::state());
        const auto snapshot = Snapshot::create(state);

        if (node->children == nullptr || node->children_n <= 0) {
            TreeNode *group = TreeNode::create();
            group->state |= node_state::is_group;
            group->state |= node_state::created;
            node->add_node(group);
        }

        TreeNode  *parent    = node->children[0];
        const auto parent_id = node->id_local;
        const auto group_id  = parent->id_local;

        TreeNode *copy = TreeNode::deep_copy(move);
        copy->state |= node_state::created;
        move->state |= node_state::moved;

        TreeNode *nco = TreeNode::deep_copy(copy);
        parent->add_node(nco);

        state.active_node = nco;

        undo_history().push(strings::gui_tree_ops_history_move_into,
        new std::function([snapshot] (ex::State *s) {
            TreeState::reset(tree::state());
            snapshot->rollback(s);
        }),
        new std::function([parent_id, group_id, copy] (ex::State *s) {
            TreeState::reset(tree::state());
            const auto p_node = s->work_tree->find_node(parent_id);
            if (p_node == nullptr)
                throw std::runtime_error("Cannot find original node");

            const auto f_node = s->work_tree->find_node(copy->id_local);
            if (f_node == nullptr)
                throw std::runtime_error("Cannot find original node");
            f_node->state |= node_state::moved;

            if (p_node->children_n <= 0 || p_node->children == nullptr) {
                TreeNode *group = TreeNode::create();
                group->id_local = group_id;
                group->state |= node_state::is_group;
                group->state |= node_state::created;
                p_node->add_node(group);
            }

            TreeNode *group    = p_node->children[0];
            TreeNode *new_copy = TreeNode::deep_copy(copy);
            new_copy->state |= node_state::created;
            group->add_node(new_copy);
            s->active_node = new_copy;
        }),
        new std::function([snapshot, copy] {
            Snapshot::free(snapshot);
            if (copy != nullptr)
                delete copy;
        }));
    }

    void group_selected(selection_node *selection) {
        const auto sorted    = bubble_sort_selection(selection->root);
        const auto parent    = selection->root->selected->parent;
        const auto parent_id = parent->id_local;

        const auto &state = ex::logic::globals::state();

        TreeState::reset(tree::state());
        const auto snapshot = state.work_tree->deep_copy();

        TreeNode *new_group = TreeNode::create();
        new_group->name     = "generic_group";
        new_group->state |= node_state::is_group;
        new_group->state |= node_state::created;

        int             index = -1;
        selection_node *head  = sorted;
        while (head != nullptr) {
            if (index == -1)
                index = head->selected->index;
            TreeNode *new_one = head->selected->deep_copy();
            new_one->state |= node_state::created;

            new_group->add_node(new_one);
            head->selected->state |= node_state::moved;
            head->selected        = TreeNode::shallow_copy(head->selected);
            head                  = head->next;
        }

        if (index >= 0)
            parent->insert_node(index, new_group->deep_copy());

        undo_history().push(strings::gui_tree_ops_history_group,
        new std::function([snapshot] (ex::State *s) {
            TreeState::reset(tree::state());
            if (s->work_tree != nullptr && s->work_tree != snapshot)
                delete s->work_tree;
            s->work_tree            = snapshot->deep_copy();
            s->work_tree->state -= node_state::connected;
            s->work_tree->state |= node_state::created;
        }),

        new std::function([new_group, parent_id, index, sorted] (ex::State *s) {
            TreeState::reset(tree::state());
            if (index < 0)
                throw std::runtime_error("Cannot find original node, index < 0");

            auto node = sorted;
            while (node != nullptr) {
                const auto child = s->work_tree->find_node(node->selected->id_local);
                if (child == nullptr)
                    throw std::runtime_error("Cannot find original child node");
                child->state |= node_state::moved;
                node         = node->next;
            }

            const auto p_node = s->work_tree->find_node(parent_id);
            if (p_node == nullptr)
                throw std::runtime_error("Cannot find original parent node");

            p_node->insert_node(index, new_group->deep_copy());
        }),

        new std::function([new_group, snapshot, sorted] {
            if (new_group != nullptr)
                delete new_group;
            if (snapshot != nullptr)
                delete snapshot;
            selection_node::free(sorted);
        }));
    }

    void ungroup_selected(selection_node *selection) {
        const auto sorted = bubble_sort_selection(selection->root);
        const auto parent = selection->root->selected->parent;

        const std::size_t parent_id = parent->parent->id_local;
        const std::size_t index     = parent->index;

        const auto &state = ex::logic::globals::state();

        TreeState::reset(tree::state());
        const auto snapshot = state.work_tree->deep_copy();

        TreeNode       *temp_group = TreeNode::create();
        selection_node *head       = sorted;

        while (head != nullptr) {
            TreeNode *new_one = head->selected->deep_copy();
            new_one->state |= node_state::created;

            temp_group->add_node(new_one);
            head->selected->state |= node_state::moved;
            head->selected        = TreeNode::shallow_copy(head->selected);
            head                  = head->next;
        }

        for (int c = 0, i = index; c < temp_group->children_n; c++) {
            parent->parent->insert_node(++i, temp_group->children[c]->deep_copy());
        }

        undo_history().push(strings::gui_tree_ops_history_ungroup,
        new std::function([snapshot] (ex::State *s) {
            TreeState::reset(tree::state());
            if (s->work_tree != nullptr && s->work_tree != snapshot)
                delete s->work_tree;
            s->work_tree            = snapshot->deep_copy();
            s->work_tree->state -= node_state::connected;
            s->work_tree->state |= node_state::created;
        }),

        new std::function([index, parent_id, sorted, temp_group] (ex::State *s) {
            TreeState::reset(tree::state());
            if (index < 0)
                throw std::runtime_error("Cannot find original node, index < 0");

            auto node = sorted;
            while (node != nullptr) {
                const auto child = s->work_tree->find_node(node->selected->id_local);
                if (child == nullptr)
                    throw std::runtime_error("Cannot find original child node");
                child->state |= node_state::moved;
                node         = node->next;
            }

            const auto p_node = s->work_tree->find_node(parent_id);
            if (p_node == nullptr)
                throw std::runtime_error("Cannot find original parent node");

            for (int c = 0, i = index; c < temp_group->children_n; c++) {
                const auto child = temp_group->children[c]->deep_copy();
                child->state |= node_state::created;
                p_node->insert_node(++i, child);
            }

        }),

        new std::function([snapshot, temp_group, sorted] {
            if (temp_group != nullptr)
                delete temp_group;
            if (snapshot != nullptr)
                delete snapshot;
            selection_node::free(sorted);
        }));
    }

    void enable_node(TreeNode *node) {
        const auto og_state = node->state;
        node->state -= node_state::disabled;

        State &state = ex::logic::globals::state();

        TreeState::reset(tree::state());

        state.force_rewire = true;

        undo_history().push(strings::gui_tree_ops_history_node_enabled,
        new std::function([id_l = node->id_local, og_state] (ex::State *s) {
            TreeState::reset(tree::state());
            s->force_rewire = true;
            const auto f_node = s->work_tree->find_node(id_l);
            if (f_node == nullptr)
                throw std::runtime_error("Cannot find original node");
            f_node->state -= node_state::disabled;
            f_node->state |= (og_state & node_state::disabled);
        }),
        new std::function([id_l = node->id_local] (ex::State *s) {
            TreeState::reset(tree::state());
            s->force_rewire = true;
            const auto f_node = s->work_tree->find_node(id_l);
            if (f_node == nullptr)
                throw std::runtime_error("Cannot find original node");
            f_node->state -= node_state::disabled;
        }));
    }

    void disable_node(TreeNode *node) {
        const auto og_state = node->state;
        node->state |= node_state::disabled;

        State &state = ex::logic::globals::state();

        TreeState::reset(tree::state());
        state.force_rewire = true;

        undo_history().push(strings::gui_tree_ops_history_node_disabled,
        new std::function([id_l = node->id_local, og_state] (ex::State *s) {
            TreeState::reset(tree::state());
            s->force_rewire = true;
            const auto f_node = s->work_tree->find_node(id_l);
            if (f_node == nullptr)
                throw std::runtime_error("Cannot find original node");
            f_node->state -= node_state::disabled;
            f_node->state |= (og_state & node_state::disabled);
        }),
        new std::function([id_l = node->id_local] (ex::State *s) {
            TreeState::reset(tree::state());
            s->force_rewire = true;
            const auto f_node = s->work_tree->find_node(id_l);
            if (f_node == nullptr)
                throw std::runtime_error("Cannot find original node");
            f_node->state |= node_state::disabled;
        }));
    }

    void enable_selected(selection_node *selection) {
        if (selection == nullptr)
            return;

        std::map<std::size_t, node_state> og_map;
        auto head = selection->root;
        while (head != nullptr) {
            og_map.emplace(head->selected->id_local, head->selected->state);
            head->selected->state -= node_state::disabled;
            head = head->next;
        }

        State &state = ex::logic::globals::state();

        TreeState::reset(tree::state());
        state.force_rewire = true;

        undo_history().push(strings::gui_tree_ops_history_select_enabled,
        new std::function([og_state = og_map](ex::State *s) {
            TreeState::reset(tree::state());
            s->force_rewire = true;
            for (const auto &[k, v] : og_state) {
                const auto f_node = s->work_tree->find_node(k);
                if (f_node == nullptr)
                    throw std::runtime_error("Cannot find original node");
                f_node->state -= node_state::disabled;
                f_node->state |= (v & node_state::disabled);
            }
        }),
        new std::function([og_state = og_map](ex::State *s) {
            TreeState::reset(tree::state());
            s->force_rewire = true;
            for (const auto &k : std::views::keys(og_state)) {
                const auto f_node = s->work_tree->find_node(k);
                if (f_node == nullptr)
                    throw std::runtime_error("Cannot find original node");
                f_node->state -= node_state::disabled;
            }
        }));
    }

    void disable_selected(selection_node *selection) {
        if (selection == nullptr)
            return;

        std::map<std::size_t, node_state> og_map;
        auto head = selection->root;
        while (head != nullptr) {
            og_map.emplace(head->selected->id_local, head->selected->state);
            head->selected->state |= node_state::disabled;
            head = head->next;
        }

        State &state = ex::logic::globals::state();

        TreeState::reset(tree::state());
        state.force_rewire = true;

        undo_history().push(strings::gui_tree_ops_history_select_disabled,
        new std::function([og_state = og_map](ex::State *s) {
            s->force_rewire = true;
            for (const auto &[k, v] : og_state) {
                const auto f_node = s->work_tree->find_node(k);
                if (f_node == nullptr)
                    throw std::runtime_error("Cannot find original node");
                f_node->state -= node_state::disabled;
                f_node->state |= (v & node_state::disabled);
            }
            TreeState::reset(tree::state());
        }),
        new std::function([og_state = og_map](ex::State *s) {
            s->force_rewire = true;
            for (const auto &k : std::views::keys(og_state)) {
                const auto f_node = s->work_tree->find_node(k);
                if (f_node == nullptr)
                    throw std::runtime_error("Cannot find original node");
                f_node->state |= node_state::disabled;
            }
            TreeState::reset(tree::state());
        }));
    }

    void delete_selected(selection_node *selection) {
        if (selection == nullptr)
            return;

        State &state = ex::logic::globals::state();

        const auto copy_selection = selection_node::copy(selection->root);
        TreeState::reset(tree::state());

        auto head = copy_selection->root;
        while (head != nullptr) {
            head->selected->state -= node_state::selected;
            head = head->next;
        }

        const auto snapshot = Snapshot::create(state);

        head = copy_selection->root;
        while (head != nullptr) {
            if (!(head->selected->state >>= node_state::is_group))
                head->selected->state |= node_state::deleted;
            head->selected = TreeNode::shallow_copy(head->selected);
            head = head->next;
        }

        undo_history().push(strings::gui_tree_ops_history_select_deleted,
        new std::function([snapshot] (ex::State *s) {
            TreeState::reset(tree::state());
            snapshot->rollback(s);
        }),
        new std::function([copy_selection] (ex::State *s) {
            TreeState::reset(tree::state());
            auto s_head = copy_selection->root;
            while (s_head != nullptr) {
                const auto f_node = s->work_tree->find_node(s_head->selected->id_local);
                if (f_node != nullptr && !(f_node->state >>= node_state::is_group))
                    f_node->state |= node_state::deleted;
                s_head = s_head->next;
            }
        }),
        new std::function([snapshot, copy_selection] {
            Snapshot::free(snapshot);
            selection_node::free(copy_selection);
        }));
    }

    void delete_node(TreeNode *node) {
        if (node == nullptr)
            return;

        State &state = ex::logic::globals::state();

        TreeState::reset(tree::state());

        node->state -= node_state::selected;
        const auto node_id  = node->id_local;
        const auto snapshot = Snapshot::create(state);
        node->state |= node_state::deleted;

        undo_history().push(strings::gui_tree_ops_history_node_deleted,
        new std::function([snapshot] (ex::State *s) {
            TreeState::reset(tree::state());
            snapshot->rollback(s);
        }),
        new std::function([node_id] (ex::State *s) {
            TreeState::reset(tree::state());

            const auto f_node = s->work_tree->find_node(node_id);
            if (f_node == nullptr)
               throw std::runtime_error("Cannot find original node");
            f_node->state |= node_state::deleted;
        }),
        new std::function([snapshot] {
            Snapshot::free(snapshot);
        }));
    }

}