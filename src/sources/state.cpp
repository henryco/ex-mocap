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
        if (this->id_local == id) return this;
        for (int i = 0; i < this->children_n; i++) {
            if (const auto result = this->children[i]->find_node(id); result != nullptr)
                return result;
        }
        return nullptr;
    }
} // namespace ex
