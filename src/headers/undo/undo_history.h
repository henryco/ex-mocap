//
// Created by henryco on 07/01/25.
//

#ifndef UNDO_HISTORY_H
#define UNDO_HISTORY_H

#include <functional>

namespace ex::undo {

    template<typename T>
    struct record {
        using rollback_fn = std::function<void(T *)>;

        const std::size_t  id;
        const char        *name = nullptr;
        const rollback_fn *undo = nullptr;
        record            *prev = nullptr;
        record            *next = nullptr;

        ~record() {
            if (undo != nullptr)
                delete undo;
            if (next != nullptr)
                delete next;
        }
    };

    template<typename T>
    struct history {
        record<T> *records = nullptr;
        record<T> *current = nullptr;

        ~history() {
            record<T> *head = records;
            while (head != nullptr) {
                record<T> *prev = head->prev;
                delete head;
                head = prev;
            }
        }

        void push(const char *name, std::function<void(T *)> *fn) {
            static std::size_t counter = 0;

            if (records == nullptr || current == nullptr) {
                records = new record<T>(counter++, name, fn, nullptr, nullptr);
                current = records;
                return;
            }

            if (records->id == current->id && current == records) {
                records->next = new record<T>(counter++, name, fn, records, nullptr);
                records = records->next;
                current = records;
                return;
            }

            record<T> *head = records;
            while (head != nullptr) {
                if (head->id == current->id && current == head) {
                    current->next = new record<T>(counter++, name, fn, current, nullptr);
                    current = current->next;
                    records = current;
                    return;
                }
                record<T> *prev = head->prev;
                delete head;
                head = prev;
            }
        }

        bool move(const std::size_t id) {
            record<T> *head = records;
            while (head != nullptr) {
                if (head->id == id) {
                    current = head;
                    return true;
                }
                head = head->prev;
            }
            return false;
        }

        void redo() {
            if (records == nullptr || current == nullptr)
                return;
            if (current->next != nullptr)
                current = current->next;
        }

        void undo() {
            if (records == nullptr || current == nullptr)
                return;
            if (current->prev != nullptr)
                current = current->prev;
        }

        void apply(T *state) {
            if (records == nullptr || current == nullptr)
                return;
            if (current->undo != nullptr)
                current->undo(state);
        }
    };

}

#endif //UNDO_HISTORY_H
