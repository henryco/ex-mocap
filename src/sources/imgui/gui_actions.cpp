//
// Created by xd on 2/14/25.
//

#include "../../headers/imgui/gui_actions.h"

#include <stdlib.h>

namespace ex::gui::actions {

    struct body {
        action_fn_t fn;
        int         id;
    };

    struct array {

        body *data     = nullptr;
        int   size     = 0;
        int   capacity = 0;

        array() = default;

        ~array() {
            if (data == nullptr)
                return;
            free(data);
        }

        explicit array(const int capacity) {
            this->capacity = capacity;
            this->data     = static_cast<body *>(malloc(capacity * sizeof(body)));
        }

        body &operator [] (const int index) {
            return data[index];
        }

        const body &operator [] (const int index) const {
            return data[index];
        }

        void push(const body &value) {
            if (size >= capacity) {
                capacity = capacity > 0 ? (2 * capacity) : 2;
                data     = static_cast<body *>(realloc(data, capacity * sizeof(body)));
            }
            data[size++] = value;
        }

        void pop() {
            size = (size > 0) ? (size - 1) : 0;
        }

        bool empty() const {
            return size <= 0;
        }
    };

    array &registry_() {
        static array array(20);
        return array;
    }

    void set_action(int action, action_fn_t fn) {

    }

    void remove_action(int action) {

    }

    action_fn_t get_action(const int action) {
        auto &reg = registry_();
        for (auto i = 0; i < reg.size; i++) {
            if (reg[i].id == action)
                return reg[i].fn;
        }
        return nullptr;
    }

    action_fn_t *list_actions(int *size_n) {
        // TODO
        return nullptr;
    }

}
