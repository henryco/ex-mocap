//
// Created by xd on 31/01/25.
//

#include "../../headers/logic/logic_state.h"

namespace ex::logic::state {

    ex::data::rb_map<id_local, limbo::graph::BaseNode *> &node_store_() {
        static ex::data::rb_map<id_local, limbo::graph::BaseNode *> instance;
        return instance;
    }

    const ex::data::rb_map<id_local, limbo::graph::BaseNode *> &node_storage() {
        return node_store_();
    }

    limbo::graph::BaseNode *get_node(const id_local &id) {
        static auto &store = node_store_();
        if (!store.contains(id))
            return nullptr;
        return store.at(id);
    }

    void store_node(const id_local &id, limbo::graph::BaseNode *node) {
        static auto &store = node_store_();
        store.put(id, node);
    }

    void replace_node(const id_local &id, limbo::graph::BaseNode *node) {
        static auto &store = node_store_();
        if (store.contains(id)) {
            const auto base = store.at(id);
            base->detach_all();
            store.remove(id);
            delete base;
        }
        store.put(id, node);
    }

    void remove_node(const id_local &id) {
        static auto &store = node_store_();
        if (store.contains(id)) {
            const auto base = store.at(id);
            base->detach_all();
            store.remove(id);
            delete base;
        }
    }

} // namespace ex::logic::globals
