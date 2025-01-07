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
        record<T> *target_ = nullptr;

        ~history() {
            record<T> *head = records;
            while (head != nullptr) {
                record<T> *prev = head->prev;
                head->next = nullptr;
                delete head;
                head = prev;
            }
        }

        void push(const char *name, std::function<void(T *)> *fn) {
            static std::size_t counter = 0;

            if (records == nullptr || current == nullptr) {
                records = new record<T>(counter++, name, fn, nullptr, nullptr);
                current = records;
                target_ = records;
                return;
            }

            if (records->id == current->id && current == records) {
                records->next = new record<T>(counter++, name, fn, records, nullptr);
                records = records->next;
                current = records;
                target_ = records;
                return;
            }

            // - - - - - - - - - - - // records
            //           x           // current
            //           > > > > x   // target_

            record<T> *head = records;
            while (head != nullptr) {
                if (head->id == current->id && current == head) {
                    current->next = new record<T>(counter++, name, fn, current, nullptr);
                    current = current->next;
                    records = current;
                    target_ = current;
                    return;
                }
                record<T> *prev = head->prev;
                head->next = nullptr;
                delete head;
                head = prev;
            }
        }

        bool move(const std::size_t id) {
            record<T> *head = records;
            while (head != nullptr) {
                if (head->id == id) {
                    target_ = head;
                    return true;
                }
                head = head->prev;
            }
            return false;
        }

        void redo() {
            if (records == nullptr || current == nullptr || target_ == nullptr)
                return;
            if (target_->next != nullptr)
                target_ = target_->next;
        }

        void undo() {
            if (records == nullptr || current == nullptr || target_ == nullptr)
                return;
            if (target_->prev != nullptr)
                target_ = target_->prev;
        }

        void apply(T *state) {
            if (records == nullptr || current == nullptr || target_ == nullptr)
                return;

            if (target_->id == current->id && current == target_)
                return;

            const bool direction = target_->id > current->id;
            while (current != nullptr) {
                if (current->id == target_->id && current == target_)
                    return;
                if (current->undo != nullptr)
                    current->undo(state);
                current = direction ? current->next : current->prev;
            }
        }
    };

}

#endif //UNDO_HISTORY_H
