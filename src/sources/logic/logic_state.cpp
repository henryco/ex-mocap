//
// Created by xd on 31/01/25.
//

#include "../../headers/logic/logic_state.h"

namespace ex::logic::state {

    ex::data::rb_map<id_local, limbo::graph::BaseNode *> &node_store_() {
        static ex::data::rb_map<id_local, limbo::graph::BaseNode *> instance;
        return instance;
    }

    limbo::graph::Clock &get_clock() {
        static limbo::graph::Clock clock_;
        return clock_;
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
        store.remove(id);
        store.put(id, node);
    }

    void remove_node(const id_local &id) {
        static auto &store = node_store_();
        store.remove(id);
    }

    void reset_nodes() {
        static auto &store = node_store_();
        static auto &clock = get_clock();
        clock.reset();
        store.clean();
    }

}
