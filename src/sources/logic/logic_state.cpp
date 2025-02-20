//
// Created by xd on 31/01/25.
//

#include "../../headers/logic/logic_state.h"

namespace ex::logic::state {

    std::map<id_local, int> &node_counter_() {
        static std::map<id_local, int> instance;
        return instance;
    }

    std::map<id_local, limbo::nodes::BaseNode *> &node_store_() {
        static std::map<id_local, limbo::nodes::BaseNode *> instance;
        return instance;
    }

    static std::map<id_local, int> &counter_map_ = node_counter_();

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
        if (!store.contains(id)) {
            store.emplace(id, node);
            counter_map_[id]++;
        }
    }

    void replace_node(const id_local &id, limbo::nodes::BaseNode *node) {
        static auto &store = node_store_();
        if (store.contains(id)) {
            const auto base = store.at(id);
            store.erase(id);
            base->detach_all_any();
            delete base;
        }
        if (!store.contains(id))
            store.emplace(id, node);
    }

    void remove_node(const id_local &id) {
        static auto &store = node_store_();
        if (store.contains(id)) {
            const auto base = store.at(id);
            store.erase(id);
            base->detach_all_any();
            delete base;
        }

        static auto &counter = node_counter_();
        if (counter.contains(id))
            counter.erase(id);
    }

    int &get_node_counter(const id_local &id) {
        return counter_map_[id];
    }

    void cleanup_nodes() {
        for (auto it = counter_map_.begin(); it != counter_map_.end(); /*nop*/) {
            const auto id_local = it->first;
            if (it->second <= 0) {

                if (node_store_().contains(id_local)) {
                    delete node_store_().at(id_local);
                    node_store_().erase(id_local);
                }

                it = counter_map_.erase(it);
            } else {
                it->second = 0;
                ++it;
            }
        }
    }

} // namespace ex::logic::globals
