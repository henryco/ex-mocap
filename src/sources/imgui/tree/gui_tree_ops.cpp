//
// Created by xd on 31/01/25.
//

// ReSharper disable CppDFAMemoryLeak
// ReSharper disable CppDFANullDereference

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
            auto &state = ex::logic::globals::state();
            rollback(&state);
        }

        void rollback(State *s) const {
            ex::entity::restore_entities(snap);
            s->logic.root_node   = root;
            s->logic.active_node = ID32_UNSET;
        }

        static Snapshot *create() {
            return create(ex::logic::globals::state());
        }

        static Snapshot *create(const State &state) {
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

    struct ParcelBox {
        GuiNodeParcel **parcel_ptr;

        GuiNodeParcel *parcel() const {
            return *parcel_ptr;
        }

        bool present() const {
            return parcel_ptr != nullptr && *parcel_ptr != nullptr;
        }

        void reset() {
            if (parcel_ptr && *parcel_ptr) {
                (*parcel_ptr)->free(*parcel_ptr);
                *parcel_ptr = nullptr;
            }
        }

        void reset(GuiNodeParcel *ptr) {
            reset();
            set(ptr);
        }

        void set(GuiNodeParcel *ptr) {
            *parcel_ptr = ptr;
        }

        static ParcelBox create() {
            ParcelBox box = { static_cast<GuiNodeParcel **>(::malloc(sizeof(void *))) };
            *box.parcel_ptr = nullptr;
            return box;
        }

        static void free(ParcelBox *box) {
            if (!box) return;
            free(*box);
        }

        static void free(ParcelBox &box) {
            if (!box.parcel_ptr)
                return;
            if (*box.parcel_ptr)
                (*box.parcel_ptr)->free(*box.parcel_ptr);
            ::free(box.parcel_ptr);
            box.parcel_ptr = nullptr;
        }
    };

    selection_node *bubble_sort_selection_(selection_node *selection) {
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

    struct id_mapping {
        id_type_32 was;
        id_type_32 now;
    };

    void deep_map_(const TreeNode *node, data::array<id_mapping> *id_array) {
        if (node == nullptr)
            return;

        if (id_array != nullptr)
            id_array->push({ node->id_node, node->id_node });

        const auto children = ex::entity::children(node->id_node);
        for (int i = 0; i < children.size; ++i)
            deep_map_(ex::entity::find(children.data[i]), id_array);
    }

    void copy_(const TreeNode *head, const id_type_32 id_parent, data::array<id_mapping> *id_array, const int index = -1) {
        if (head == nullptr)
            return;

        TreeNode *node = ex::entity::create();
        node->id_link  = head->id_link;
        node->id_type  = head->id_type;
        node->state    = head->state;

        if (head->name != nullptr)
            node->name = (new std::string(head->name))->append(" (copy)").c_str();
        else
            node->name = "";

        if (id_array != nullptr)
            id_array->push({ head->id_node, node->id_node });

        const auto children = ex::entity::children(head->id_node);
        for (int i = 0; i < children.size; ++i) {
            const id_type_32 parent = node->id_node;
            const TreeNode  *child  = ex::entity::find(children.data[i]);
            copy_(child, parent, id_array);
        }

        ex::entity::add_child(id_parent, node->id_node, index);

        const auto mapper     = i_::mappers_.at_or_default(head->id_type);
        const auto store_fn   = mapper.store;
        const auto restore_fn = mapper.restore;
        if (store_fn && restore_fn)
            restore_fn(node, store_fn(head));
    }

    void del_(const TreeNode *node) {
        if (node == nullptr)
            return;

        const id_type_32 node_id   = node->id_node;
        const id_type_32 parent_id = node->id_parent;

        ex::entity::remove(node_id);

        const auto children = ex::entity::children(parent_id);
        if (children.size > 0)
            return;
        if (TreeNode *parent = ex::entity::find(parent_id)) {
            if (parent->state >>= node_state::section)
                return;
            if (!(parent->state >>= node_state::group))
                return;
            if (!ex::entity::find(parent->id_parent))
                return;
            ex::entity::remove(parent_id);
        }
    }

    void move_(TreeNode *node, const id_type_32 id_target, data::array<id_mapping> *id_array, const int index = -1) {
        if (node == nullptr)
            return;

        const TreeNode node_0 = *node;

        const id_type_32 old_parent_id = node->id_parent;

        int idx_fix = 0;
        if (old_parent_id == id_target) {
            if (ex::entity::index_of(node->id_node) < index)
                idx_fix = -1;
        }

        ex::entity::del_child(node->id_parent, node->id_node);
        ex::entity::add_child(id_target, node->id_node, index + idx_fix);

        const auto mapper = i_::mappers_.at_or_default(node->id_type);
        if (const auto &upd_fn = mapper.update_tree_node)
            upd_fn(&node_0, node);

        if (id_array != nullptr)
            id_array->push({node->id_node, node->id_node});

        if (old_parent_id == id_target)
            return;

        if (ex::entity::children(old_parent_id).size > 0)
            return;

        if (TreeNode *parent = ex::entity::find(old_parent_id)) {
            if (parent->state >>= node_state::section)
                return;
            if (!(parent->state >>= node_state::group))
                return;
            if (!ex::entity::find(parent->id_parent))
                return;
            ex::entity::remove(old_parent_id);
        }
    }

    void state_(TreeNode *node, const node_state state, const bool add, ex::data::array<id_mapping> *id_array) {
        if (node == nullptr)
            return;

        const TreeNode before = *node;

        if (add) node->state |= state;
        else     node->state -= state;

        if (id_array != nullptr)
            id_array->push({ node->id_node, node->id_node });

        const id_type_16 state_b = static_cast<id_type_16>(before.state);
        const id_type_16 state_a = static_cast<id_type_16>(node->state);

        if (state_b != state_a) {
            if (const auto &upd_fn = i_::mappers_.at_or_default(node->id_type).update_tree_node)
                upd_fn(&before, node);
        }

        const auto children = ex::entity::children(node->id_node);
        for (int i = 0; i < children.size; ++i)
            state_(ex::entity::find(children.data[i]), state, add, id_array);
    }

    void link_(const TreeNode *head, const id_type_32 id_target, data::array<id_mapping> *id_array, const int index = -1) {
        if (head == nullptr)
            return;

        TreeNode *node = ex::entity::create();
        node->id_type  = head->id_type;
        node->id_link  = head->id_node;
        node->state    = node_state::link;

        if (head->name != nullptr)
            node->name = (new std::string(head->name))->c_str();
        else
            node->name = "";

        ex::entity::add_child(id_target, node->id_node, index);

        if (id_array != nullptr)
            id_array->push({node->id_node, node->id_node});
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
        Snapshot *before = Snapshot::create(state);

        TreeNode *created = ex::entity::create();
        created->id_type  = add->id_type;
        created->id_link  = add->id_link;
        created->state    = add->state;

        if (add->name != nullptr)
            created->name = (new std::string(add->name))->c_str();
        else
            created->name = "";

        const id_type_32 id_node = created->id_node;
        state.logic.active_node  = id_node;

        const int index = (add->state >>= node_state::section) ? (0) : (-1);

        ex::entity::add_child(state.logic.root_node, id_node, index);

        const auto after = Snapshot::create();
        ParcelBox box = ParcelBox::create();

        undo_history().push(strings::gui_tree_ops_history_add,
        new std::function([before, id_node, box] (State *) mutable {
            TreeState::reset(tree::state());

            const TreeNode *node   = ex::entity::find(id_node);
            if (const auto store_fn = i_::mappers_.at_or_default(node->id_type).store)
                box.reset(store_fn(node));

            before->rollback();
        }),
        new std::function([after, id_node, box] (State *) {
            TreeState::reset(tree::state());
            after->rollback();

            TreeNode *new_node = ex::entity::find(id_node);
            if (box.present()) {
                if (const auto restore_fn = i_::mappers_.at_or_default(new_node->id_type).restore)
                    restore_fn(new_node, box.parcel());
            }
        }),
        new std::function([before, after, box] () mutable {
            Snapshot::free(before);
            Snapshot::free(after);
            ParcelBox::free(box);
        }));
    }

    void paste_node() {

        const auto &tree_state = tree::state();
        if (tree_state.buffer == nullptr) {
            TreeState::reset(tree::state());
            return;
        }

        State &state = ex::logic::globals::state();

        const auto selection = bubble_sort_selection_(tree_state.buffer->selection->root);
        const bool move      = tree_state.buffer->move;
        const bool ref       = tree_state.buffer->ref;

        TreeState::reset(tree::state());
        ex::data::array<id_mapping> id_array;
        ex::data::array<ParcelBox>  parcels;

        const auto before = Snapshot::create(state);

        if (ref) {
            for (selection_node *head = selection; head != nullptr; head = head->next)
                link_(ex::entity::find(head->selected->id_node), state.logic.root_node, &id_array);
        }
        else if (move) {
            for (selection_node *head = selection; head != nullptr; head = head->next)
                move_(ex::entity::find(head->selected->id_node), state.logic.root_node, &id_array);
        }
        else {
            for (selection_node *head = selection; head != nullptr; head = head->next)
                copy_(ex::entity::find(head->selected->id_node), state.logic.root_node, &id_array);

            for (int i = 0; i < id_array.size; ++i)
                parcels.push(ParcelBox::create());
        }

        state.logic.active_node = id_array[0].now;

        selection_node::free(selection);
        const auto after = Snapshot::create(state);

        undo_history().push(strings::gui_tree_ops_history_paste,
        new std::function([before, id_array, move, ref, parcels] (ex::State *) mutable {
            TreeState::reset(tree::state());

            if (move) {
                ex::data::array<TreeNode> before_array;
                for (int i = 0; i < id_array.size; ++i) {
                    before_array.push(*ex::entity::find(id_array[i].now));
                }
                before->rollback();
                for (int i = 0; i < id_array.size; ++i) {
                    TreeNode  *node   = ex::entity::find(id_array[i].now);
                    const auto mapper = i_::mappers_.at_or_default(node->id_type);
                    if (const auto &upd_fn = mapper.update_tree_node)
                        upd_fn(&before_array[i], node);
                }
                return;
            }

            if (!ref) { // copy
                for (int i = 0; i < id_array.size; ++i) {
                    TreeNode *node_1 = ex::entity::find(id_array[i].now);
                    ParcelBox box = parcels[i];

                    const auto mapper = i_::mappers_.at_or_default(node_1->id_type);
                    if (const auto store_fn = mapper.store)
                        box.reset(store_fn(node_1));
                }
            }

            before->rollback();
        }),
        new std::function([after, id_array, move, ref, parcels] (ex::State *) mutable {
            TreeState::reset(tree::state());

            if (move) {
                ex::data::array<TreeNode> before_array;
                for (int i = 0; i < id_array.size; ++i) {
                    before_array.push(*ex::entity::find(id_array[i].now));
                }
                after->rollback();
                for (int i = 0; i < id_array.size; ++i) {
                    TreeNode  *node   = ex::entity::find(id_array[i].now);
                    const auto mapper = i_::mappers_.at_or_default(node->id_type);
                    if (const auto &upd_fn = mapper.update_tree_node)
                        upd_fn(&before_array[i], node);
                }
                return;
            }

            if (!ref) { // copy
                after->rollback();

                for (int i = 0; i < id_array.size; ++i) {
                    TreeNode *node_1 = ex::entity::find(id_array[i].now);
                    ParcelBox box = parcels[i];
                    const auto mapper = i_::mappers_.at_or_default(node_1->id_type);
                    const auto restore_fn = mapper.restore;
                    if (box.present() && restore_fn)
                        restore_fn(node_1, box.parcel());
                }
                return;
            }

            after->rollback();
        }),
        new std::function([before, after, parcels] () mutable {
            Snapshot::free(before);
            Snapshot::free(after);
            for (int i = 0; i < parcels.size; ++i)
                ParcelBox::free(parcels[i]);
        }));
    }

    void add_into_node(TreeNode *node, TreeNode *add) {
        State &state = ex::logic::globals::state();

        TreeState::reset(tree::state());
        const auto before = Snapshot::create(state);

        TreeNode *created  = ex::entity::create();
        created->id_type   = add->id_type;
        created->id_link   = add->id_link;
        created->state     = add->state;

        if (add->name != nullptr)
            created->name = (new std::string(add->name))->c_str();
        else
            created->name = "";

        state.logic.active_node = created->id_node;

        TreeNode *parent = nullptr;
        if (node->state >>= node_state::section) {
            parent = node;
        } else {
            entity::IdArray children_1 = ex::entity::children(node->id_node);
            if (children_1.data == nullptr || children_1.size <= 0) {
                TreeNode *group = ex::entity::create();
                group->name  = "generic_container";
                group->state = node_state::group;
                ex::entity::add_child(node->id_node, group->id_node);
            }
            entity::IdArray children_2 = ex::entity::children(node->id_node);
            parent                     = ex::entity::find(children_2.data[0]);
        }

        const id_type_32 id_node    = created->id_node;
        ex::entity::add_child(parent->id_node, id_node);

        ParcelBox box = ParcelBox::create();

        const auto after = Snapshot::create(state);

        undo_history().push(strings::gui_tree_ops_history_add_into,
        new std::function([before, id_node, box] (ex::State *s) mutable {
            TreeState::reset(tree::state());

            const TreeNode *new_node   = ex::entity::find(id_node);
            if (const auto store_fn = i_::mappers_.at_or_default(new_node->id_type).store)
                box.reset(store_fn(new_node));

            before->rollback(s);
        }),
        new std::function([after, id_node, box] (ex::State *s) {
            TreeState::reset(tree::state());
            after->rollback(s);

            TreeNode *new_node = ex::entity::find(id_node);
            if (box.present()) {
                if (const auto restore_fn = i_::mappers_.at_or_default(new_node->id_type).restore)
                    restore_fn(new_node, box.parcel());
            }
        }),
        new std::function([before, after, box] () mutable {
            Snapshot::free(before);
            Snapshot::free(after);
            ParcelBox::free(box);
        }));
    }

    void add_after_node(TreeNode *node, TreeNode *add) {
        State &state = ex::logic::globals::state();

        TreeState::reset(tree::state());
        const auto before = Snapshot::create(state);

        TreeNode *created  = ex::entity::create();
        created->id_type   = add->id_type;
        created->id_link   = add->id_link;
        created->state     = add->state;

        if (add->name != nullptr)
            created->name = (new std::string(add->name))->c_str();
        else
            created->name = "";

        state.logic.active_node = created->id_node;

        const int        index   = ex::entity::index_of(node->id_node);
        const id_type_32 id_node = created->id_node;

        ex::entity::add_child(node->id_parent, created->id_node, index + 1);

        ParcelBox box = ParcelBox::create();
        const auto after = Snapshot::create();

        undo_history().push(strings::gui_tree_ops_history_add_after,
        new std::function([before, id_node, box] (ex::State *) mutable {
            TreeState::reset(tree::state());

            const TreeNode *new_node   = ex::entity::find(id_node);
            if (const auto store_fn = i_::mappers_.at_or_default(new_node->id_type).store)
                box.reset(store_fn(new_node));

            before->rollback();
        }),
        new std::function([after, id_node, box] (ex::State *) {
            TreeState::reset(tree::state());
            after->rollback();

            TreeNode *new_node = ex::entity::find(id_node);
            if (box.present()) {
                if (const auto restore_fn = i_::mappers_.at_or_default(new_node->id_type).restore)
                    restore_fn(new_node, box.parcel());
            }
        }),
        new std::function([before, after, box] () mutable {
            Snapshot::free(before);
            Snapshot::free(after);
            ParcelBox::free(box);
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

        const auto &upd_fn = i_::mappers_.at_or_default(node->id_type).update_tree_node;
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
        if (node->state >>= node_state::link)
            return;

        TreeState::reset(tree::state());

        State &state = ex::logic::globals::state();
        const auto before = Snapshot::create(state);

        ex::data::array<id_mapping> id_array;
        ex::data::array<ParcelBox>  parcels;

        const int index = ex::entity::index_of(node->id_node);
        copy_(node, node->id_parent, &id_array, index + 1); // specify index?
        for (int i = 0; i < id_array.size; ++i)
            parcels.push(ParcelBox::create());

        state.logic.active_node = id_array[0].now;

        const auto after = Snapshot::create();

        undo_history().push(strings::gui_tree_ops_history_node_duplicated,
        new std::function([before, id_array, parcels] (ex::State *) mutable {
            TreeState::reset(tree::state());

            for (int i = 0; i < id_array.size; ++i) {
                TreeNode *node_1 = ex::entity::find(id_array[i].now);
                ParcelBox box = parcels[i];

                const auto mapper = i_::mappers_.at_or_default(node_1->id_type);
                if (const auto store_fn = mapper.store)
                    box.reset(store_fn(node_1));
            }

            before->rollback();
        }),

        new std::function([after, id_array, parcels] (ex::State *) mutable {
            TreeState::reset(tree::state());
            after->rollback();

            for (int i = 0; i < id_array.size; ++i) {
                TreeNode *node_1 = ex::entity::find(id_array[i].now);
                ParcelBox box = parcels[i];
                const auto mapper = i_::mappers_.at_or_default(node_1->id_type);
                const auto restore_fn = mapper.restore;
                if (box.present() && restore_fn)
                    restore_fn(node_1, box.parcel());
            }
        }),

        new std::function([after, before, parcels] () mutable {
            Snapshot::free(before);
            Snapshot::free(after);
            for (int i = 0; i < parcels.size; ++i)
                ParcelBox::free(parcels[i]);
        }));
    }

    void paste_after_node(TreeNode *node_before) {
        const auto &tree_state = tree::state();
        if (tree_state.buffer == nullptr) {
            TreeState::reset(tree::state());
            return;
        }

        State &state = ex::logic::globals::state();

        const auto selection = bubble_sort_selection_(tree_state.buffer->selection->root);

        // self-referencing safety check
        for (selection_node *head = selection; head != nullptr; head = head->next) {
            if (ex::entity::is_within(head->selected->id_node, node_before->id_node)) {
                selection_node::free(selection);
                return;
            }
        }

        const bool move      = tree_state.buffer->move;
        const bool ref       = tree_state.buffer->ref;

        TreeState::reset(tree::state());
        ex::data::array<id_mapping> id_array;
        ex::data::array<ParcelBox>  parcels;

        const int  index  = ex::entity::index_of(node_before->id_node);
        const auto before = Snapshot::create(state);

        if (ref) {
            int i = 1;
            for (selection_node *head = selection; head != nullptr; head = head->next)
                link_(ex::entity::find(head->selected->id_node), node_before->id_parent, &id_array, index + (i++));
        }
        else if (move) {
            int i = 1;
            for (selection_node *head = selection; head != nullptr; head = head->next)
                move_(ex::entity::find(head->selected->id_node), node_before->id_parent, &id_array, index + (i++));
        }
        else {
            int j = 1;
            for (selection_node *head = selection; head != nullptr; head = head->next)
                copy_(ex::entity::find(head->selected->id_node), node_before->id_parent, &id_array, index + (j++));

            for (int i = 0; i < id_array.size; ++i)
                parcels.push(ParcelBox::create());
        }

        state.logic.active_node = id_array[0].now;

        selection_node::free(selection);
        const auto after = Snapshot::create(state);

        undo_history().push(strings::gui_tree_ops_history_paste_after,
        new std::function([before, id_array, move, ref, parcels] (ex::State *) mutable {
            TreeState::reset(tree::state());

            if (move) {
                ex::data::array<TreeNode> before_array;
                for (int i = 0; i < id_array.size; ++i) {
                    before_array.push(*ex::entity::find(id_array[i].now));
                }
                before->rollback();
                for (int i = 0; i < id_array.size; ++i) {
                    TreeNode  *node   = ex::entity::find(id_array[i].now);
                    const auto mapper = i_::mappers_.at_or_default(node->id_type);
                    if (const auto &upd_fn = mapper.update_tree_node)
                        upd_fn(&before_array[i], node);
                }
                return;
            }

            if (!ref) { // copy
                for (int i = 0; i < id_array.size; ++i) {
                    TreeNode *node_1 = ex::entity::find(id_array[i].now);
                    ParcelBox box = parcels[i];

                    const auto mapper = i_::mappers_.at_or_default(node_1->id_type);
                    if (const auto store_fn = mapper.store)
                        box.reset(store_fn(node_1));
                }
            }

            before->rollback();
        }),
        new std::function([after, id_array, move, ref, parcels] (ex::State *) mutable {
            TreeState::reset(tree::state());

            if (move) {
                ex::data::array<TreeNode> before_array;
                for (int i = 0; i < id_array.size; ++i) {
                    before_array.push(*ex::entity::find(id_array[i].now));
                }
                after->rollback();
                for (int i = 0; i < id_array.size; ++i) {
                    TreeNode  *node   = ex::entity::find(id_array[i].now);
                    const auto mapper = i_::mappers_.at_or_default(node->id_type);
                    if (const auto &upd_fn = mapper.update_tree_node)
                        upd_fn(&before_array[i], node);
                }
                return;
            }

            if (!ref) { // copy
                after->rollback();

                for (int i = 0; i < id_array.size; ++i) {
                    TreeNode *node_1 = ex::entity::find(id_array[i].now);
                    ParcelBox box = parcels[i];
                    const auto mapper = i_::mappers_.at_or_default(node_1->id_type);
                    const auto restore_fn = mapper.restore;
                    if (box.present() && restore_fn)
                        restore_fn(node_1, box.parcel());
                }
                return;
            }

            after->rollback();
        }),
        new std::function([before, after, parcels] () mutable {
            Snapshot::free(before);
            Snapshot::free(after);
            for (int i = 0; i < parcels.size; ++i)
                ParcelBox::free(parcels[i]);
        }));
    }

    void paste_into_node(TreeNode *node_into) {
        const auto &tree_state = tree::state();
        if (tree_state.buffer == nullptr) {
            TreeState::reset(tree::state());
            return;
        }

        State &state = ex::logic::globals::state();

        const auto selection = bubble_sort_selection_(tree_state.buffer->selection->root);

        // self-referencing safety check
        for (selection_node *head = selection; head != nullptr; head = head->next) {
            if (ex::entity::is_within(head->selected->id_node, node_into->id_node)) {
                selection_node::free(selection);
                return;
            }
        }

        const bool move      = tree_state.buffer->move;
        const bool ref       = tree_state.buffer->ref;

        TreeState::reset(tree::state());
        ex::data::array<id_mapping> id_array;
        ex::data::array<ParcelBox>  parcels;

        const auto before = Snapshot::create(state);

        TreeNode *parent = nullptr;
        if (node_into->state >>= node_state::section) {
            parent = node_into;
        } else {
            entity::IdArray children_1 = ex::entity::children(node_into->id_node);
            if (children_1.data == nullptr || children_1.size <= 0) {
                TreeNode *group = ex::entity::create();
                group->name  = "generic_container";
                group->state = node_state::group;
                ex::entity::add_child(node_into->id_node, group->id_node);
            }
            entity::IdArray children_2 = ex::entity::children(node_into->id_node);
            parent                     = ex::entity::find(children_2.data[0]);
        }

        if (ref) {
            for (selection_node *head = selection; head != nullptr; head = head->next)
                link_(ex::entity::find(head->selected->id_node), parent->id_node, &id_array);
        }
        else if (move) {
            for (selection_node *head = selection; head != nullptr; head = head->next)
                move_(ex::entity::find(head->selected->id_node), parent->id_node, &id_array);
        }
        else {
            for (selection_node *head = selection; head != nullptr; head = head->next)
                copy_(ex::entity::find(head->selected->id_node), parent->id_node, &id_array);

            for (int i = 0; i < id_array.size; ++i)
                parcels.push(ParcelBox::create());
        }

        state.logic.active_node = id_array[0].now;

        selection_node::free(selection);
        const auto after = Snapshot::create(state);

        undo_history().push(strings::gui_tree_ops_history_paste_into,
        new std::function([before, id_array, move, ref, parcels] (ex::State *) mutable {
            TreeState::reset(tree::state());

            if (move) {
                ex::data::array<TreeNode> before_array;
                for (int i = 0; i < id_array.size; ++i) {
                    before_array.push(*ex::entity::find(id_array[i].now));
                }
                before->rollback();
                for (int i = 0; i < id_array.size; ++i) {
                    TreeNode  *node   = ex::entity::find(id_array[i].now);
                    const auto mapper = i_::mappers_.at_or_default(node->id_type);
                    if (const auto &upd_fn = mapper.update_tree_node)
                        upd_fn(&before_array[i], node);
                }
                return;
            }

            if (!ref) { // copy
                for (int i = 0; i < id_array.size; ++i) {
                    TreeNode *node_1 = ex::entity::find(id_array[i].now);
                    ParcelBox box = parcels[i];

                    const auto mapper = i_::mappers_.at_or_default(node_1->id_type);
                    if (const auto store_fn = mapper.store)
                        box.reset(store_fn(node_1));
                }
            }

            before->rollback();
        }),
        new std::function([after, id_array, move, ref, parcels] (ex::State *) mutable {
            TreeState::reset(tree::state());

            if (move) {
                ex::data::array<TreeNode> before_array;
                for (int i = 0; i < id_array.size; ++i) {
                    before_array.push(*ex::entity::find(id_array[i].now));
                }
                after->rollback();
                for (int i = 0; i < id_array.size; ++i) {
                    TreeNode  *node   = ex::entity::find(id_array[i].now);
                    const auto mapper = i_::mappers_.at_or_default(node->id_type);
                    if (const auto &upd_fn = mapper.update_tree_node)
                        upd_fn(&before_array[i], node);
                }
                return;
            }

            if (!ref) { // copy
                after->rollback();

                for (int i = 0; i < id_array.size; ++i) {
                    TreeNode *node_1 = ex::entity::find(id_array[i].now);
                    ParcelBox box = parcels[i];
                    const auto mapper = i_::mappers_.at_or_default(node_1->id_type);
                    const auto restore_fn = mapper.restore;
                    if (box.present() && restore_fn)
                        restore_fn(node_1, box.parcel());
                }
                return;
            }

            after->rollback();
        }),
        new std::function([before, after, parcels] () mutable {
            Snapshot::free(before);
            Snapshot::free(after);
            for (int i = 0; i < parcels.size; ++i)
                ParcelBox::free(parcels[i]);
        }));
    }

    void move_after_node(TreeNode *node_before, TreeNode *move) {
        if (node_before == nullptr || move == nullptr)
            return;

        // self-referencing safety check
        if (ex::entity::is_within(move->id_node, node_before->id_node))
            return;

        State &state = ex::logic::globals::state();

        TreeState::reset(tree::state());
        ex::data::array<id_mapping> id_array;

        const int  index  = ex::entity::index_of(node_before->id_node);
        const auto before = Snapshot::create(state);

        move_(ex::entity::find(move->id_node), node_before->id_parent, &id_array, index + 1);

        state.logic.active_node = id_array[0].now;

        const auto after = Snapshot::create(state);

        undo_history().push(strings::gui_tree_ops_history_move_after,
        new std::function([before, id_array] (ex::State *) mutable {
            TreeState::reset(tree::state());
            ex::data::array<TreeNode> before_array;
            for (int i = 0; i < id_array.size; ++i) {
                before_array.push(*ex::entity::find(id_array[i].now));
            }
            before->rollback();
            for (int i = 0; i < id_array.size; ++i) {
                TreeNode  *node   = ex::entity::find(id_array[i].now);
                const auto mapper = i_::mappers_.at_or_default(node->id_type);
                if (const auto &upd_fn = mapper.update_tree_node)
                    upd_fn(&before_array[i], node);
            }
        }),
        new std::function([after, id_array] (ex::State *) mutable {
            TreeState::reset(tree::state());
            ex::data::array<TreeNode> before_array;
            for (int i = 0; i < id_array.size; ++i) {
                before_array.push(*ex::entity::find(id_array[i].now));
            }
            after->rollback();
            for (int i = 0; i < id_array.size; ++i) {
                TreeNode  *node   = ex::entity::find(id_array[i].now);
                const auto mapper = i_::mappers_.at_or_default(node->id_type);
                if (const auto &upd_fn = mapper.update_tree_node)
                    upd_fn(&before_array[i], node);
            }
        }),
        new std::function([before, after] () mutable {
            Snapshot::free(before);
            Snapshot::free(after);
        }));
    }

    void move_into_node(TreeNode *node_into, TreeNode *move) {
        if (node_into == nullptr || move == nullptr)
            return;

        // self-referencing safety check
        if (ex::entity::is_within(move->id_node, node_into->id_node))
            return;

        State &state = ex::logic::globals::state();

        TreeState::reset(tree::state());
        ex::data::array<id_mapping> id_array;

        const auto before = Snapshot::create(state);

        TreeNode *parent = nullptr;
        if (node_into->state >>= node_state::section) {
            parent = node_into;
        } else {
            entity::IdArray children_1 = ex::entity::children(node_into->id_node);
            if (children_1.data == nullptr || children_1.size <= 0) {
                TreeNode *group = ex::entity::create();
                group->name  = "generic_container";
                group->state = node_state::group;
                ex::entity::add_child(node_into->id_node, group->id_node);
            }
            entity::IdArray children_2 = ex::entity::children(node_into->id_node);
            parent                     = ex::entity::find(children_2.data[0]);
        }

        move_(ex::entity::find(move->id_node), parent->id_node, &id_array);

        state.logic.active_node = id_array[0].now;

        const auto after = Snapshot::create(state);

        undo_history().push(strings::gui_tree_ops_history_move_into,
        new std::function([before, id_array] (ex::State *) mutable {
            TreeState::reset(tree::state());
            ex::data::array<TreeNode> before_array;
            for (int i = 0; i < id_array.size; ++i) {
                before_array.push(*ex::entity::find(id_array[i].now));
            }
            before->rollback();
            for (int i = 0; i < id_array.size; ++i) {
                TreeNode  *node   = ex::entity::find(id_array[i].now);
                const auto mapper = i_::mappers_.at_or_default(node->id_type);
                if (const auto &upd_fn = mapper.update_tree_node)
                    upd_fn(&before_array[i], node);
            }
        }),
        new std::function([after, id_array] (ex::State *) mutable {
            TreeState::reset(tree::state());
            ex::data::array<TreeNode> before_array;
            for (int i = 0; i < id_array.size; ++i) {
                before_array.push(*ex::entity::find(id_array[i].now));
            }
            after->rollback();
            for (int i = 0; i < id_array.size; ++i) {
                TreeNode  *node   = ex::entity::find(id_array[i].now);
                const auto mapper = i_::mappers_.at_or_default(node->id_type);
                if (const auto &upd_fn = mapper.update_tree_node)
                    upd_fn(&before_array[i], node);
            }
        }),
        new std::function([before, after] () mutable {
            Snapshot::free(before);
            Snapshot::free(after);
        }));
    }

    void group_selected(selection_node *selection) {
        selection_node  *sorted    = bubble_sort_selection_(selection->root);
        const TreeNode  *root_node = selection->root->selected;
        const id_type_32 parent_id = root_node->id_parent;

        TreeState::reset(tree::state());
        const auto before = Snapshot::create();

        TreeNode *new_group = ex::entity::create();
        new_group->state    = node_state::group;
        new_group->name     = "generic_group";

        ex::entity::add_child(parent_id, new_group->id_node, ex::entity::index_of(root_node->id_node));

        ex::data::array<id_mapping> id_array;
        for (selection_node *head = sorted; head != nullptr; head = head->next)
            move_(ex::entity::find(head->selected->id_node), new_group->id_node, &id_array);

        selection_node::free(sorted);
        const auto after = Snapshot::create();

        undo_history().push(strings::gui_tree_ops_history_group,
        new std::function([before, id_array] (ex::State *) mutable {
            TreeState::reset(tree::state());
            ex::data::array<TreeNode> before_array;
            for (int i = 0; i < id_array.size; ++i) {
                before_array.push(*ex::entity::find(id_array[i].now));
            }
            before->rollback();
            for (int i = 0; i < id_array.size; ++i) {
                TreeNode  *node   = ex::entity::find(id_array[i].now);
                const auto mapper = i_::mappers_.at_or_default(node->id_type);
                if (const auto &upd_fn = mapper.update_tree_node)
                    upd_fn(&before_array[i], node);
            }
        }),
        new std::function([after, id_array] (ex::State *) mutable {
            TreeState::reset(tree::state());
            ex::data::array<TreeNode> before_array;
            for (int i = 0; i < id_array.size; ++i) {
                before_array.push(*ex::entity::find(id_array[i].now));
            }
            after->rollback();
            for (int i = 0; i < id_array.size; ++i) {
                TreeNode  *node   = ex::entity::find(id_array[i].now);
                const auto mapper = i_::mappers_.at_or_default(node->id_type);
                if (const auto &upd_fn = mapper.update_tree_node)
                    upd_fn(&before_array[i], node);
            }
        }),
        new std::function([before, after] () mutable {
            Snapshot::free(after);
            Snapshot::free(before);
        }));
    }

    void ungroup_selected(selection_node *selection) {
        selection_node  *sorted      = bubble_sort_selection_(selection->root);
        TreeNode        *root_node   = selection->root->selected;
        TreeNode        *parent_node = ex::entity::find(root_node->id_parent);
        const int        parent_idx  = ex::entity::index_of(parent_node->id_node);
        const id_type_32 grand_id    = parent_node->id_parent;

        TreeState::reset(tree::state());
        const auto before = Snapshot::create();

        ex::data::array<id_mapping> id_array;
        if (!(parent_node->state >>= node_state::section)) {
            int j = 0;
            for (selection_node *head = sorted; head != nullptr; head = head->next)
                move_(ex::entity::find(head->selected->id_node), grand_id, &id_array, parent_idx + (j++));
        } else {
            for (selection_node *head = sorted; head != nullptr; head = head->next)
                move_(ex::entity::find(head->selected->id_node), grand_id, &id_array);
        }

        selection_node::free(sorted);
        const auto after = Snapshot::create();

        undo_history().push(strings::gui_tree_ops_history_ungroup,
        new std::function([before, id_array] (ex::State *) mutable {
            TreeState::reset(tree::state());
            ex::data::array<TreeNode> before_array;
            for (int i = 0; i < id_array.size; ++i) {
                before_array.push(*ex::entity::find(id_array[i].now));
            }
            before->rollback();
            for (int i = 0; i < id_array.size; ++i) {
                TreeNode  *node   = ex::entity::find(id_array[i].now);
                const auto mapper = i_::mappers_.at_or_default(node->id_type);
                if (const auto &upd_fn = mapper.update_tree_node)
                    upd_fn(&before_array[i], node);
            }
        }),
        new std::function([after, id_array] (ex::State *) mutable {
            TreeState::reset(tree::state());
            ex::data::array<TreeNode> before_array;
            for (int i = 0; i < id_array.size; ++i) {
                before_array.push(*ex::entity::find(id_array[i].now));
            }
            after->rollback();
            for (int i = 0; i < id_array.size; ++i) {
                TreeNode  *node   = ex::entity::find(id_array[i].now);
                const auto mapper = i_::mappers_.at_or_default(node->id_type);
                if (const auto &upd_fn = mapper.update_tree_node)
                    upd_fn(&before_array[i], node);
            }
        }),
        new std::function([before, after] () mutable {
            Snapshot::free(after);
            Snapshot::free(before);
        }));
    }

    void enable_node(TreeNode *node) {
        TreeState::reset(tree::state());
        const auto before = Snapshot::create();

        ex::data::array<id_mapping> id_array;
        state_(node, node_state::disabled, false, &id_array);
        ex::entity::set_dirty(true);

        const auto after = Snapshot::create();

        undo_history().push(strings::gui_tree_ops_history_node_enabled,
        new std::function([before, id_array] (ex::State *) {
            TreeState::reset(tree::state());
            ex::data::array<TreeNode> before_array;
            for (int i = 0; i < id_array.size; ++i) {
                before_array.push(*ex::entity::find(id_array[i].now));
            }
            before->rollback();
            for (int i = 0; i < id_array.size; ++i) {
                TreeNode  *node_0 = ex::entity::find(id_array[i].now);
                const auto mapper = i_::mappers_.at_or_default(node_0->id_type);
                if (const auto &upd_fn = mapper.update_tree_node)
                    upd_fn(&before_array[i], node_0);
            }
        }),
        new std::function([after, id_array] (ex::State *) {
            TreeState::reset(tree::state());
            ex::data::array<TreeNode> before_array;
            for (int i = 0; i < id_array.size; ++i) {
                before_array.push(*ex::entity::find(id_array[i].now));
            }
            after->rollback();
            for (int i = 0; i < id_array.size; ++i) {
                TreeNode  *node_0 = ex::entity::find(id_array[i].now);
                const auto mapper = i_::mappers_.at_or_default(node_0->id_type);
                if (const auto &upd_fn = mapper.update_tree_node)
                    upd_fn(&before_array[i], node_0);
            }
        }),
        new std::function([before, after] () mutable {
            Snapshot::free(after);
            Snapshot::free(before);
        }));
    }

    void disable_node(TreeNode *node) {
        TreeState::reset(tree::state());
        const auto before = Snapshot::create();

        ex::data::array<id_mapping> id_array;
        state_(node, node_state::disabled, true, &id_array);
        ex::entity::set_dirty(true);

        const auto after = Snapshot::create();

        undo_history().push(strings::gui_tree_ops_history_node_disabled,
        new std::function([before, id_array] (ex::State *) {
            TreeState::reset(tree::state());
            ex::data::array<TreeNode> before_array;
            for (int i = 0; i < id_array.size; ++i) {
                before_array.push(*ex::entity::find(id_array[i].now));
            }
            before->rollback();
            for (int i = 0; i < id_array.size; ++i) {
                TreeNode  *node_0 = ex::entity::find(id_array[i].now);
                const auto mapper = i_::mappers_.at_or_default(node_0->id_type);
                if (const auto &upd_fn = mapper.update_tree_node)
                    upd_fn(&before_array[i], node_0);
            }
        }),
        new std::function([after, id_array] (ex::State *) {
            TreeState::reset(tree::state());
            ex::data::array<TreeNode> before_array;
            for (int i = 0; i < id_array.size; ++i) {
                before_array.push(*ex::entity::find(id_array[i].now));
            }
            after->rollback();
            for (int i = 0; i < id_array.size; ++i) {
                TreeNode  *node_0 = ex::entity::find(id_array[i].now);
                const auto mapper = i_::mappers_.at_or_default(node_0->id_type);
                if (const auto &upd_fn = mapper.update_tree_node)
                    upd_fn(&before_array[i], node_0);
            }
        }),
        new std::function([before, after] () mutable {
            Snapshot::free(after);
            Snapshot::free(before);
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

            if (const auto &upd_fn = i_::mappers_.at_or_default(head->selected->id_type).update_tree_node)
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

                if (const auto &upd_fn = i_::mappers_.at_or_default(f_node->id_type).update_tree_node)
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

                if (const auto &upd_fn = i_::mappers_.at_or_default(f_node->id_type).update_tree_node)
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

            if (const auto &upd_fn = i_::mappers_.at_or_default(head->selected->id_type).update_tree_node)
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

                if (const auto &upd_fn = i_::mappers_.at_or_default(f_node->id_type).update_tree_node)
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

                if (const auto &upd_fn = i_::mappers_.at_or_default(f_node->id_type).update_tree_node)
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

        const auto before = Snapshot::create();
        selection_node::free(copy_selection);

        ex::data::array<id_mapping> id_array;
        ex::data::array<ParcelBox>  parcels;

        for (int k = 0; k < selections.size; ++k) {
            TreeNode *node = ex::entity::find(selections[k]);

            ex::data::array<id_mapping> id_array_2;
            deep_map_(node, &id_array_2);

            for (int i = 0; i < id_array_2.size; ++i) {
                TreeNode *node_1 = ex::entity::find(id_array_2[i].now);
                ParcelBox box = ParcelBox::create();
                const auto mapper = i_::mappers_.at_or_default(node_1->id_type);
                if (const auto store_fn = mapper.store)
                    box.set(store_fn(node_1));

                parcels.push(box);
                id_array.push(id_array_2[i]);
            }

//            ex::entity::remove(node->id_node);
            del_(node);
        }

        const auto after = Snapshot::create();

        undo_history().push(strings::gui_tree_ops_history_select_deleted,
        new std::function([before, id_array, parcels] (ex::State *) mutable {
            TreeState::reset(tree::state());
            before->rollback();

            for (int i = 0; i < id_array.size; ++i) {
                TreeNode *node_1 = ex::entity::find(id_array[i].now);
                ParcelBox box = parcels[i];
                const auto mapper = i_::mappers_.at_or_default(node_1->id_type);
                const auto restore_fn = mapper.restore;
                if (box.present() && restore_fn)
                    restore_fn(ex::entity::find(node_1->id_node), box.parcel());
            }
        }),
        new std::function([after, id_array, parcels] (ex::State *) mutable {
            TreeState::reset(tree::state());

            for (int i = 0; i < id_array.size; ++i) {
                TreeNode *node_1 = ex::entity::find(id_array[i].now);
                ParcelBox box = parcels[i];
                const auto mapper = i_::mappers_.at_or_default(node_1->id_type);
                const auto store_fn = mapper.store;

                if (box.present() && store_fn)
                    box.reset(store_fn(ex::entity::find(node_1->id_node)));
            }

            after->rollback();
        }),
        new std::function([after, before, parcels] () mutable {
            Snapshot::free(after);
            Snapshot::free(before);
            for (int i = 0; i < parcels.size; ++i)
                ParcelBox::free(parcels[i]);
        }));
    }

    void delete_node(TreeNode *node) {
        if (node == nullptr)
            return;

        TreeState::reset(tree::state());

        node->state -= node_state::selected;
        const auto before = Snapshot::create();

        ex::data::array<id_mapping> id_array;
        ex::data::array<ParcelBox>  parcels;

        deep_map_(node, &id_array);

        for (int i = 0; i < id_array.size; ++i) {
            TreeNode *node_1 = ex::entity::find(id_array[i].now);
            ParcelBox box = ParcelBox::create();
            const auto mapper = i_::mappers_.at_or_default(node_1->id_type);
            if (const auto store_fn = mapper.store)
                box.set(store_fn(node_1));
            parcels.push(box);
        }

//        ex::entity::remove(node->id_node);
        del_(node);

        const auto after = Snapshot::create();

        undo_history().push(strings::gui_tree_ops_history_node_deleted,
        new std::function([before, id_array, parcels] (ex::State *) mutable {
            TreeState::reset(tree::state());
            before->rollback();

            for (int i = 0; i < id_array.size; ++i) {
                TreeNode *node_1 = ex::entity::find(id_array[i].now);
                ParcelBox box = parcels[i];
                const auto mapper = i_::mappers_.at_or_default(node_1->id_type);
                const auto restore_fn = mapper.restore;
                if (box.present() && restore_fn)
                    restore_fn(ex::entity::find(node_1->id_node), box.parcel());
            }
        }),
        new std::function([after, id_array, parcels] (ex::State *) mutable {
            TreeState::reset(tree::state());

            for (int i = 0; i < id_array.size; ++i) {
                TreeNode *node_1 = ex::entity::find(id_array[i].now);
                ParcelBox box = parcels[i];
                const auto mapper = i_::mappers_.at_or_default(node_1->id_type);
                const auto store_fn = mapper.store;

                if (box.present() && store_fn)
                    box.reset(store_fn(ex::entity::find(node_1->id_node)));
            }

            after->rollback();
        }),
        new std::function([after, before, parcels] () mutable {
            Snapshot::free(after);
            Snapshot::free(before);
            for (int i = 0; i < parcels.size; ++i)
                ParcelBox::free(parcels[i]);
        }));
    }

}