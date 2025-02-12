//
// Created by xd on 31/01/25.
//

#include "../../headers/logic/logic_globals.h"
#include "../../headers/imgui/cmp/test_cmp.h"

namespace ex::logic::state {

    std::map<id_local, int> &node_counter_() {
        static std::map<id_local, int> instance;
        return instance;
    }

    std::map<id_local, limbo::nodes::BaseNode *> &node_store_() {
        static std::map<id_local, limbo::nodes::BaseNode *> instance;
        return instance;
    }

    State &get_global_state() {
        static State instance;
        return instance;
    }

    const std::map<id_type, ex::LogicNodeComponent> &node_components() {
        // TODO REPLACE WITH REAL COMPONENTS
        static const std::map<id_type, ex::LogicNodeComponent> instance = {
            // { cmp::CameraNode::get_node_id(), map::camera::mappers },
            { ex::gui::test::NodeOne::get_node_id(), ex::gui::test::one::data_mappers },
            { ex::gui::test::NodeTwo::get_node_id(), ex::gui::test::two::data_mappers },
            { ex::gui::test::NodeTri::get_node_id(), ex::gui::test::tri::data_mappers },
            { 0, ex::gui::test::one::data_mappers },
            { 1, ex::gui::test::tri::data_mappers },
            { 2, ex::gui::test::two::data_mappers },
            { 3, ex::gui::test::one::data_mappers },
            { 4, ex::gui::test::tri::data_mappers },
            { 5, ex::gui::test::two::data_mappers },
            { 6, ex::gui::test::one::data_mappers },
        };
        return instance;
    }

    const std::map<id_local, limbo::nodes::BaseNode *> &node_storage() {
        return node_store_();
    }

    limbo::nodes::BaseNode *get_node(const id_local &id) {
        if (!node_store_().contains(id))
            return nullptr;
        return node_store_()[id];
    }

    void store_node(const id_local &id, limbo::nodes::BaseNode *node) {
        if (!node_store_().contains(id))
            node_store_().emplace(id, node);
        inc_node_counter(id);
    }

    void replace_node(const id_local &id, limbo::nodes::BaseNode *node) {
        if (node_store_().contains(id)) {
            const auto base = node_store_().at(id);
            node_store_().erase(id);
            base->detach_all_any();
            delete base;
        }
        if (!node_store_().contains(id))
            node_store_().emplace(id, node);
    }

    void remove_node(const id_local &id) {
        if (node_store_().contains(id)) {
            const auto base = node_store_().at(id);
            node_store_().erase(id);
            base->detach_all_any();
            delete base;
        }
        if (node_counter_().contains(id))
            node_counter_().erase(id);
    }

    void inc_node_counter(const id_local &id) {
        if (node_counter_().contains(id)) {
            const auto v          = node_counter_().at(id);
            node_counter_().at(id) = std::min(v + 1, 10000000);
        }
        else
            node_counter_().emplace(id, 1);
    }

    void cleanup_nodes() {
        for (auto it = node_counter_().begin(); it != node_counter_().end(); /*nop*/) {
            const auto id_local = it->first;
            if (it->second <= 0) {

                if (node_store_().contains(id_local)) {
                    delete node_store_().at(id_local);
                    node_store_().erase(id_local);
                }

                it = node_counter_().erase(it);
            } else {
                it->second = 0;
                ++it;
            }
        }
    }

} // namespace ex::logic::globals
