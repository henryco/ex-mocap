//
// Created by xd on 2/13/25.
//

#include "../../../headers/imgui/tree/gui_tree_state.h"

namespace ex::gui::tree {

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

    void reset_state() {
        selection_node::free(*selected_nodes_ptr(), true);
        node_buffer::remove(node_buffer_ptr());
        *(selected_nodes_ptr()) = nullptr;
    }

    selection_node **selected_nodes_ptr() {
        static selection_node *ptr = nullptr;
        return &ptr;
    }

    node_buffer **node_buffer_ptr() {
        static node_buffer *ptr = nullptr;
        return &ptr;
    }

}