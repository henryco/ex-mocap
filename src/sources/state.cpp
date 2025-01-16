//
// Created by xd on 29/12/24.
//

#include "../headers/state.h"

namespace ex {

    TreeNode::~TreeNode() {
        if (children != nullptr) {
            for (int i = 0; i < children_n - 1; i++)
                delete children[i];
            delete children;
        }
    }

    void TreeNode::remove_node(TreeNode *node) {
        if (node == nullptr)
            return;

        if (this->children == nullptr || this->children_n <= 0)
            return;

        const int curr_n   = static_cast<int>(this->children_n);
        const int new_size = (curr_n <= 0 ? 0 : (curr_n - 1));

        auto **children = new TreeNode *[new_size];
        for (int i = 0, k = 0; i < curr_n - 1; i++) {
            const auto child = this->children[i];
            if (child != node)
                children[k++] = child;
        }

        delete this->children;

        this->children_n = new_size;
        this->children   = children;
    }

    void TreeNode::add_node(TreeNode *node) {
        if (node == nullptr)
            return;

        node->parent = this;

        const int curr_n   = static_cast<int>(this->children_n);
        const int new_size = (curr_n < 0 ? 0 : curr_n) + 1;

        auto **children = new TreeNode *[new_size];
        for (int i = 0; i < new_size - 1; i++)
            children[i] = this->children[i];

        children[new_size - 1] = node;

        if (this->children != nullptr)
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
        if  (tree == nullptr)
            return nullptr;
        return tree->find_node(id);
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
} // namespace ex
