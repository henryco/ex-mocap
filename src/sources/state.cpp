//
// Created by xd on 29/12/24.
//

#include "../headers/state.h"
#include <cstring>

namespace ex {

    namespace state::uuid {
        static std::uint32_t id_counter_ = 0;

        std::uint32_t generate_id_inc() {
            return ++id_counter_;
        }

        std::uint32_t get_last_id() {
            return id_counter_;
        }

        void set_last_id(const std::uint32_t id) {
            if (id > id_counter_)
                id_counter_ = id;
        }

        void reset_last_id(const std::uint32_t id) {
            id_counter_ = id;
        }
    }

    TreeNode::~TreeNode() {
        if (children != nullptr) {
            for (int i = 0; i < children_n; i++) {
                if (children[i] != nullptr)
                    delete children[i];
            }
            delete[] children;
        }

        if (data != nullptr && data_size > 0)
            free(data);
    }

    TreeNode *TreeNode::shallow_copy() const {
        const auto new_node  = new TreeNode;
        new_node->state      = this->state;
        new_node->parent     = this->parent;
        new_node->children   = nullptr;
        new_node->data       = nullptr;
        new_node->name       = this->name;
        new_node->index      = this->index;
        new_node->id_local   = this->id_local;
        new_node->id_node    = this->id_node;
        new_node->id_link    = this->id_link;
        new_node->data_size  = 0;
        new_node->children_n = 0;
        return new_node;
    }

    TreeNode *TreeNode::deep_copy(const bool generated_id) const {
        const auto new_node  = new TreeNode;
        new_node->id_local   = generated_id ? state::uuid::generate_id_inc() : id_local;
        new_node->state      = generated_id ? node_state::created : state;
        new_node->id_link    = id_link;
        new_node->parent     = parent;
        new_node->name       = name;
        new_node->index      = index;
        new_node->id_node    = id_node;
        new_node->data_size  = data_size;
        new_node->children_n = children_n;

        new_node->state |= (state & node_state::is_group);
        new_node->state |= (state & node_state::is_link);

        if (data != nullptr && data_size > 0) {
            void *new_data = malloc(data_size);
            std::memcpy(new_data, data, data_size);
            new_node->data = new_data;
        }
        else {
            new_node->data = nullptr;
        }

        if (children_n <= 0 || children == nullptr) {
            new_node->children = nullptr;
            new_node->children_n = 0;
            return new_node;
        }

        new_node->children = new TreeNode *[children_n];
        for (int i = 0; i < children_n; i++) {
            const auto child = children[i];

            if (child == nullptr) {
                new_node->children[i] = nullptr;
                continue;
            }

            const auto new_child  = child->deep_copy(generated_id);
            new_child->parent     = new_node;
            new_node->children[i] = new_child;
        }

        return new_node;
    }

    void TreeNode::remove_node(TreeNode *node) {
        if (node == nullptr)
            return;

        if (this->children == nullptr || this->children_n <= 0)
            return;

        const int curr_n   = static_cast<int>(this->children_n);
        const int new_size = (curr_n <= 0 ? 0 : (curr_n - 1));

        auto **children = new TreeNode *[new_size];
        for (int i = 0, k = 0; i < curr_n; i++) {
            const auto child = this->children[i];
            if (child != node) {
                children[k] = child;

                if (children[k] != nullptr)
                    children[k]->index = k;

                k++;
            }
        }

        delete[] this->children;

        this->children_n = new_size;
        this->children   = children;
        node->index      = 0;
    }

    bool TreeNode::is_parent(TreeNode *node) const {
        if (node == nullptr)
            return false;
        auto head = this;
        while (head != nullptr) {
            if (head == node)
                return true;
            head = head->parent;
        }
        return false;
    }

    void TreeNode::add_node(TreeNode *node) {
        if (node == nullptr)
            return;

        node->parent = this;

        const int curr_n   = static_cast<int>(this->children_n);
        const int new_size = (curr_n < 0 ? 0 : curr_n) + 1;

        auto **children = new TreeNode *[new_size];
        for (int i = 0; i < new_size - 1; i++) {
            children[i] = this->children[i];

            if (children[i] != nullptr)
                children[i]->index = i;
        }

        children[new_size - 1] = node;
        node->index            = new_size - 1;

        if (this->children != nullptr)
            delete[] this->children;

        this->children_n = new_size;
        this->children   = children;
    }

    void TreeNode::insert_node(const int idx, TreeNode *node) {
        if (node == nullptr)
            return;

        node->parent = this;

        const int curr_n   = static_cast<int>(this->children_n);
        const int new_size = (curr_n < 0 ? 0 : curr_n) + 1;

        if (new_size <= 0)
            return;

        if (curr_n <= 0 || new_size == 1 || this->children == nullptr) {
            if (this->children != nullptr)
                delete[] this->children;
            this->children           = new TreeNode *[1];
            this->children_n         = 1;
            this->children[0]        = node;
            this->children[0]->index = 0;
            return;
        }

        auto **children = new TreeNode *[new_size];

        for (int i = 0, k = 0; i < new_size; i++) {
            if (i == idx || k == new_size - 1) {
                children[i]        = node;
                children[i]->index = i;
                continue;
            }
            children[i]        = this->children[k++];
            children[i]->index = i;
        }

        delete[] this->children;

        this->children_n = new_size;
        this->children   = children;
    }

    TreeNode *TreeNode::find_node(const std::uint32_t id) {
        if (this->id_local == id)
            return this;
        for (int i = 0; i < this->children_n; i++) {
            const auto child = this->children[i];
            if (child == nullptr)
                continue;
            if (const auto result = child->find_node(id); result != nullptr)
                return result;
        }
        return nullptr;
    }

    TreeNode *TreeNode::find_node(TreeNode *node) {
        if (this == node)
            return this;
        for (int i = 0; i < this->children_n; i++) {
            const auto child = this->children[i];
            if (child == nullptr)
                continue;
            if (const auto result = child->find_node(node); result != nullptr)
                return result;
        }
        return nullptr;
    }

    TreeNode *TreeNode::find_node(TreeNode *const tree, const std::uint32_t id) {
        if (tree == nullptr)
            return nullptr;
        return tree->find_node(id);
    }

    TreeNode *TreeNode::shallow_copy(TreeNode *node) {
        if (node == nullptr)
            return nullptr;
        return node->shallow_copy();
    }

    TreeNode *TreeNode::deep_copy(TreeNode *node, const bool generated_id) {
        if (node == nullptr)
            return nullptr;
        return node->deep_copy(generated_id);
    }

    TreeNode *TreeNode::create(const std::uint32_t id) {
        const auto node = new TreeNode;
        if (id > 0) {
            state::uuid::set_last_id(id);
            node->id_local = id;
            node->state -= node_state::disabled;
        }
        else {
            node->id_local = state::uuid::generate_id_inc();
            node->state -= node_state::disabled;
        }
        return node;
    }

    TreeNode *TreeNode::link(TreeNode *node) {
        if (node == nullptr)
            return nullptr;
        TreeNode *new_one = TreeNode::create();
        new_one->state    = node_state::created | node_state::is_link;
        new_one->name     = node->name;
        new_one->id_link  = node->id_local;
        new_one->id_node  = node->id_node;
        return new_one;
    }

    bool TreeNode::is_parent(TreeNode *test, TreeNode *node) {
        if (node == nullptr || test == nullptr)
            return false;
        return test->is_parent(node);
    }

    selection_node *selection_node::copy() const {
        // need to test it somehow in the future

        auto copy_selected      = new selection_node;
        copy_selected->selected = nullptr;
        copy_selected->root     = copy_selected;
        copy_selected->prev     = nullptr;
        copy_selected->next     = nullptr;
        copy_selected->index    = -1;

        auto head_src = this;
        while (head_src != nullptr) {
            copy_selected->selected = head_src->selected;
            copy_selected->index    = head_src->index;
            head_src                = head_src->next;
            if (head_src != nullptr) {
                const auto next_copy = new selection_node;
                next_copy->root      = copy_selected->root;
                next_copy->prev      = copy_selected;
                next_copy->next      = nullptr;
                next_copy->index     = -1;
                copy_selected->next  = next_copy;
                copy_selected        = next_copy;
            }
        }
        return copy_selected;
    }

    selection_node *selection_node::copy(selection_node *src) {
        if (src == nullptr)
            return nullptr;
        return src->copy();
    }

    selection_node *selection_node::from_node(TreeNode *node) {
        if (node == nullptr)
            return nullptr;
        const auto instance = new selection_node;
        instance->selected  = node;
        instance->root      = instance;
        instance->prev      = nullptr;
        instance->next      = nullptr;
        return instance;
    }

    bool selection_node::single(selection_node *node) {
        return node == nullptr || node->root->next == nullptr;
    }

    void selection_node::remove(selection_node *node, selection_node **next) {
        if (node == nullptr)
            return;

        if (node->prev == nullptr) {
            if (node->next == nullptr) {
                *next = nullptr;
                delete node;
                return;
            }

            const auto root = node->next;

            root->prev = nullptr;
            *next      = root;

            auto head = node->next;
            while (head != nullptr) {
                head->root = root;
                head       = head->next;
            }

            delete node;
            return;
        }

        if (node->next == nullptr) {
            node->prev->next = nullptr;
            *next            = node->prev;
            delete node;
            return;
        }

        node->prev->next = node->next;
        *next            = node->next;
        delete node;
    }

    void selection_node::free(selection_node *node, const bool unselect) {
        if (node == nullptr)
            return;
        auto head = node->root;
        while (head != nullptr) {
            const auto next = head->next;
            if (head != node) {
                if (unselect && head->selected != nullptr)
                    head->selected->state -= node_state::selected;
                delete head;
            }
            head = next;
        }
        if (unselect && node->selected != nullptr)
            node->selected->state -= node_state::selected;
        delete node;
    }

    node_buffer::~node_buffer() {
        selection_node::free(this->selection);
    }

    void node_buffer::create(node_buffer **buffer, selection_node *selection, const bool move, const bool ref) {
        if (*buffer != nullptr)
            node_buffer::remove(buffer);
        *buffer              = new node_buffer;
        (*buffer)->selection = selection_node::copy(selection);
        (*buffer)->move      = move;
        (*buffer)->ref       = ref;
    }

    void node_buffer::create(node_buffer **buffer, TreeNode *node, const bool move, const bool ref) {
        if (*buffer != nullptr)
            node_buffer::remove(buffer);
        *buffer = new node_buffer;

        const auto selection = new selection_node;
        selection->selected  = node;
        selection->root      = selection;
        selection->prev      = nullptr;
        selection->next      = nullptr;

        (*buffer)->selection = selection;
        (*buffer)->move      = move;
        (*buffer)->ref       = ref;
    }

    void node_buffer::remove(node_buffer **buffer) {
        if (*buffer == nullptr)
            return;

        if (const auto selection = (*buffer)->selection; selection != nullptr) {
            auto head = selection->root;
            while (head != nullptr) {
                if (head->selected != nullptr) {
                    head->selected->state -= node_state::selected;
                    head->selected->state -= node_state::dimmed;
                }
                head = head->next;
            }
        }

        delete (*buffer);
        (*buffer) = nullptr;
    }
}
