//
// Created by xd on 17/01/25.
//

// ReSharper disable CppDFAMemoryLeak
// ReSharper disable CppDFANullDereference
#include <oneapi/tbb/partitioner.h>
#include <ranges>
#include "../../../headers/imgui/gui_keymap.h"
#include "../../../headers/imgui/gui_layout.h"
#include "../../../headers/imgui/gui_tree_ops.h"

namespace ex::gui {

    namespace tree {

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

        void reset_state_(State &state) {
            selection_node::free(state.selected_nodes, true);
            node_buffer::remove(&(state.clipboard_node));
            state.selected_nodes = nullptr;
        }

        void copy_selected(State &state, GuiState &, selection_node *selection) {
            const auto copy = selection_node::copy(selection->root);
            reset_state_(state);
            node_buffer::create(&state.clipboard_node, copy->root, false, false);
            selection_node::free(copy);
        }

        void copy_node(State &state, GuiState &, TreeNode *node) {
            reset_state_(state);
            node_buffer::create(&state.clipboard_node, node, false, false);
        }

        void ref_node(State &state, GuiState &, TreeNode *node) {
            reset_state_(state);
            node_buffer::create(&state.clipboard_node, node, false, true);
        }

        void add_node(State &state, GuiState &, TreeNode *add) {
            reset_state_(state);
            const auto snapshot = state.work_tree->deep_copy();

            TreeNode *copy = TreeNode::deep_copy(add);
            copy->created  = true;
            add->created   = true;

            state.work_tree->add_node(add);

            undo_history().push("Add node",
            new std::function([snapshot](State *s) {
                reset_state_(*s);
                if (s->work_tree != nullptr && s->work_tree != snapshot)
                    delete s->work_tree;
                s->work_tree            = snapshot->deep_copy();
                s->work_tree->connected = false;
                s->work_tree->created   = true;
            }),
            new std::function([copy](State *s) {
                reset_state_(*s);
                TreeNode *new_copy = TreeNode::deep_copy(copy);
                new_copy->created  = true;
                s->work_tree->add_node(new_copy);
            }),
            new std::function([snapshot, copy] {
                if (snapshot != nullptr)
                    delete snapshot;
                if (copy != nullptr)
                    delete copy;
            }));
        }

        void add_into_node(State &state, GuiState &, TreeNode *node, TreeNode *add) {
            reset_state_(state);
            const auto snapshot = state.work_tree->deep_copy();

            TreeNode *copy = TreeNode::deep_copy(add);
            copy->created  = true;
            add->created   = true;

            if (node->children == nullptr || node->children_n <= 0) {
                TreeNode *group = TreeNode::create();
                group->is_group = true;
                group->created  = true;
                node->add_node(group);
            }

            TreeNode  *parent    = node->children[0];
            const auto parent_id = node->id_local;
            const auto group_id  = parent->id_local;

            parent->add_node(add);
            state.active_node = add;

            undo_history().push("Add into",
            new std::function([snapshot] (ex::State *s) {
                reset_state_(*s);
                if (s->work_tree != nullptr && s->work_tree != snapshot)
                    delete s->work_tree;
                s->work_tree            = snapshot->deep_copy();
                s->work_tree->connected = false;
                s->work_tree->created   = true;
            }),
            new std::function([parent_id, group_id, copy] (ex::State *s) {
                reset_state_(*s);
                const auto p_node = s->work_tree->find_node(parent_id);
                if (p_node == nullptr)
                    throw std::runtime_error("Cannot find original node");

                if (p_node->children_n <= 0 || p_node->children == nullptr) {
                    TreeNode *group = TreeNode::create();
                    group->id_local = group_id;
                    group->is_group = true;
                    group->created  = true;
                    p_node->add_node(group);
                }

                TreeNode *group    = p_node->children[0];
                TreeNode *new_copy = TreeNode::deep_copy(copy);
                new_copy->created  = true;
                group->add_node(new_copy);
            }),
            new std::function([snapshot, copy] {
                if (snapshot != nullptr)
                    delete snapshot;
                if (copy != nullptr)
                    delete copy;
            }));
        }

        void add_after_node(State &state, GuiState &, TreeNode *node, TreeNode *add) {
            reset_state_(state);
            const auto snapshot = state.work_tree->deep_copy();

            TreeNode *copy = TreeNode::deep_copy(add);
            copy->created  = true;
            add->created   = true;

            const auto parent    = node->parent;
            const auto index     = node->index;
            const auto parent_id = parent->id_local;

            parent->insert_node(index + 1, add);

            undo_history().push("Add after",
            new std::function([snapshot] (ex::State *s) {
                reset_state_(*s);
                if (s->work_tree != nullptr && s->work_tree != snapshot)
                    delete s->work_tree;
                s->work_tree            = snapshot->deep_copy();
                s->work_tree->connected = false;
                s->work_tree->created   = true;
            }),
            new std::function([parent_id, index, copy] (ex::State *s) {
                reset_state_(*s);
                const auto p_node = s->work_tree->find_node(parent_id);
                if (p_node == nullptr)
                    throw std::runtime_error("Cannot find original node");

                TreeNode *new_copy = TreeNode::deep_copy(copy);
                new_copy->created  = true;

                p_node->insert_node(index + 1, new_copy);
            }),
            new std::function([snapshot, copy] {
                if (snapshot != nullptr)
                    delete snapshot;
                if (copy != nullptr)
                    delete copy;
            }));
        }

        void cut_selected(State &state, GuiState &, selection_node *selection) {
            const auto copy = selection_node::copy(selection->root);
            reset_state_(state);

            node_buffer::create(&state.clipboard_node, copy->root, true, false);
            auto head = copy->root;
            while (head != nullptr) {
                head->selected->dimmed = true;
                head = head->next;
            }
            selection_node::free(copy);
        }

        void cut_node(State &state, GuiState &, TreeNode *node) {
            reset_state_(state);
            node_buffer::create(&state.clipboard_node, node, true, false);
            node->dimmed = true;
        }

        void rename_node(State &state, GuiState &, TreeNode *node, const char *name) {
            const auto id_local = node->id_local;
            const auto og_name = node->name;
            const auto new_name = new char[64]{ "" };

            std::memcpy(new_name, name, std::strlen(name));
            node->name = new_name;

            reset_state_(state);

            undo_history().push("Node renamed",
            new std::function([id_local, og_name] (State *s) {
                reset_state_(*s);
                const auto f_node = s->work_tree->find_node(id_local);
                if (f_node == nullptr)
                    throw std::runtime_error("Cannot find original node");
                f_node->name = og_name;
            }),
            new std::function([id_local, new_name] (State *s) {
                reset_state_(*s);
                const auto f_node = s->work_tree->find_node(id_local);
                if (f_node == nullptr)
                    throw std::runtime_error("Cannot find original node");
                f_node->name = new_name;
            }));
        }

        void duplicate_node(State &state, GuiState &, TreeNode *node) {
            reset_state_(state);
            const auto snapshot = state.work_tree->deep_copy();

            TreeNode *copy = TreeNode::deep_copy(node, true);
            copy->name     = (new std::string(node->name))->append(" (copy)").c_str();
            copy->parent   = nullptr;
            copy->index    = 0;

            const auto parent_id   = node->parent->id_local;
            const auto child_index = node->index + 1;

            TreeNode *snap = TreeNode::deep_copy(copy);
            node->parent->insert_node(child_index, copy);

            undo_history().push("Node duplicated",
            new std::function([snapshot] (ex::State *s) -> void {
                reset_state_(*s);
                if (s->work_tree != nullptr && s->work_tree != snapshot)
                    delete s->work_tree;
                s->work_tree            = snapshot->deep_copy();
                s->work_tree->created   = true;
                s->work_tree->connected = false;
            }),

            new std::function([parent_id, child_index, snap] (ex::State *s) -> void {
                reset_state_(*s);
                if (const auto p_node = s->work_tree->find_node(parent_id); p_node != nullptr) {
                    const auto new_copy = snap->deep_copy();
                    new_copy->created   = true;
                    p_node->insert_node(child_index, new_copy);
                } else
                    throw std::runtime_error("Cannot find original node");
            }),

            new std::function([snapshot, snap] {
                if (snapshot != nullptr)
                    delete snapshot;
                if (snap != nullptr)
                    delete snap;
            }));
        }

        void paste_after_node(State &state, GuiState &, TreeNode *node) {
            if (node == nullptr || node->parent == nullptr || state.clipboard_node == nullptr) {
                reset_state_(state);
                return;
            }
            const auto parent    = node->parent;
            const auto parent_id = parent->id_local;
            const auto index     = node->index;

            const auto selection = bubble_sort_selection(state.clipboard_node->selection->root);
            const bool move      = state.clipboard_node->move;
            const bool ref       = state.clipboard_node->ref;

            reset_state_(state);
            const auto snapshot = state.work_tree->deep_copy();

            selection_node *pasted = nullptr;

            int c = index;
            auto head = selection;
            while (head != nullptr) {
                TreeNode *deep_copy = ref ? TreeNode::link(head->selected) : TreeNode::deep_copy(head->selected, !move);
                const auto copy = TreeNode::shallow_copy(head->selected);

                if (!move && !ref) {
                    deep_copy->name = (new std::string(deep_copy->name))->append(" (copy)").c_str();
                }

                head->selected->moved = move;
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

                deep_copy->created = true;
                parent->insert_node(++c, deep_copy);
            }

            undo_history().push("Paste after",
            new std::function([snapshot] (ex::State *s) {
                reset_state_(*s);
                if (s->work_tree != nullptr && s->work_tree != snapshot)
                    delete s->work_tree;
                s->work_tree            = snapshot->deep_copy();
                s->work_tree->connected = false;
                s->work_tree->created   = true;
            }),
            new std::function([parent_id, move, index, selection, pasted] (ex::State *s) {
                reset_state_(*s);
                const auto p_node = s->work_tree->find_node(parent_id);
                if (p_node == nullptr)
                    throw std::runtime_error("Cannot find original node");

                auto root = selection;
                while (root != nullptr) {
                    const auto f_node = s->work_tree->find_node(root->selected->id_local);
                    if (f_node == nullptr)
                        throw std::runtime_error("Cannot find original node");
                    f_node->moved = move;
                    root = root->next;
                }

                int k = index;
                root = pasted->root;
                while (root != nullptr) {
                    const auto dcp = TreeNode::deep_copy(root->selected);
                    dcp->created   = true;
                    p_node->insert_node(++k, dcp);
                    root = root->next;
                }
            }),
            new std::function([snapshot, selection, pasted] {
                if (snapshot != nullptr)
                    delete snapshot;
                selection_node::free(selection);
                selection_node::free(pasted);
            }));
        }

        void paste_into_node(State &state, GuiState &, TreeNode *node) {
            if (node == nullptr || state.clipboard_node == nullptr) {
                reset_state_(state);
                return;
            }

            const auto selection = bubble_sort_selection(state.clipboard_node->selection->root);
            const bool move      = state.clipboard_node->move;
            const bool ref       = state.clipboard_node->ref;

            reset_state_(state);
            const auto snapshot = state.work_tree->deep_copy();

            if (node->children == nullptr || node->children_n <= 0) {
                TreeNode *group = TreeNode::create();
                group->is_group = true;
                group->created  = true;
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

                head->selected->moved = move;
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

                deep_copy->created = true;
                parent->add_node(deep_copy);
            }

            undo_history().push("Paste into",
            new std::function([snapshot] (ex::State *s) {
                reset_state_(*s);
                if (s->work_tree != nullptr && s->work_tree != snapshot)
                    delete s->work_tree;
                s->work_tree            = snapshot->deep_copy();
                s->work_tree->connected = false;
                s->work_tree->created   = true;
            }),
            new std::function([parent_id, group_id, move, selection, pasted] (ex::State *s) {
                reset_state_(*s);
                const auto p_node = s->work_tree->find_node(parent_id);
                if (p_node == nullptr)
                    throw std::runtime_error("Cannot find original node");

                auto root = selection;
                while (root != nullptr) {
                    const auto f_node = s->work_tree->find_node(root->selected->id_local);
                    if (f_node == nullptr)
                        throw std::runtime_error("Cannot find original node");
                    f_node->moved = move;
                    root = root->next;
                }

                if (p_node->children_n <= 0 || p_node->children == nullptr) {
                    TreeNode *group = TreeNode::create();
                    group->id_local = group_id;
                    group->is_group = true;
                    group->created  = true;
                    p_node->add_node(group);
                }

                TreeNode *group = p_node->children[0];
                root            = pasted->root;
                while (root != nullptr) {
                    const auto dcp = TreeNode::deep_copy(root->selected);
                    dcp->created   = true;
                    group->add_node(dcp);
                    root = root->next;
                }
            }),
            new std::function([snapshot, selection, pasted] {
                if (snapshot != nullptr)
                    delete snapshot;
                selection_node::free(selection);
                selection_node::free(pasted);
            }));
        }

        void move_after_node(State &state, GuiState &, TreeNode *node, TreeNode *move) {
            if (node == nullptr || move == nullptr)
                return;

            reset_state_(state);
            const auto snapshot = state.work_tree->deep_copy();

            const auto parent    = node->parent;
            const auto index     = node->index;
            const auto parent_id = parent->id_local;

            TreeNode *copy = TreeNode::deep_copy(move);
            copy->created  = true;
            move->moved    = true;

            parent->insert_node(index + 1, TreeNode::deep_copy(copy));

            undo_history().push("Move after",
            new std::function([snapshot] (ex::State *s) {
                reset_state_(*s);
                if (s->work_tree != nullptr && s->work_tree != snapshot)
                    delete s->work_tree;
                s->work_tree            = snapshot->deep_copy();
                s->work_tree->connected = false;
                s->work_tree->created   = true;
            }),
            new std::function([parent_id, index, copy] (ex::State *s) {
                reset_state_(*s);
                const auto p_node = s->work_tree->find_node(parent_id);
                if (p_node == nullptr)
                    throw std::runtime_error("Cannot find original node");

                const auto f_node = s->work_tree->find_node(copy->id_local);
                if (f_node == nullptr)
                    throw std::runtime_error("Cannot find original node");
                f_node->moved = true;

                TreeNode *new_copy = TreeNode::deep_copy(copy);
                new_copy->created  = true;

                p_node->insert_node(index + 1, new_copy);
            }),
            new std::function([snapshot, copy] {
                if (snapshot != nullptr)
                    delete snapshot;
                if (copy != nullptr)
                    delete copy;
            }));
        }

        void move_into_node(State &state, GuiState &, TreeNode *node, TreeNode *move) {
            if (node == nullptr || move == nullptr)
                return;

            reset_state_(state);
            const auto snapshot = state.work_tree->deep_copy();

            if (node->children == nullptr || node->children_n <= 0) {
                TreeNode *group = TreeNode::create();
                group->is_group = true;
                group->created  = true;
                node->add_node(group);
            }

            TreeNode  *parent    = node->children[0];
            const auto parent_id = node->id_local;
            const auto group_id  = parent->id_local;

            TreeNode *copy = TreeNode::deep_copy(move);
            copy->created  = true;
            move->moved    = true;

            parent->add_node(TreeNode::deep_copy(copy));

            undo_history().push("Move into",
            new std::function([snapshot] (ex::State *s) {
                reset_state_(*s);
                if (s->work_tree != nullptr && s->work_tree != snapshot)
                    delete s->work_tree;
                s->work_tree            = snapshot->deep_copy();
                s->work_tree->connected = false;
                s->work_tree->created   = true;
            }),
            new std::function([parent_id, group_id, copy] (ex::State *s) {
                reset_state_(*s);
                const auto p_node = s->work_tree->find_node(parent_id);
                if (p_node == nullptr)
                    throw std::runtime_error("Cannot find original node");

                const auto f_node = s->work_tree->find_node(copy->id_local);
                if (f_node == nullptr)
                    throw std::runtime_error("Cannot find original node");
                f_node->moved = true;

                if (p_node->children_n <= 0 || p_node->children == nullptr) {
                    TreeNode *group = TreeNode::create();
                    group->id_local = group_id;
                    group->is_group = true;
                    group->created  = true;
                    p_node->add_node(group);
                }

                TreeNode *group    = p_node->children[0];
                TreeNode *new_copy = TreeNode::deep_copy(copy);
                new_copy->created  = true;
                group->add_node(new_copy);
            }),
            new std::function([snapshot, copy] {
                if (snapshot != nullptr)
                    delete snapshot;
                if (copy != nullptr)
                    delete copy;
            }));
        }

        void group_selected(State &state, GuiState &, selection_node *selection) {
            const auto sorted    = bubble_sort_selection(selection->root);
            const auto parent    = selection->root->selected->parent;
            const auto parent_id = parent->id_local;

            reset_state_(state);
            const auto snapshot = state.work_tree->deep_copy();

            TreeNode *new_group = TreeNode::create();
            new_group->name     = "generic_group";
            new_group->is_group = true;
            new_group->created  = true;

            int             index = -1;
            selection_node *head  = sorted;
            while (head != nullptr) {
                if (index == -1)
                    index = head->selected->index;
                TreeNode *new_one = head->selected->deep_copy();
                new_one->created  = true;

                new_group->add_node(new_one);
                head->selected->moved = true;
                head->selected        = TreeNode::shallow_copy(head->selected);
                head                  = head->next;
            }

            if (index >= 0)
                parent->insert_node(index, new_group->deep_copy());

            undo_history().push("Group selected",
            new std::function([snapshot] (ex::State *s) {
                reset_state_(*s);
                if (s->work_tree != nullptr && s->work_tree != snapshot)
                    delete s->work_tree;
                s->work_tree            = snapshot->deep_copy();
                s->work_tree->connected = false;
                s->work_tree->created   = true;
            }),

            new std::function([new_group, parent_id, index, sorted] (ex::State *s) {
                reset_state_(*s);
                if (index < 0)
                    throw std::runtime_error("Cannot find original node, index < 0");

                auto node = sorted;
                while (node != nullptr) {
                    const auto child = s->work_tree->find_node(node->selected->id_local);
                    if (child == nullptr)
                        throw std::runtime_error("Cannot find original child node");
                    child->moved = true;
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

        void ungroup_selected(State &state, GuiState &, selection_node *selection) {
            const auto sorted = bubble_sort_selection(selection->root);
            const auto parent = selection->root->selected->parent;

            const std::size_t parent_id = parent->parent->id_local;
            const std::size_t index     = parent->index;

            reset_state_(state);
            const auto snapshot = state.work_tree->deep_copy();

            TreeNode       *temp_group = TreeNode::create();
            selection_node *head       = sorted;

            while (head != nullptr) {
                TreeNode *new_one = head->selected->deep_copy();
                new_one->created  = true;

                temp_group->add_node(new_one);
                head->selected->moved = true;
                head->selected        = TreeNode::shallow_copy(head->selected);
                head                  = head->next;
            }

            for (int c = 0, i = index; c < temp_group->children_n; c++) {
                parent->parent->insert_node(++i, temp_group->children[c]->deep_copy());
            }

            undo_history().push("Ungroup selected",
            new std::function([snapshot] (ex::State *s) {
                reset_state_(*s);
                if (s->work_tree != nullptr && s->work_tree != snapshot)
                    delete s->work_tree;
                s->work_tree            = snapshot->deep_copy();
                s->work_tree->connected = false;
                s->work_tree->created   = true;
            }),

            new std::function([index, parent_id, sorted, temp_group] (ex::State *s) {
                reset_state_(*s);
                if (index < 0)
                    throw std::runtime_error("Cannot find original node, index < 0");

                auto node = sorted;
                while (node != nullptr) {
                    const auto child = s->work_tree->find_node(node->selected->id_local);
                    if (child == nullptr)
                        throw std::runtime_error("Cannot find original child node");
                    child->moved = true;
                    node         = node->next;
                }

                const auto p_node = s->work_tree->find_node(parent_id);
                if (p_node == nullptr)
                    throw std::runtime_error("Cannot find original parent node");

                for (int c = 0, i = index; c < temp_group->children_n; c++) {
                    const auto child = temp_group->children[c]->deep_copy();
                    child->created   = true;
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

        void enable_node(State &state, GuiState &, TreeNode *node) {
            const bool og_state = node->disabled;
            node->disabled = false;

            reset_state_(state);

            state.force_rewire = true;

            undo_history().push("Node enabled",
            new std::function([id_l = node->id_local, og_state] (ex::State *s) {
                reset_state_(*s);
                s->force_rewire = true;
                const auto f_node = s->work_tree->find_node(id_l);
                if (f_node == nullptr)
                    throw std::runtime_error("Cannot find original node");
                f_node->disabled = og_state;
                f_node->updated = true;
            }),
            new std::function([id_l = node->id_local] (ex::State *s) {
                reset_state_(*s);
                s->force_rewire = true;
                const auto f_node = s->work_tree->find_node(id_l);
                if (f_node == nullptr)
                    throw std::runtime_error("Cannot find original node");
                f_node->disabled = false;
                f_node->updated = true;
            }));
        }

        void disable_node(State &state, GuiState &, TreeNode *node) {
            const bool og_state = node->disabled;
            node->disabled = true;

            reset_state_(state);
            state.force_rewire = true;

            undo_history().push("Node disabled",
            new std::function([id_l = node->id_local, og_state] (ex::State *s) {
                reset_state_(*s);
                s->force_rewire = true;
                const auto f_node = s->work_tree->find_node(id_l);
                if (f_node == nullptr)
                    throw std::runtime_error("Cannot find original node");
                f_node->disabled = og_state;
                f_node->updated = true;
            }),
            new std::function([id_l = node->id_local] (ex::State *s) {
                reset_state_(*s);
                s->force_rewire = true;
                const auto f_node = s->work_tree->find_node(id_l);
                if (f_node == nullptr)
                    throw std::runtime_error("Cannot find original node");
                f_node->disabled = true;
                f_node->updated = true;
            }));
        }

        void enable_selected(State &state, GuiState &, selection_node *selection) {
            if (selection == nullptr)
                return;

            std::map<std::size_t, bool> og_map;
            auto head = selection->root;
            while (head != nullptr) {
                og_map.emplace(head->selected->id_local, head->selected->disabled);
                head->selected->disabled = false;
                head = head->next;
            }

            reset_state_(state);
            state.force_rewire = true;

            undo_history().push("Selection enabled",
            new std::function([og_state = og_map](ex::State *s) {
                reset_state_(*s);
                s->force_rewire = true;
                for (const auto &[k, v] : og_state) {
                    const auto f_node = s->work_tree->find_node(k);
                    if (f_node == nullptr)
                        throw std::runtime_error("Cannot find original node");
                    f_node->disabled = v;
                    f_node->updated = true;
                }
            }),
            new std::function([og_state = og_map](ex::State *s) {
                reset_state_(*s);
                s->force_rewire = true;
                for (const auto &k : std::views::keys(og_state)) {
                    const auto f_node = s->work_tree->find_node(k);
                    if (f_node == nullptr)
                        throw std::runtime_error("Cannot find original node");
                    f_node->disabled = false;
                    f_node->updated = true;
                }
            }));
        }

        void disable_selected(State &state, GuiState &, selection_node *selection) {
            if (selection == nullptr)
                return;

            std::map<std::size_t, bool> og_map;
            auto head = selection->root;
            while (head != nullptr) {
                og_map.emplace(head->selected->id_local, head->selected->disabled);
                head->selected->disabled = true;
                head = head->next;
            }

            reset_state_(state);
            state.force_rewire = true;

            undo_history().push("Selection disabled",
            new std::function([og_state = og_map](ex::State *s) {
                s->force_rewire = true;
                for (const auto &[k, v] : og_state) {
                    const auto f_node = s->work_tree->find_node(k);
                    if (f_node == nullptr)
                        throw std::runtime_error("Cannot find original node");
                    f_node->disabled = v;
                    f_node->updated = true;
                }
                reset_state_(*s);
            }),
            new std::function([og_state = og_map](ex::State *s) {
                s->force_rewire = true;
                for (const auto &k : std::views::keys(og_state)) {
                    const auto f_node = s->work_tree->find_node(k);
                    if (f_node == nullptr)
                        throw std::runtime_error("Cannot find original node");
                    f_node->disabled = true;
                    f_node->updated = true;
                }
                reset_state_(*s);
            }));
        }

        void delete_selected(State &state, GuiState &, selection_node *selection) {
            if (selection == nullptr)
                return;

            const auto copy_selection = selection_node::copy(selection->root);
            reset_state_(state);

            auto head = copy_selection->root;
            while (head != nullptr) {
                head->selected->selected = false;
                head = head->next;
            }

            const auto snapshot = state.work_tree->deep_copy();

            head = copy_selection->root;
            while (head != nullptr) {
                if (!head->selected->is_group)
                    head->selected->deleted = true;
                head->selected = TreeNode::shallow_copy(head->selected);
                head = head->next;
            }

            undo_history().push("Selection deleted",
            new std::function([snapshot] (ex::State *s) {
                reset_state_(*s);
                if (s->work_tree != nullptr && s->work_tree != snapshot)
                    delete s->work_tree;
                s->work_tree            = snapshot->deep_copy();
                s->work_tree->connected = false;
                s->work_tree->created   = true;
            }),
            new std::function([copy_selection] (ex::State *s) {
                reset_state_(*s);
                auto s_head = copy_selection->root;
                while (s_head != nullptr) {
                    const auto f_node = s->work_tree->find_node(s_head->selected->id_local);
                    if (f_node != nullptr && !f_node->is_group)
                        f_node->deleted = true;
                    s_head = s_head->next;
                }
            }),
            new std::function([snapshot, copy_selection] {
                if (snapshot != nullptr)
                    delete snapshot;
                selection_node::free(copy_selection);
            }));
        }

        void delete_node(State &state, GuiState &, TreeNode *node) {
            if (node == nullptr)
                return;

            reset_state_(state);

            node->selected = false;
            const auto node_id  = node->id_local;
            const auto snapshot = state.work_tree->deep_copy();
            node->deleted = true;

            undo_history().push("Node deleted",
            new std::function([snapshot] (ex::State *s) {
                reset_state_(*s);

                if (s->work_tree != nullptr && s->work_tree != snapshot)
                    delete s->work_tree;
                s->work_tree            = snapshot->deep_copy();
                s->work_tree->connected = false;
                s->work_tree->created   = true;
            }),
            new std::function([node_id] (ex::State *s) {
                reset_state_(*s);

                const auto f_node = s->work_tree->find_node(node_id);
                if (f_node == nullptr)
                   throw std::runtime_error("Cannot find original node");
                s->selected_nodes = nullptr;
                f_node->deleted   = true;
            }),
            new std::function([snapshot] {
                if (snapshot != nullptr)
                    delete snapshot;
            }));
        }
    }

    bool show_context_menu_group(State &state, GuiState &gui, selection_node *selection) {
        if (selection == nullptr)
            return false;

        static auto menu_id = "show_context_menu_group";
        bool clicked = false;

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup(menu_id);
            clicked = true;
        }

        auto head = selection->root;
        bool enabled = false, disabled = false, same_root = true;
        while (head != nullptr) {
            enabled |= !head->selected->disabled;
            disabled |= head->selected->disabled;
            if (head->selected->parent != selection->root->selected->parent)
                same_root = false;
            head = head->next;
        }

        const bool is_grouped = selection->selected->parent != nullptr &&
                                selection->selected->parent->parent != nullptr &&
                                selection->selected->parent->parent->is_group;

        if (ImGui::BeginPopup(menu_id)) {

            if (enabled && ImGui::MenuItem("Disable")) {
                tree::disable_selected(state, gui, selection);
            }

            if (disabled && ImGui::MenuItem("Enable")) {
                tree::enable_selected(state, gui, selection);
            }

            if (same_root) {
                if (ImGui::MenuItem("Group")) {
                    tree::group_selected(state, gui, selection);
                }

                if (is_grouped) {
                    if (ImGui::MenuItem("Ungroup")) {
                        tree::ungroup_selected(state, gui, selection);
                    }
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Cut", key_mappings().work_tree.cut.label)) {
                    tree::cut_selected(state, gui, selection);
                }

                if (ImGui::MenuItem("Copy", key_mappings().work_tree.copy.label)) {
                    tree::copy_selected(state, gui, selection);
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Delete")) {
                tree::delete_selected(state, gui, selection);
            }

            ImGui::EndPopup();
        }
        ImGui::PopStyleVar(2);

        return clicked;
    }

    TreeNode *create_node_from_menu(State &state, GuiState &gui) {
        for (const auto &[type_id, mapper]: node_mappers_) {
            ImGui::PushID(type_id);
            if (ImGui::MenuItem(mapper.get_node_name(gui, state) )) {
                ImGui::PopID();
                TreeNode *new_node = TreeNode::create();
                new_node->created    = true;
                new_node->children_n = 0;
                new_node->id_node    = type_id;
                new_node->name       = "New node";
                return new_node;
            }
            ImGui::PopID();
        }
        return nullptr;
    }

    bool show_context_menu_element(State &state, GuiState &gui, TreeNode *node,
        const bool last, const bool nested, const bool selected)
    {
        if (node == nullptr)
            return false;

        static auto menu_id = "show_context_menu_element";
        static char name_buffer[64] = "";

        bool rename_popup = false;
        bool clicked      = false;

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup(menu_id);
            clicked = true;
        }
        else if (last) {
            ImGui::OpenPopup(menu_id);
        }

        const auto parent = node->parent;
        // ReSharper disable once CppDFANullDereference
        const bool is_grouped = (parent != nullptr) && (parent->parent != nullptr) && (parent->parent->is_group);

        if (ImGui::BeginPopup(menu_id)) {

            if (ImGui::BeginMenu("New")) {
                if (ImGui::BeginMenu("Add into")) {
                    if (const auto new_node = create_node_from_menu(state, gui))
                        tree::add_into_node(state, gui, node, new_node);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Add After")) {
                    if (const auto new_node = create_node_from_menu(state, gui))
                        tree::add_after_node(state, gui, node, new_node);
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Rename")) {
                std::memset(name_buffer, 0, sizeof(name_buffer));
                std::memcpy(name_buffer, node->name, std::strlen(node->name));
                rename_popup = true;
            }

            const bool enabled = !node->disabled;
            if (ImGui::MenuItem(enabled ? "Disable" : "Enable")) {
                if (enabled) tree::disable_node(state, gui, node);
                else tree::enable_node(state, gui, node);
            }

            if (ImGui::MenuItem("Group")) {
                selection_node selection;
                selection.selected = node;
                selection.root     = &selection;
                selection.next     = nullptr;
                selection.prev     = nullptr;
                tree::group_selected(state, gui, &selection);
            }

            if (is_grouped) {
                if (ImGui::MenuItem("Ungroup")) {
                    selection_node selection;
                    selection.selected = node;
                    selection.root     = &selection;
                    selection.next     = nullptr;
                    selection.prev     = nullptr;
                    tree::ungroup_selected(state, gui, &selection);
                }
            }

            if (!node->is_link) {
                if (ImGui::MenuItem("Inspect")) {
                    tree::pick_inspected_node(state, gui, node);
                }

                node_mappers_.at(node->id_node).draw_tree_menu(gui, state, node, nested, selected);

                if (ImGui::MenuItem("Duplicate", key_mappings().work_tree.duplicate.label)) {
                    tree::duplicate_node(state, gui, node);
                }

                if (ImGui::MenuItem("Cut", key_mappings().work_tree.cut.label)) {
                    tree::cut_node(state, gui, node);
                }

                if (ImGui::BeginMenu("Copy")) {

                    if (ImGui::MenuItem("Copy", key_mappings().work_tree.copy.label)) {
                        tree::copy_node(state, gui, node);
                    }

                    if (ImGui::MenuItem("Copy Ref", key_mappings().work_tree.reference.label)) {
                        tree::ref_node(state, gui, node);
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Paste", state.clipboard_node != nullptr)) {

                    if (ImGui::MenuItem("Paste after", key_mappings().work_tree.paste.label)) {
                        tree::paste_after_node(state, gui, node);
                    }

                    if (ImGui::MenuItem("Paste into", key_mappings().work_tree.insert.label)) {
                        tree::paste_into_node(state, gui, node);
                    }

                    ImGui::EndMenu();
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Delete")) {
                tree::delete_node(state, gui, node);
            }

            ImGui::EndPopup();
        }

        if (rename_popup) {
            ImGui::OpenPopup("rename-popup");
        }

        if (ImGui::BeginPopup("rename-popup")) {
            ImGui::Text("Edit name: ");
            ImGui::InputText("##edit", name_buffer, IM_ARRAYSIZE(name_buffer));
            if (ImGui::Button("Apply")) {
                tree::rename_node(state, gui, node, name_buffer);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        } else if (rename_popup) {
            ImGui::OpenPopup("rename-popup");
        }

        ImGui::PopStyleVar(2);
        return clicked;
    }

    bool show_context_menu_general(State &state, GuiState &gui) {
        static auto menu_id = "show_context_menu_general";

        bool clicked = false;

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));
        if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            selection_node::free(state.selected_nodes, true);
            state.selected_nodes = nullptr;
            ImGui::OpenPopup(menu_id);
            clicked = true;
        }

        if (ImGui::BeginPopup(menu_id)) {
            if (ImGui::BeginMenu("Add new node")) {
                if (const auto new_node = create_node_from_menu(state, gui))
                    tree::add_node(state, gui, new_node);
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

        ImGui::PopStyleVar(2);
        return clicked;
    }

}