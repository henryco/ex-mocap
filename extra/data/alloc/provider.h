//
// Created by henryco on 18/03/25.
//

#ifndef EX_LIMBO_DATA_MEMPROVIDER_H
#define EX_LIMBO_DATA_MEMPROVIDER_H

#include <cstdlib>

namespace ex::data {

    // TODO REPLACE WITH SYS-CALLS
    struct mem_provider {

        struct mem_node {
            mem_node *prev;
            void     *data;
            long      size;
        };

        mem_node *node = nullptr;

        mem_provider() = default;

        ~mem_provider() {
            if (node == nullptr)
                return;
            mem_node *head = node;
            while (head != nullptr) {
                mem_node *prev = head->prev;
                ::free(head->data);
                ::free(head);
                head = prev;
            }
            node = nullptr;
        }

        void clean() {
            if (node == nullptr)
                return;
            mem_node *head = node;
            while (head != nullptr) {
                mem_node *prev = head->prev;
                ::free(head->data);
                ::free(head);
                head = prev;
            }
            node = nullptr;
        }

        long size() const {
            long            s    = 0;
            const mem_node *head = node;
            while (head != nullptr) {
                s += head->size;
                head = head->prev;
            }
            return s;
        }

        void *request(const std::size_t size, std::size_t *size_allocated = nullptr) {
            const std::size_t real_size = (this->size() * 2) + size;

            void *mem = ::malloc(real_size);
            if (mem == nullptr)
                abort_("Memory allocation error");

            if (node == nullptr) {
                node = static_cast<mem_node *>(::malloc(sizeof(mem_node)));
                if (node == nullptr)
                    abort_("Memory allocation error");
                node->prev = nullptr;
                node->data = mem;
                node->size = real_size;
            } else {
                mem_node *new_node = static_cast<mem_node *>(::malloc(sizeof(mem_node)));
                if (new_node == nullptr)
                    abort_("Memory allocation error");
                new_node->prev     = node;
                new_node->data     = mem;
                new_node->size     = real_size;
                node               = new_node;
            }

            *size_allocated = real_size;
            return mem;
        }
    };
}

#endif //EX_LIMBO_DATA_MEMPROVIDER_H
