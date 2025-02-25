//
// Created by xd on 31/01/25.
//

#include "../../headers/logic/logic_state.h"

namespace ex::logic::state {

    std::map<id_local, limbo::nodes::BaseNode *> &node_store_() {
        static std::map<id_local, limbo::nodes::BaseNode *> instance;
        return instance;
    }

    const std::map<id_local, limbo::nodes::BaseNode *> &node_storage() {
        return node_store_();
    }

    limbo::nodes::BaseNode *get_node(const id_local &id) {
        static auto &store = node_store_();
        if (!store.contains(id))
            return nullptr;
        return store.at(id);
    }

    void store_node(const id_local &id, limbo::nodes::BaseNode *node) {
        static auto &store = node_store_();
        if (!store.contains(id))
            store.emplace(id, node);
    }

    void replace_node(const id_local &id, limbo::nodes::BaseNode *node) {
        static auto &store = node_store_();
        if (store.contains(id)) {
            const auto base = store.at(id);
            base->detach_all_any();
            store.erase(id);
            delete base;
        }
        if (!store.contains(id))
            store.emplace(id, node);
    }

    void remove_node(const id_local &id) {
        static auto &store = node_store_();
        if (store.contains(id)) {
            const auto base = store.at(id);
            base->detach_all_any();
            store.erase(id);
            delete base;
        }
    }

} // namespace ex::logic::globals
