//
// Created by xd on 29/12/24.
//

#include "../headers/state.h"

#include <cstring>
#include <string>

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

    std::string from_id(const TreeNode *node) {
        return std::string(std::string(node->name) + "##"
            + std::to_string(node->id_local) + "/"
            + std::to_string(node->id_node));
    }

    void TreeNode::gen_uuid() {
        const std::string s_uuid = from_id(this);
        const auto        buffer = new char[s_uuid.size() + 1];
        strcpy(buffer, s_uuid.c_str());
        this->uuid = buffer;
    }

} // namespace ex
