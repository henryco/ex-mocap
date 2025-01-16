//
// Created by xd on 29/12/24.
//

#include "../headers/state.h"

namespace ex {

    TreeNode::~TreeNode() {
        // TODO
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
                delete head;
                head = next;
            }
            delete this->selection;
        }
    }

    void node_buffer::create(node_buffer **buffer, selection_node *selection, const bool move) {
        if (*buffer != nullptr)
            delete *buffer;
        *buffer = new node_buffer;
        (*buffer)->selection = selection_node::copy(selection);
        (*buffer)->move = move;
    }
} // namespace ex
