//
// Created by xd on 29/12/24.
//

#include "../headers/state.h"

namespace ex {

    TreeNode::~TreeNode() {
        if (children != nullptr) {
            for (int i = 0; i < children_n; i++)
                delete children[i];
            delete children;
        }
    }

    TreeNode *TreeNode::shallow_copy() {
        const auto new_node  = new TreeNode;
        new_node->connected  = false;
        new_node->selected   = false;
        new_node->deleted    = false;
        new_node->created    = true;
        new_node->updated    = false;
        new_node->children   = nullptr;
        new_node->parent     = nullptr;
        new_node->data       = nullptr;
        new_node->name       = this->name;
        new_node->index      = this->index;
        new_node->is_group   = this->is_group;
        new_node->shared     = this->shared;
        new_node->disabled   = this->disabled;
        new_node->id_local   = this->id_local;
        new_node->id_node    = this->id_node;
        new_node->children_n = 0;
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
                children[k]        = child;
                children[k]->index = k;
                k++;
            }
        }

        delete this->children;

        this->children_n = new_size;
        this->children   = children;
        node->index      = 0;
    }

    int TreeNode::get_child_index(TreeNode *node) {
        for (int i = 0; i < this->children_n; i++) {
            if (this->children[i] == node)
                return i;
        }
        return -1;
    }

    void TreeNode::add_node(TreeNode *node) {
        if (node == nullptr)
            return;

        node->parent = this;

        const int curr_n   = static_cast<int>(this->children_n);
        const int new_size = (curr_n < 0 ? 0 : curr_n) + 1;

        auto **children = new TreeNode *[new_size];
        for (int i = 0; i < new_size - 1; i++) {
            children[i]        = this->children[i];
            children[i]->index = i;
        }

        children[new_size - 1] = node;
        node->index            = new_size - 1;

        if (this->children != nullptr)
            delete this->children;

        this->children_n = new_size;
        this->children   = children;
    }

    void TreeNode::set_node(const int idx, TreeNode *node) {
        if (node == nullptr)
            return;

        node->parent = this;

        const int curr_n   = static_cast<int>(this->children_n);
        const int new_size = (curr_n < 0 ? 0 : curr_n) + 1;

        if (new_size <= 0)
            return;

        if (curr_n <= 0 || new_size == 1 || this->children == nullptr) {
            if (this->children != nullptr)
                delete this->children;
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

        delete this->children;

        this->children_n = new_size;
        this->children   = children;
    }

    TreeNode *TreeNode::find_node(const std::size_t id) {
        if (this->id_local == id)
            return this;
        for (int i = 0; i < this->children_n; i++) {
            if (const auto result = this->children[i]->find_node(id); result != nullptr)
                return result;
        }
        return nullptr;
    }

    TreeNode *TreeNode::find_node(TreeNode *node) {
        if (this == node)
            return this;
        for (int i = 0; i < this->children_n; i++) {
            if (const auto result = this->children[i]->find_node(node); result != nullptr)
                return result;
        }
        return nullptr;
    }

    TreeNode *TreeNode::find_node(TreeNode *const tree, const std::size_t id) {
        if (tree == nullptr)
            return nullptr;
        return tree->find_node(id);
    }

    TreeNode *TreeNode::shallow_copy(TreeNode *node) {
        if (node == nullptr)
            return nullptr;
        return node->shallow_copy();
    }

    selection_node *selection_node::copy() {
        // need to test it somehow in the future

        auto copy_selected      = new selection_node;
        copy_selected->selected = nullptr;
        copy_selected->root     = copy_selected;
        copy_selected->prev     = nullptr;
        copy_selected->next     = nullptr;

        auto head_src = this;
        while (head_src != nullptr) {
            copy_selected->selected = head_src->selected;
            head_src                = head_src->next;
            if (head_src != nullptr) {
                const auto next_copy = new selection_node;
                next_copy->root      = copy_selected->root;
                next_copy->prev      = copy_selected;
                next_copy->next      = nullptr;
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
                head = head->next;
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

    node_buffer::~node_buffer() {
        if (this->selection != nullptr) {
            auto head = this->selection->root;
            while (head != nullptr) {
                const auto next = head->next;
                if (head != this->selection)
                    delete head;
                head = next;
            }
            delete this->selection;
        }
    }

    void node_buffer::create(node_buffer **buffer, selection_node *selection, const bool move, const bool ref) {
        if (*buffer != nullptr)
            delete *buffer;
        *buffer              = new node_buffer;
        (*buffer)->selection = selection_node::copy(selection);
        (*buffer)->move      = move;
        (*buffer)->ref       = ref;
    }

    void node_buffer::create(node_buffer **buffer, TreeNode *node, const bool move, const bool ref) {
        if (*buffer != nullptr)
            delete *buffer;
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
        delete (*buffer);
        (*buffer) = nullptr;
    }
}
